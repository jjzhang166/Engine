#ifndef		__ENGINE_SCRIPT_H_INCLUDED__
#define		__ENGINE_SCRIPT_H_INCLUDED__

extern "C" {
#	include	"lua/lua.h"
#	include "lua/lualib.h"
#	include	"lua/lauxlib.h"
}

#include	<cstring>
#include	<string>

#define		LUA(code)	#code
#define		GLua		LuaVM::Instance()

/**
 * Macro for declare enum in Lua
 */
#define		LUA_ENUM(T)	\
template<> struct LuaRef<T> { typedef T Type; }; \
template<> struct LuaGetter<T> { static T Do(lua_State * p, int n) { return (T)lua_tointeger(p, n); } }; \
template<> struct LuaPusher<T> { static void Do(lua_State * p, T t) { lua_pushinteger(p, (lua_Integer)t); } }; \
template<> struct LuaName<T> { static const char * Of() { return "integer"; } }; \
template<> struct LuaType<T> { static bool Is(int n) { return n == LUA_TNUMBER; } };

/**
 * Declaration
 */
class LuaNil;
class LuaTable;
class LuaStack;
class LuaVM;

/**
 * Template helpers.
 */
template<typename T> struct LuaRef { typedef T & Type; };
template<typename T> struct LuaGetter {};
template<typename T> struct LuaPusher {};
template<typename T> struct LuaName { static const char * Of() { return "unknown"; } };
template<typename T> struct LuaType { static bool Is(int) { return false; }; };

/**
 * Lua nil type
 */
class LuaNil {};
extern LuaNil GLuaNil;

/**
 * Lua stack
 */
class LuaStack {
public:
	LuaStack(lua_State * pL, int nClean) : _pL(pL), _nClean(nClean) {}
	virtual ~LuaStack() { if (_nClean > 0) lua_pop(_pL, _nClean); }

	/**
	 * Get top of current stack.
	 */
	inline int Top() { return lua_gettop(_pL); }

	/**
	 * Pop number of values on top.
	 *
	 * \param	n	Number of elements to pop.
	 */
	inline void Pop(int n) { lua_pop(_pL, n); }

	/**
	 * Get type of stack element.
	 *
	 * \param	nIdx	Index of this element on this stack.
	 * \return	Type of this element.
	 */
	inline int Type(int nIdx) { return lua_type(_pL, nIdx); }

	/**
	 * Check element type.
	 *
	 * \param	nIdx	Index of this element on this stack.
	 * \return	If this element is convertible with T returns true. Otherwise false will be returned.
	 */
	template<typename T> bool Is(int nIdx);

	/**
	 * Convert Lua element on stack to C++ value.
	 *
	 * \param	nIdx	Index of this element on this stack.
	 * \return	C++ value. Note : This method will always check this element's type!
	 **/
	template<typename T> T Get(int nIdx);

	/**
	 * Push value onto this stack.
	 *
	 * \param	t	Reference value. See \ref LuaRef<T>.
	 */
	template<typename T> void Push(typename LuaRef<T>::Type t);

private:
	lua_State *	_pL;
	int			_nClean;
};

/**
 * Lua Table.
 */
class LuaTable {
public:
	LuaTable();
	LuaTable(lua_State * pL, int nRef) : _pL(pL), _nRef(nRef) {}
	LuaTable(const LuaTable & r) : _pL(r._pL), _nRef(LUA_NOREF) { lua_rawgeti(_pL, LUA_REGISTRYINDEX, r._nRef); _nRef = luaL_ref(_pL, LUA_REGISTRYINDEX); }
	virtual ~LuaTable() { __Unref(); }

	/**
	 * Reference on given table.
	 *
	 * \param	r	Data to reference on.
	 * \return	Self.
	 */
	LuaTable & operator=(const LuaTable & r);

	/**
	 * Is this table valid?
	 */
	inline bool IsValid() const { return (_pL && _nRef != LUA_NOREF); }

	/**
	 * Push this table onto stack. Use LuaStack::Push<LuaTable>() instead.
	 */
	void Push() { if (IsValid()) lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef); else lua_pushnil(_pL); }

	/**
	 * Get size of this table. Equal with '#' operator in Lua.
	 */
	int Size() const;

	/**
	 * Check element type in array table.
	 *
	 * \param	nIdx	Index of this element in array table.
	 * \return	True for element is convertible with given type.
	 */
	template<typename T> bool Is(int nIdx);

	/**
	 * Check element type in map table.
	 *
	 * \param	sKey	Key to find this element in map table.
	 * \return	True for element is convertible with given type.
	 */
	template<typename T> bool Is(const std::string & sKey);

	/**
	 * Get element in array table.
	 *
	 * \param	nIdx	element index.
	 * \return	Value of this element.
	 */
	template<typename T> T Get(int nIdx);

	/**
	 * Get element in map table.
	 *
	 * \param	sKey	Key to find this element.
	 * \return	Value of this element.
	 */
	template<typename T> T Get(const std::string & sKey);

	/**
	 * Set value in array table.
	 *
	 * \param	nIdx	Element's index in this array table.
	 * \param	t		Value of this element.
	 */
	template<typename T> void Set(int nIdx, typename LuaRef<T>::Type t);

	/**
	 * Set value in map table.
	 *
	 * \param	sKey	Key to find this element in map table.
	 * \param	t		Value of this element.
	 */
	template<typename T> void Set(const std::string & sKey, typename LuaRef<T>::Type t);

private:
	void	__Unref();

