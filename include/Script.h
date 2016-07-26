#ifndef		__ENGINE_SCRIPT_H_INCLUDED__
#define		__ENGINE_SCRIPT_H_INCLUDED__

extern "C" {
#	include		"Lua/luajit.h"
#	include		"Lua/lualib.h"
#	include		"Lua/lauxlib.h"
}

#include	<functional>
#include	<cstdint>
#include	<cstring>
#include	<string>
#include	<vector>

#define		LUA(code)	#code
#define		GLua		LuaScript::Instance()

/**
 * Reference type for template programing.
 **/
template<typename T> struct LuaRefOf { typedef T Type; };
template<> struct LuaRefOf<std::string> { typedef const std::string & Type; };
template<> struct LuaRefOf<class LuaNil> { typedef LuaNil & Type; };
template<> struct LuaRefOf<class LuaTable> { typedef LuaTable & Type; };

/**
 * Global preference setting of LUA.
 **/
struct LuaPreference {
	static std::function<void (const std::string &)> HookError;
};

/**
 * LUA nil type in C++
 **/
class LuaNil {};
extern LuaNil GLuaNil;

/**
 * Stack operator for LUA.
 **/
class LuaState {
	friend class LuaTable;

public:
	LuaState(lua_State * pL, int nCleanTop) : _pL(pL), _nCleanTop(nCleanTop) {}
	virtual ~LuaState();

	int		Top();
	void	Pop(int nCount);
	int		Type(int nIdx);

	template<typename T>
	bool	Is(int nIdx);

	template<typename T>
	T		Get(int nIdx);

	template<typename T>
	void	Push(typename LuaRefOf<T>::Type t);

private:
	lua_State *	_pL;
	int			_nCleanTop;
};

/**
 * LUA table in C++.
 **/
class LuaTable {
public:
	LuaTable(lua_State * pL, int nRef) : _pL(pL), _nRef(nRef) {}
	LuaTable(const LuaTable & r);
	LuaTable(LuaState & r);
	virtual ~LuaTable() { Unref(); }

	LuaTable & operator=(const LuaTable & r);

	void	Push();
	void	Unref();

	template<typename T>
	T	Get(int nIdx);

	template<typename T>
	T	Get(const char * sKey);

	template<typename T>
	void	Set(int nIdx, typename LuaRefOf<T>::Type t);

	template<typename T>
	void	Set(const char * sKey, typename LuaRefOf<T>::Type t);

	template<typename ... Args>
	LuaState	Call(const char * sFunc, Args && ... args);

	template<typename ... Args>
	LuaState	SelfCall(const char * sFunc, Args && ... args);

protected:
	lua_State *	_pL;
	int			_nRef;
};

/**
 * Register for global properties or methods.
 **/
class LuaRegister : public LuaTable {
public:
	LuaRegister(lua_State * pL, int nRef) : LuaTable(pL, nRef) {}

	template<typename T>
	LuaRegister &	Property(const char * sProp, T * pData, bool bWritable = true);

	template<typename T>
	LuaRegister &	Property(const char * sProp, T (*fGetter)(), void (*fSetter)(typename LuaRefOf<T>::Type) = 0);

	LuaRegister &	Method(const char * sMethod, int (*fOpt)(LuaState &));
};

/**
 * Register for class properties or methods.
 **/
template<class O>
class LuaRegisterClass : public LuaTable {
public:
	LuaRegisterClass(lua_State * pL, int nRef, const char * sClass, bool bNeedInit);

	template<typename T>
	LuaRegisterClass<O> &	Property(const char * sProp, T O::*pData, bool bWritable = true);

	template<typename T>
	LuaRegisterClass<O> &	Property(const char * sProp, T (O::*fGetter)(), void (O::*fSetter)(typename LuaRefOf<T>::Type) = 0);

	LuaRegisterClass<O> &	Method(const char * sMethod, int (O::*fOpt)(LuaState &));
};

/**
 * High level interface for LUA.
 **/
class LuaScript {
public:
	LuaScript();
	virtual ~LuaScript();

	static LuaScript &	Instance();

	void		DoFile(const char * sFile, bool bRequireOnce = true, bool bRestoreStack = true);
	void		Run(const char * sCode);
	LuaState	Stack();
	void		GC();

	template<typename T>
	T			Get(const char * sName);

	template<typename T>
	void		Set(const char * sName, typename LuaRefOf<T>::Type t);

	template<typename ... Args>
	LuaState	Call(const char * sTable, const char * sFunc, Args && ... args);

	template<typename ... Args>
	LuaState	SelfCall(const char * sTable, const char * sFunc, Args && ... args);

	template<typename T>
	LuaRegisterClass<T>			Register(const char * sClass);

	template<typename Derived, typename Base>
	LuaRegisterClass<Derived>	Register(const char * sClass);

	LuaRegister					Register(const char * sNamespace);

private:
	LuaScript(const LuaScript &) = delete;
	LuaScript & operator=(const LuaScript &) = delete;

private:
	lua_State *					_pL;
	std::vector<std::string>	_vLoaded;
};

#include	"Script.inl"

#endif//!	__ENGINE_SCRIPT_H_INCLUDED__