protected:
	lua_State *	_pL;
	int			_nRef;
};

/**
 * Register for global properties or methods.
 */
class LuaNamespace : public LuaTable {
public:
	LuaNamespace(lua_State * pL, int nRef) : LuaTable(pL, nRef) {}

	/**
	 * Register property by it's address.
	 *
	 * \param	sProp		Key to find this property.
	 * \param	pData		Pointer address of this property.
	 * \param	bReadonly	Is this property read-only?
	 * \return	Self.
	 */
	template<typename T> LuaNamespace &	Property(const std::string & sProp, T * pData, bool bReadonly = true);

	/**
	 * Register property by getter and setter function.
	 *
	 * \param	sProp	Key to find this property.
	 * \param	fGetter	Getter function.
	 * \param	fSetter	Setter function.
	 * \return	Self.
	 */
	template<typename T> LuaNamespace & Property(const std::string & sProp, T (*fGetter)(), void (*fSetter)(typename LuaRef<T>::Type) = nullptr);

	/**
	 * Register method into this namespace(table).
	 *
	 * \param	sMethod	Method name used in Lua.
	 * \param	fOpt	C++ function to call.
	 * \return	Self.
	 */
	LuaNamespace & Method(const std::string & sMethod, int (*fOpt)(LuaStack &));
};

/**
 * Class Register.
 */
template<class O>
class LuaClass : public LuaTable {
public:
	LuaClass(lua_State * pL, int nRef) : LuaTable(pL, nRef) {}

	/**
	 * Register property by it's address.
	 *
	 * \param	sProp		Key to find this property.
	 * \param	pData		Pointer address of this property.
	 * \param	bReadonly	Is this property read-only?
	 * \return	Self.
	 */
	template<typename T> LuaClass<O> &	Property(const std::string & sProp, T O::*pData, bool bReadonly = true);

	/**
	 * Register property by getter and setter function.
	 *
	 * \param	sProp	Key to find this property.
	 * \param	fGetter	Getter function.
	 * \param	fSetter	Setter function.
	 * \return	Self.
	 */
	template<typename T> LuaClass<O> & Property(const std::string & sProp, T (O::*fGetter)(), void (O::*fSetter)(typename LuaRef<T>::Type) = nullptr);

	/**
	 * Register property by getter and setter function.
	 *
	 * \param	sProp	Key to find this property.
	 * \param	fGetter	Getter function const.
	 * \param	fSetter	Setter function.
	 * \return	Self.
	 */
	template<typename T> LuaClass<O> & Property(const std::string & sProp, T (O::*fGetter)() const, void (O::*fSetter)(typename LuaRef<T>::Type) = nullptr);

	/**
	 * Register method into this class(table).
	 *
	 * \param	sMethod	Method name used in Lua.
	 * \param	fOpt	C++ function to call.
	 * \return	Self.
	 */
	LuaClass<O> & Method(const std::string & sMethod, int (*fOpt)(LuaStack &));
};

/**
 * Lua VM. High level interface for Lua.
 */
class LuaVM {
public:
	LuaVM();
	virtual ~LuaVM() { if (_pL) lua_close(_pL); }

	/**
	 * Runtime singleton instance of Lua Virtual Machine.
	 */
	static LuaVM &	Instance();

	/**
	 * Get low level interface of Lua VM.
	 */
	inline lua_State * State() { return _pL; }

	/**
	 * Get current stack of this Lua VM.
	 */
	inline LuaStack	Stack() { return LuaStack(_pL, 0); }

	/**
	 * Run Lua code in given file.
	 *
	 * \param	sFile		Lua file path.
	 * \param	bRequire	Load this file in 'require' mode?
	 */
	void DoFile(const std::string & sFile, bool bRequre = false);

	/**
	 * Run Lua code in string.
	 *
	 * \param	sCode	Lua code.
	 */
	void Run(const std::string & sCode);

	/**
	 * Check global variable's type.
	 *
	 * \param	sName	Name to find this variable.
	 * \return	True for it is convertible.
	 */
	template<typename T> bool Is(const std::string & sName);

	/**
	 * Get global variable's value.
	 *
	 * \param	sName	Name to find this variable.
	 * \return	Value of this variable. This method will always check type.
	 */
	template<typename T> T Get(const std::string & sName);

	/**
	 * Set global variable.
	 *
	 * \param	sName	Name to find this variable.
	 * \param	t		Value.
	 */
	template<typename T> void Set(const std::string & sName, typename LuaRef<T>::Type t);

	/**
	 * Call function defined in Lua. Equals sTable.sFunc(args...).
	 *
	 * \param	sTable	Name of table that contains this function. Can be global '_G'.
	 * \param	sFunc	Name of method to call.
	 * \param	args...	Parameters.
	 * \return	Result stack.
	 */
	template<typename ... Args> LuaStack Call(const std::string & sTable, const std::string & sFunc, Args && ... args);

	/**
	 * Call function defined in Lua. Equals sTable:sFunc(args...).
	 *
	 * \param	sTable	Name of table that contains this function. Can be global '_G'.
	 * \param	sFunc	Name of method to call.
	 * \param	args...	Parameters.
	 * \return	Result stack.
	 */
	template<typename ... Args> LuaStack SelfCall(const std::string & sTable, const std::string & sFunc, Args && ... args);

	/**
	 * Register C++ property or method into Lua.
	 *
	 * \param	sNamespace	Name of this namespace(table).
	 * \return	Register to add properties and methods.
	 */
	LuaNamespace Register(const std::string & sNamespace);

	/**
	 * Register C++ class into Lua.
	 *
	 * \param	sClass	Class name to extends.
	 * \return	Register to add properties and methods.
	 */
	template<class O> LuaClass<O> Register(const std::string & sClass);

	/**
	 * Register C++ class into Lua & make it extends from another class.
	 *
	 * \param	sClass	Class name to extends.
	 * \return	Register to add properties and methods.
	 */
	template<class Derived, class Base> LuaClass<Derived> Register(const std::string & sClass);

private:
	LuaVM(const LuaVM &) = delete;
	LuaVM & operator=(const LuaVM &) = delete;

private:
	lua_State * _pL;
};

/**
 * Lua scope guard helper
 */
template<class O>
class LuaGuard {
public:
	/// Store the old one & Set _G[sKey] = new one.
	LuaGuard(const std::string & sKey, O * p) : _sKey(sKey), _p(nullptr) {
		if (GLua.Is<O *>(sKey)) _p = GLua.Get<O *>(sKey);
		GLua.Set<O *>(sKey, p);
	}

	/// Restore the old one. _G[sKey] = old one.
	virtual ~LuaGuard() { GLua.Set<O *>(_sKey, _p); }

private:
	std::string	_sKey;
	O *			_p;
};

///////////////////////////////////////////////////////////////////////////////
/// LuaRef
///////////////////////////////////////////////////////////////////////////////
template<> struct LuaRef<bool> { typedef bool Type; };
template<> struct LuaRef<int8_t> { typedef int8_t Type; };
template<> struct LuaRef<int16_t> { typedef int16_t Type; };
template<> struct LuaRef<int32_t> { typedef int32_t Type; };
template<> struct LuaRef<int64_t> { typedef int64_t Type; };
template<> struct LuaRef<uint8_t> { typedef uint8_t Type; };
template<> struct LuaRef<uint16_t> { typedef uint16_t Type; };
template<> struct LuaRef<uint32_t> { typedef uint32_t Type; };
template<> struct LuaRef<uint64_t> { typedef uint64_t Type; };
template<> struct LuaRef<float> { typedef float Type; };
template<> struct LuaRef<double> { typedef double Type; };
template<> struct LuaRef<const char *> { typedef const char * Type; };
template<> struct LuaRef<std::string> { typedef const std::string & Type; };

///////////////////////////////////////////////////////////////////////////////
/// LuaClassInfo
///////////////////////////////////////////////////////////////////////////////
template<class O> struct LuaClassInfo {
	static std::string	Name;
	static bool			Used;
	static int			RefIdx;
};
template<typename O> std::string LuaClassInfo<O>::Name = "Unknown";
template<typename O> bool LuaClassInfo<O>::Used = false;
template<typename O> int LuaClassInfo<O>::RefIdx = LUA_NOREF;

////////////////////////////////////////////////////////////////////////////////
/// LuaGetter
////////////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaGetter<T *> { static T * Do(lua_State * p, int n) { return *((T **)lua_touserdata(p, n)); } };
template<> struct LuaGetter<bool> { static bool Do(lua_State * p, int n) { return 1 == lua_toboolean(p, n); } };
template<> struct LuaGetter<int8_t> { static int8_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFF; } };
template<> struct LuaGetter<uint8_t> { static uint8_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFF; } };
template<> struct LuaGetter<int16_t> { static int16_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFFFF; } };
template<> struct LuaGetter<uint16_t> { static uint16_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFFFF; } };
template<> struct LuaGetter<int32_t> { static int32_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFFFFFFFF; } };
template<> struct LuaGetter<uint32_t> { static uint32_t Do(lua_State * p, int n) { return lua_tointeger(p, n) & 0xFFFFFFFF; } };
template<> struct LuaGetter<int64_t> { static int64_t Do(lua_State * p, int n) { return (int64_t)lua_tointeger(p, n); } };
template<> struct LuaGetter<uint64_t> { static uint64_t Do(lua_State * p, int n) { return (uint64_t)lua_tointeger(p, n); } };
template<> struct LuaGetter<float> { static float Do(lua_State * p, int n) { return (float)lua_tonumber(p, n); } };
template<> struct LuaGetter<double> { static double Do(lua_State * p, int n) { return lua_tonumber(p, n); } };
template<> struct LuaGetter<const char *> { static const char * Do(lua_State * p, int n) { return lua_tostring(p, n); } };
template<> struct LuaGetter<std::string> { static std::string Do(lua_State * p, int n) { size_t l; const char * s = lua_tolstring(p, n, &l); return std::string(s, l); } };
template<> struct LuaGetter<LuaNil> { static LuaNil Do(lua_State * p, int n) { return GLuaNil; } };
template<> struct LuaGetter<LuaTable> { static LuaTable Do(lua_State * p, int n) { lua_pushvalue(p, n); return LuaTable(p, luaL_ref(p, LUA_REGISTRYINDEX)); } };

////////////////////////////////////////////////////////////////////////////////
/// LuaPusher
////////////////////////////////////////////////////////////////////////////////
template<> struct LuaPusher<bool> { static void Do(lua_State * p, bool b) { lua_pushboolean(p, b ? 1 : 0); } };
template<> struct LuaPusher<int8_t> { static void Do(lua_State * p, int8_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<uint8_t> { static void Do(lua_State * p, uint8_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<int16_t> { static void Do(lua_State * p, int16_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<uint16_t> { static void Do(lua_State * p, uint16_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<int32_t> { static void Do(lua_State * p, int32_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<uint32_t> { static void Do(lua_State * p, uint32_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<int64_t> { static void Do(lua_State * p, int64_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<uint64_t> { static void Do(lua_State * p, uint64_t n) { lua_pushinteger(p, (lua_Integer)n); } };
template<> struct LuaPusher<float> { static void Do(lua_State * p, float n) { lua_pushnumber(p, (lua_Number)n); } };
template<> struct LuaPusher<double> { static void Do(lua_State * p, double n) { lua_pushnumber(p, n); } };
template<> struct LuaPusher<const char *> { static void Do(lua_State * p, const char * s) { lua_pushstring(p, s); } };
template<int N> struct LuaPusher<const char[N]> { static void Do(lua_State * p, const char * s) { lua_pushstring(p, s); } };
template<> struct LuaPusher<std::string> { static void Do(lua_State * p, const std::string & s) { lua_pushlstring(p, s.data(), s.size()); } };
template<> struct LuaPusher<const std::string> { static void Do(lua_State * p, const std::string & s) { lua_pushlstring(p, s.data(), s.size()); } };
template<> struct LuaPusher<LuaNil> { static void Do(lua_State * p, LuaNil & r) { lua_pushnil(p); } };
template<> struct LuaPusher<LuaTable> { static void Do(lua_State * p, LuaTable & r) { r.Push(); } };
template<typename T> struct LuaPusher<T *> {
	static void Do(lua_State * p, T * u) {
		if (u == nullptr) {
			lua_pushnil(p);
			return;
		}

		T ** pData = static_cast<T **>(lua_newuserdata(p, sizeof(T *)));
		*pData = u;
		luaL_getmetatable(p, LuaClassInfo<T>::Name.c_str());
		if (lua_isnil(p, -1)) luaL_error(p, "Class %s NOT registered!!!", LuaClassInfo<T>::Name.c_str());
		lua_setmetatable(p, -2);
	}
};

//////////////////////////////////////////////////////////////////////////
/// LuaName
//////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaName<T *> { static const char * Of() { return LuaClassInfo<T>::Used ? LuaClassInfo<T>::Name.c_str() : "userdata"; } };
template<> struct LuaName<bool> { static const char * Of() { return "boolean"; } };
template<> struct LuaName<int8_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<uint8_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<int16_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<uint16_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<int32_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<uint32_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<int64_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<uint64_t> { static const char * Of() { return "integer"; } };
template<> struct LuaName<float> { static const char * Of() { return "number"; } };
template<> struct LuaName<double> { static const char * Of() { return "number"; } };
template<> struct LuaName<const char *> { static const char * Of() { return "string"; } };
template<> struct LuaName<std::string> { static const char * Of() { return "string"; } };
template<> struct LuaName<LuaNil> { static const char * Of() { return "nil"; } };
template<> struct LuaName<LuaTable> { static const char * Of() { return "table"; } };

//////////////////////////////////////////////////////////////////////////
/// LuaType
//////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaType<T *> { static bool Is(int n) { return n == LUA_TUSERDATA || n == LUA_TLIGHTUSERDATA; } };
template<> struct LuaType<bool> { static bool Is(int n) { return n == LUA_TBOOLEAN; } };
template<> struct LuaType<int8_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<uint8_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<int16_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<uint16_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<int32_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<uint32_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<int64_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<uint64_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<float> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<double> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaType<const char *> { static bool Is(int n) { return n == LUA_TSTRING; } };
template<> struct LuaType<std::string> { static bool Is(int n) { return n == LUA_TSTRING; } };
template<> struct LuaType<LuaNil> { static bool Is(int n) { return n == LUA_TNIL; } };
template<> struct LuaType<LuaTable> { static bool Is(int n) { return n == LUA_TTABLE; } };

//////////////////////////////////////////////////////////////////////////
/// LuaMultiPusher
//////////////////////////////////////////////////////////////////////////
struct LuaMultiPusher {
	template<typename Head, typename ... Tail>
	static void Do(lua_State * p, Head && h, Tail ... tail) {
		LuaPusher<typename std::remove_reference<Head>::type>::Do(p, h);
		Do(p, tail...);
	}

	template<typename Head>
	static void Do(lua_State * p, Head && h) {
		LuaPusher<typename std::remove_reference<Head>::type>::Do(p, h);
	}

	static void Do(lua_State * p) {}
};

//////////////////////////////////////////////////////////////////////////
/// LuaProxy
//////////////////////////////////////////////////////////////////////////
struct LuaProxy {
	static int	Readonly(lua_State * L);
	static int	Index(lua_State * L);
	static int	NewIndex(lua_State * L);
	static int	Method(lua_State * L);

	template<typename T>
	static int Get(lua_State * L) {
		if (!lua_islightuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Get property by address error!");

		T const * pData = (T const *)(lua_touserdata(L, lua_upvalueindex(1)));
		if (!pData) luaL_error(L, "Get property by address error. Pointer missing!!!");

		LuaPusher<T>::Do(L, *pData);
		return 1;
	}

	template<typename T>
	static int Set(lua_State * L) {
		if (!lua_islightuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Set property by address error!");

		T const * pData = (T const *)(lua_touserdata(L, lua_upvalueindex(1)));
		if (!pData) luaL_error(L, "Set property by address error. Pointer missing!!!");

		*pData = LuaGetter<T>::Do(L, 1);
		return 0;
	}

	template<typename T>
	static int Getter(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Get property by function error!");

		typedef T(*GetterFunc)();
		GetterFunc * f = (GetterFunc *)(lua_touserdata(L, lua_upvalueindex(1)));
		if (!f) luaL_error(L, "Get property by function error. Function missing!!!");

		LuaPusher<T>::Do(L, (*f)());
		return 1;
	}

	template<typename T>
	static int Setter(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Set property by function error!");

		typedef void(*SetterFunc)(typename LuaRef<T>::Type);
		SetterFunc * f = (SetterFunc *)(lua_touserdata(L, lua_upvalueindex(1)));
		if (!f) luaL_error(L, "Set property by function error. Function missing!!!");

		(*f)(LuaGetter<T>::Do(L, 1));
		return 0;
	}

	template<class O>
	static int ClassIndex(lua_State * L) {
		if (!lua_isuserdata(L, 1)) return 0;

		lua_getmetatable(L, 1); // tb, key, meta
		while (true) {
			lua_pushvalue(L, 2);	// tb, key, meta, key
			lua_rawget(L, -2);		// tb, key, meta, meta[key]

			if (lua_iscfunction(L, -1) || lua_isfunction(L, -1)) {
				lua_remove(L, -2);	// tb, key, meta[key]
				return 1;
			}

			if (!lua_isnil(L, -1)) {
				int n = lua_type(L, -1);
				lua_pop(L, 2);		// tb, key
				luaL_error(L, "Index of '%s' of class '%s' must be a function but current is '%s'", lua_tostring(L, 2), LuaName<O *>::Of(), lua_typename(L, n));
			}

			lua_pop(L, 1);					// tb, key, meta
			lua_pushstring(L, "__propget");	// tb, key, meta, '__propget'
			lua_rawget(L, -2);				// tb, key, meta, meta.__propget

			if (!lua_istable(L, -1)) {
				lua_pop(L, 2);
				luaL_error(L, "Class '%s' missing '__propget' in metatable!!!", LuaName<O *>::Of());
			}

			lua_pushvalue(L, 2);	// tb, key, meta, meta.__propget, key
			lua_rawget(L, -2);		// tb, key, meta, meta.__propget, meta.__propget[key]
			lua_remove(L, -2);		// tb, key, meta, meta.__propget[key]

			if (lua_iscfunction(L, -1)) {
				lua_pushvalue(L, 1);	// tb, key, meta, meta.__propget[key], tb
				lua_remove(L, -3);		// tb, key, meta.__propget[key], tb
				lua_call(L, 1, 1);
				return 1;
			}

			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);					// tb, key, meta
				lua_pushstring(L, "__parent");	// tb, key, meta, '__parent'
				lua_rawget(L, -2);				// tb, key, meta, meta.__parent
				lua_remove(L, -2);				// tb, key, meta.__parent

				if (lua_istable(L, -1)) continue;
				if (lua_isnil(L, -1)) return 1;

				lua_pop(L, 1);
				lua_pushnil(L);
				return 1;
			} else {
				lua_pop(L, 2);
				luaL_error(L, "Class '%s' bad data in metatable.__propget[%s]!!!", LuaName<O *>::Of(), lua_tostring(L, 2));
			}			
		}
	}

	template<class O>
	static int ClassNewIndex(lua_State * L) {
		if (!lua_isuserdata(L, 1)) return 0;

		lua_getmetatable(L, 1); // tb, k, v, meta
		while (true) {
			lua_pushstring(L, "__propset");	// tb, k, v, meta, '__propset'
			lua_rawget(L, -2);				// tb, k, v, meta, meta.__propset

			if (!lua_istable(L, -1)) {
				lua_pop(L, 2);
				luaL_error(L, "Class '%s' missing metatable.__propset!!!", LuaName<O *>::Of());
			}

			lua_pushvalue(L, 2);	// tb, k, v, meta, meta.__propset, k
			lua_rawget(L, -2);		// tb, k, v, meta, meta.__propset, meta.__propset[k]
			lua_remove(L, -2);		// tb, k, v, meta, meta.__propset[k]

			if (lua_isfunction(L, -1)) {
				lua_remove(L, -2);		// tb, k, v, meta.__propset[k]
				lua_pushvalue(L, 1);	// tb, k, v, meta.__propset[k], tb
				lua_pushvalue(L, 3);	// tb, k, v, meta.__propset[k], tb, v
				lua_call(L, 2, 0);
				break;
			}

			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);					// tb, k, v, meta
				lua_pushstring(L, "__parent");	// tb, k, v, meta, '__parent'
				lua_rawget(L, -2);				// tb, k, v, meta, meta.__parent
				lua_remove(L, -2);				// tb, k, v, meta.__parent

				if (!lua_istable(L, -1)) {
					lua_pop(L, 1);
					luaL_error(L, "Try to set '%s'.'%s' which is not registed!!!", LuaName<O *>::Of(), lua_tostring(L, 2));
				}
			}
		}

		return 0;
	}

	template<class O>
	static int ClassToString(lua_State * L) {
		O ** p = (O **)lua_touserdata(L, 1);
		lua_pushfstring(L, "%s[%p]", LuaName<O *>::Of(), *p);
		return 1;
	}

	template<class O>
	static int ClassMethod(lua_State * L) {
		if (lua_gettop(L) < 1) luaL_error(L, "Class '%s' call function error syntax. Please use ':' instead of '.'!", LuaName<O *>::Of());
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' call function error!", LuaName<O *>::Of());

		typedef int(*ClassExternalFunc)(LuaStack &);

		ClassExternalFunc f = (ClassExternalFunc)(lua_touserdata(L, lua_upvalueindex(1)));
		if (!f) luaL_error(L, "Class '%s' call function error! nil", LuaName<O *>::Of());

		LuaStack _(L, 0);
		return f(_);
	}

	template<class O, typename T>
	static int ClassGet(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' get property by address error!", LuaName<O *>::Of());

		O ** p = (O **)lua_touserdata(L, 1);
		T O::** pData = (T O::**)lua_touserdata(L, lua_upvalueindex(1));
		if (!pData) luaL_error(L, "Class '%s' get property by address error! Missing pointer address", LuaName<O *>::Of());

		LuaPusher<T>::Do(L, (*p)->**pData);
		return 1;
	}

	template<class O, typename T>
	static int ClassSet(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' set property by address error!", LuaName<O *>::Of());

		O ** p = (O **)lua_touserdata(L, 1);
		T O::** pData = (T O::**)lua_touserdata(L, lua_upvalueindex(1));
		if (!pData) luaL_error(L, "Class '%s' set property by address error! Missing pointer address", LuaName<O *>::Of());

		(*p)->**pData = LuaGetter<T>::Do(L, 2);
		return 0;
	}

	template<class O, typename T>
	static int ClassGetter(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' get property by function error!", LuaName<O *>::Of());

		typedef T(O::*ClassGetFunc)();

		O ** p = (O **)lua_touserdata(L, 1);
		ClassGetFunc * f = (ClassGetFunc *)lua_touserdata(L, lua_upvalueindex(1));
		if (!f) luaL_error(L, "Class '%s' get property by function error! nil", LuaName<O *>::Of());

		LuaPusher<T>::Do(L, ((*p)->**f)());
		return 1;
	}

	template<class O, typename T>
	static int ClassConstGetter(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' get property by function error!", LuaName<O *>::Of());

		typedef T(O::*ClassConstGetFunc)() const;

		O ** p = (O **)lua_touserdata(L, 1);
		ClassConstGetFunc * f = (ClassConstGetFunc *)lua_touserdata(L, lua_upvalueindex(1));
		if (!f) luaL_error(L, "Class '%s' get property by function error! nil", LuaName<O *>::Of());

		LuaPusher<T>::Do(L, ((*p)->**f)());
		return 1;
	}

	template<class O, typename T>
	static int ClassSetter(lua_State * L) {
		if (!lua_isuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Class '%s' set property by function error!", LuaName<O *>::Of());

		typedef void(O::*ClassSetFunc)(typename LuaRef<T>::Type);

		O ** p = (O **)lua_touserdata(L, 1);
		ClassSetFunc * f = (ClassSetFunc *)lua_touserdata(L, lua_upvalueindex(1));
		if (!f) luaL_error(L, "Class '%s' set property by function error! nil", LuaName<O *>::Of());

		((*p)->**f)(LuaGetter<T>::Do(L, 2));
		return 0;
	}
};

//////////////////////////////////////////////////////////////////////////
/// LuaStack
//////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaStack::Is(int nIdx) {
	return LuaType<T>::Is(Type(nIdx));
}

template<typename T>
T LuaStack::Get(int nIdx) {
	int n = Type(nIdx);
	if (!LuaType<T>::Is(n)) luaL_error(_pL, "Get index '%d' from stack error. '%s' expected but got '%s'", nIdx, LuaName<T>::Of(), lua_typename(_pL, n));
	return LuaGetter<T>::Do(_pL, nIdx);
}

template<typename T>
void LuaStack::Push(typename LuaRef<T>::Type t) {
	LuaPusher<T>::Do(_pL, t);
}

//////////////////////////////////////////////////////////////////////////
/// LuaTable
//////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaTable::Is(int nIdx) {
	if (!IsValid()) return false;

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_rawgeti(_pL, -1, nIdx);
	bool bCheck = LuaType<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 2);
	return bCheck;
}

template<typename T>
bool LuaTable::Is(const std::string & sName) {
	if (!IsValid()) return false;

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sName.c_str());
	bool bCheck = LuaType<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 2);
	return bCheck;
}

template<typename T>
T LuaTable::Get(int nIdx) {
	if (!IsValid()) luaL_error(_pL, "Try to index an invalid table!");

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_rawgeti(_pL, -1, nIdx);

	int n = lua_type(_pL, -1);
	if (!LuaType<T>::Is(n)) {
		lua_pop(_pL, 2);
		luaL_error(_pL, "Get index '%d' of table. '%s' expected but got '%s'", nIdx, LuaName<T>::Of(), lua_typename(_pL, n));
	}

	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 2);
	return t;
}

template<typename T>
T LuaTable::Get(const std::string & sName) {
	if (!IsValid()) luaL_error(_pL, "Try to index an invalid table!");

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sName.c_str());

	int n = lua_type(_pL, -1);
	if (!LuaType<T>::Is(n)) {
		lua_pop(_pL, 2);
		luaL_error(_pL, "Get index '%s' of table. '%s' expected but got '%s'", sName.c_str(), LuaName<T>::Of(), lua_typename(_pL, n));
	}

	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 2);
	return t;
}

template<typename T>
void LuaTable::Set(int nIdx, typename LuaRef<T>::Type t) {
	if (!IsValid()) luaL_error(_pL, "Try to index an invalid table!");

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	LuaPusher<T>::Do(_pL, t);
	lua_rawseti(_pL, -2, nIdx);
	lua_pop(_pL, 1);
}

template<typename T>
void LuaTable::Set(const std::string & sName, typename LuaRef<T>::Type t) {
	if (!IsValid()) luaL_error(_pL, "Try to index an invalid table!");

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	LuaPusher<T>::Do(_pL, t);
	lua_setfield(_pL, -2, sName.c_str());
	lua_pop(_pL, 1);
}

//////////////////////////////////////////////////////////////////////////
/// LuaNamespace
//////////////////////////////////////////////////////////////////////////
template<typename T>
LuaNamespace & LuaNamespace::Property(const std::string & sProp, T * pData, bool bReadonly /* = true */) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	lua_pushlightuserdata(_pL, pData);
	lua_pushcclosure(_pL, &LuaProxy::Get<T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	if (bReadonly) {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaProxy::Readonly, 1);
	} else {
		lua_pushlightuserdata(_pL, pData);
		lua_pushcclosure(_pL, &LuaProxy::Set<T>, 1);
	}
	lua_rawset(_pL, -3);

	lua_settop(_pL, nTop);
	return *this;
}

template<typename T>
LuaNamespace & LuaNamespace::Property(const std::string & sProp, T (*fGetter)(), void (*fSetter)(typename LuaRef<T>::Type) /* = nullptr */) {
	typedef T(*Getter)();
	typedef void(*Setter)(typename LuaRef<T>::Type);

	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	new (lua_newuserdata(_pL, sizeof(Getter))) Getter(fGetter);
	lua_pushcclosure(_pL, &LuaProxy::Getter<T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	if (fSetter == nullptr) {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaProxy::Readonly, 1);
	} else {
		new (lua_newuserdata(_pL, sizeof(Setter))) Setter(fSetter);
		lua_pushcclosure(_pL, &LuaProxy::Setter<T>, 1);
	}
	lua_rawset(_pL, -3);

	lua_settop(_pL, nTop);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
/// LuaClass
//////////////////////////////////////////////////////////////////////////
template<class O> template<typename T>
LuaClass<O> & LuaClass<O>::Property(const std::string & sProp, T O::*pData, bool bReadonly /* = true */) {
	typedef T O::*U;

	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	new (lua_newuserdata(_pL, sizeof(U))) U(pData);
	lua_pushcclosure(_pL, &LuaProxy::ClassGet<O, T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	if (bReadonly) {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaProxy::Readonly, 1);
	} else {
		new (lua_newuserdata(_pL, sizeof(U))) U(pData);
		lua_pushcclosure(_pL, &LuaProxy::ClassSet<O, T>, 1);
	}
	lua_rawset(_pL, -3);

	lua_settop(_pL, nTop);
	return *this;
}

template<class O> template<typename T>
LuaClass<O> & LuaClass<O>::Property(const std::string & sProp, T (O::*fGetter)(), void (O::*fSetter)(typename LuaRef<T>::Type) /* = nullptr */) {
	typedef T(O::*ClassGetter)();
	typedef void (O::*ClassSetter)(typename LuaRef<T>::Type);

	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	new (lua_newuserdata(_pL, sizeof(ClassGetter))) ClassGetter(fGetter);
	lua_pushcclosure(_pL, &LuaProxy::ClassGetter<O, T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	if (fSetter == nullptr) {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaProxy::Readonly, 1);
	} else {
		new (lua_newuserdata(_pL, sizeof(ClassSetter))) ClassSetter(fSetter);
		lua_pushcclosure(_pL, &LuaProxy::ClassSetter<O, T>, 1);
	}
	lua_rawset(_pL, -3);

	lua_settop(_pL, nTop);
	return *this;
}

template<class O> template<typename T>
LuaClass<O> & LuaClass<O>::Property(const std::string & sProp, T (O::*fGetter)() const, void (O::*fSetter)(typename LuaRef<T>::Type) /* = nullptr */) {
	typedef T(O::*ClassGetter)() const;
	typedef void (O::*ClassSetter)(typename LuaRef<T>::Type);

	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	new (lua_newuserdata(_pL, sizeof(ClassGetter))) ClassGetter(fGetter);
	lua_pushcclosure(_pL, &LuaProxy::ClassGetter<O, T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on class without metatable!!!");
	}

	lua_pushstring(_pL, sProp.c_str());
	if (fSetter == nullptr) {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaProxy::Readonly, 1);
	} else {
		new (lua_newuserdata(_pL, sizeof(ClassSetter))) ClassSetter(fSetter);
		lua_pushcclosure(_pL, &LuaProxy::ClassSetter<O, T>, 1);
	}
	lua_rawset(_pL, -3);

	lua_settop(_pL, nTop);
	return *this;
}

template<class O>
LuaClass<O> & LuaClass<O>::Method(const std::string & sMethod, int (*fOpt)(LuaStack &)) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_pushstring(_pL, sMethod.c_str());
	lua_pushlightuserdata(_pL, (void *)fOpt);
	lua_pushcclosure(_pL, &LuaProxy::ClassMethod<O>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
/// LuaVM
//////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaVM::Is(const std::string & sName) {
	lua_getglobal(_pL, sName.c_str());
	bool bCheck = LuaType<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 1);
	return bCheck;
}

template<typename T>
T LuaVM::Get(const std::string & sName) {
	lua_getglobal(_pL, sName.c_str());
	int n = lua_type(_pL, -1);
	if (!LuaType<T>::Is(n)) {
		lua_pop(_pL, 1);
		luaL_error(_pL, "Get global '%s' error. '%s' expected but got '%s'", sName.c_str(), LuaName<T>::Of(), lua_typename(_pL, n));
	}
	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 1);
	return t;
}

template<typename T>
void LuaVM::Set(const std::string & sName, typename LuaRef<T>::Type t) {
	LuaPusher<T>::Do(_pL, t);
	lua_setglobal(_pL, sName.c_str());
}

template<typename ... Args>
LuaStack LuaVM::Call(const std::string & sTable, const std::string & sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	try {
		lua_getglobal(_pL, sTable.c_str());
		if (!lua_istable(_pL, -1)) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Call %s.%s() error, %s is not a table!", sTable.c_str(), sFunc.c_str(), sTable.c_str());
		}

		lua_getfield(_pL, -1, sFunc.c_str());
		lua_remove(_pL, -2);
		if (!lua_isfunction(_pL, -1)) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Call %s.%s() error, %s is not a function!", sTable.c_str(), sFunc.c_str(), sFunc.c_str());
		}

		LuaMultiPusher::Do(_pL, args...);

		if (lua_pcall(_pL, sizeof ... (Args), LUA_MULTRET, nTop + 1) != 0) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}

		lua_remove(_pL, nTop + 1);
		return LuaStack(_pL, lua_gettop(_pL) - nTop);
	} catch (...) {
		lua_settop(_pL, nTop);
		return LuaStack(_pL, 0);
	}
}

template<typename ... Args>
LuaStack LuaVM::SelfCall(const std::string & sTable, const std::string & sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	try {
		lua_getglobal(_pL, sTable.c_str());
		if (!lua_istable(_pL, -1)) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Call %s.%s() error, %s is not a table!", sTable.c_str(), sFunc.c_str(), sTable.c_str());
		}

		lua_getfield(_pL, -1, sFunc.c_str());
		if (!lua_isfunction(_pL, -1)) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Call %s.%s() error, %s is not a function!", sTable.c_str(), sFunc.c_str(), sFunc.c_str());
		}

		lua_pushvalue(_pL, -2);
		lua_remove(_pL, -3);
		LuaMultiPusher::Do(_pL, args...);

		if (lua_pcall(_pL, sizeof ... (Args) + 1, LUA_MULTRET, nTop + 1) != 0) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}

		lua_remove(_pL, nTop + 1);
		return LuaStack(_pL, lua_gettop(_pL) - nTop);
	} catch (...) {
		lua_settop(_pL, nTop);
		return LuaStack(_pL, 0);
	}
}

template<class O>
LuaClass<O> LuaVM::Register(const std::string & sClass) {
	if (LuaClassInfo<O>::Used) {
		if (LuaClassInfo<O>::Name != sClass) {
			luaL_error(_pL, "Class '%s' has already registered with key '%s'!!!", sClass.c_str(), LuaClassInfo<O>::Name.c_str());
		} else {
			return LuaClass<O>(_pL, LuaClassInfo<O>::RefIdx);
		}
	}

	if (luaL_newmetatable(_pL, sClass.c_str()) == 0)
		luaL_error(_pL, "Bad key for register class. '%s' already exists with type '%s'!!!", sClass.c_str(), lua_typename(_pL, lua_type(_pL, -1)));
	
	lua_pushstring(_pL, "__index");
	lua_pushcfunction(_pL, &LuaProxy::ClassIndex<O>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__newindex");
	lua_pushcfunction(_pL, &LuaProxy::ClassNewIndex<O>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__tostring");
	lua_pushcfunction(_pL, &LuaProxy::ClassToString<O>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propget");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propset");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	int nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);

	LuaClassInfo<O>::Name	= sClass;
	LuaClassInfo<O>::Used	= true;
	LuaClassInfo<O>::RefIdx	= nRef;

	return LuaClass<O>(_pL, nRef);
}

template<class Derived, class Base>
LuaClass<Derived> LuaVM::Register(const std::string & sClass) {
	if (!LuaClassInfo<Base>::Used) {
		luaL_error(_pL, "You should register base class before derived['%s']!!!", sClass.c_str());
	}

	if (LuaClassInfo<Derived>::Used) {
		if (LuaClassInfo<Derived>::Name != sClass) {
			luaL_error(_pL, "Class '%s' has already registered with key '%s'!!!", sClass.c_str(), LuaClassInfo<Derived>::Name.c_str());
		} else {
			return LuaClass<Derived>(_pL, LuaClassInfo<Derived>::RefIdx);
		}
	}

	if (luaL_newmetatable(_pL, sClass.c_str()) == 0)
		luaL_error(_pL, "Bad key for register class. '%s' already exists with type '%s'!!!", sClass.c_str(), lua_typename(_pL, lua_type(_pL, -1)));
	
	lua_pushstring(_pL, "__index");
	lua_pushcfunction(_pL, &LuaProxy::ClassIndex<Derived>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__newindex");
	lua_pushcfunction(_pL, &LuaProxy::ClassNewIndex<Derived>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__tostring");
	lua_pushcfunction(_pL, &LuaProxy::ClassToString<Derived>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propget");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propset");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__parent");
	luaL_getmetatable(_pL, LuaClassInfo<Base>::Name.c_str());
	lua_rawset(_pL, -3);

	int nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);

	LuaClassInfo<Derived>::Name		= sClass;
	LuaClassInfo<Derived>::Used		= true;
	LuaClassInfo<Derived>::RefIdx	= nRef;

	return LuaClass<Derived>(_pL, nRef);
}

#endif//!	__ENGINE_SCRIPT_H_INCLUDED__
