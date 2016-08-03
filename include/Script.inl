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

///////////////////////////////////////////////////////////////////////////////
/// LuaTypeName
///////////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaTypeName { static const char * Get() { return "none"; } };
template<typename T> struct LuaTypeName<T *> { static const char * Get() { return "userdata"; } };
template<> struct LuaTypeName<bool> { static const char * Get() { return "boolean"; } };
template<> struct LuaTypeName<int8_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<uint8_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<int16_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<uint16_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<int32_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<uint32_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<int64_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<uint64_t> { static const char * Get() { return "integer"; } };
template<> struct LuaTypeName<float> { static const char * Get() { return "number"; } };
template<> struct LuaTypeName<double> { static const char * Get() { return "number"; } };
template<> struct LuaTypeName<const char *> { static const char * Get() { return "string"; } };
template<> struct LuaTypeName<std::string> { static const char * Get() { return "string"; } };
template<> struct LuaTypeName<LuaNil> { static const char * Get() { return "nil"; } };
template<> struct LuaTypeName<LuaTable> { static const char * Get() { return "table"; } };

///////////////////////////////////////////////////////////////////////////////
/// LuaTypeAssert
///////////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaTypeAssert { static bool Is(int n) { return false; } };
template<typename T> struct LuaTypeAssert<T *> { static bool Is(int n) { return n == LUA_TUSERDATA || n == LUA_TLIGHTUSERDATA; } };
template<> struct LuaTypeAssert<bool> { static bool Is(int n) { return n == LUA_TBOOLEAN; } };
template<> struct LuaTypeAssert<int8_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<uint8_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<int16_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<uint16_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<int32_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<uint32_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<int64_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<uint64_t> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<float> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<double> { static bool Is(int n) { return n == LUA_TNUMBER; } };
template<> struct LuaTypeAssert<const char *> { static bool Is(int n) { return n == LUA_TSTRING; } };
template<> struct LuaTypeAssert<std::string> { static bool Is(int n) { return n == LUA_TSTRING; } };
template<> struct LuaTypeAssert<LuaNil> { static bool Is(int n) { return n == LUA_TNIL; } };
template<> struct LuaTypeAssert<LuaTable> { static bool Is(int n) { return n == LUA_TTABLE; } };

///////////////////////////////////////////////////////////////////////////////
/// LuaGetter
///////////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaGetter {};
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

///////////////////////////////////////////////////////////////////////////////
/// LuaPusher
///////////////////////////////////////////////////////////////////////////////
template<typename T> struct LuaPusher {};
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
template<> struct LuaPusher<LuaNil> { static void Do(lua_State * p, LuaNil & r) { lua_pushnil(p); } };
template<> struct LuaPusher<LuaTable> { static void Do(lua_State * p, LuaTable & r) { r.Push(); } };
template<typename T> struct LuaPusher<T *> {
	static void Do(lua_State * p, T * u) {
		T ** pData = static_cast<T **>(lua_newuserdata(p, sizeof(T *)));
		*pData = u;
		luaL_getmetatable(p, LuaClassInfo<T>::Name.c_str());
		if (lua_isnil(p, -1)) luaL_error(p, "Class %s NOT registered!!!", LuaClassInfo<T>::Name.c_str());
		lua_setmetatable(p, -2);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// LuaMultPusher
///////////////////////////////////////////////////////////////////////////////
struct LuaMultPusher {
	template<typename Head, typename ... Tail>
	static void Do(lua_State * p, Head && h, Tail ... args) {
		LuaPusher<typename std::remove_reference<Head>::type>::Do(p, h);
		Do(p, args...);
	}

	template<typename Head>
	static void Do(lua_State * p, Head && h) {
		LuaPusher<typename std::remove_reference<Head>::type>::Do(p, h);
	}

	static void Do(lua_State * p) {
	}
};

///////////////////////////////////////////////////////////////////////////////
/// LuaMetatableProxy
///////////////////////////////////////////////////////////////////////////////
struct LuaMetatableProxy {
	static int	Readonly(lua_State * pL);
	static int	Include(lua_State * pL);
	static int	Extends(lua_State * pL);
	static int	Index(lua_State * pL);
	static int	NewIndex(lua_State * pL);
	static int	Func(lua_State * pL);

	template<typename T>
	static int	Get(lua_State * pL) {
		if (!lua_islightuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Get property error!");

		T const * pData = static_cast<T const *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!pData) luaL_error(pL, "Get property error! nil");

		LuaPusher<T>::Do(pL, *pData);
		return 1;
	}

	template<typename T>
	static int	Set(lua_State * pL) {
		if (!lua_islightuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Set property error!");

		T const * pData = static_cast<T const *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!pData) luaL_error(pL, "Set property error! nil");

		*pData = LuaGetter<T>::Do(pL, 1);
		return 0;
	}

	template<typename T>
	static int	Getter(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Get property by function error!");

		typedef T(*GetFunc)();
		GetFunc * f = static_cast<GetFunc *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!f) luaL_error(pL, "Get property by function error! nil");

		LuaPusher<T>::Do(pL, (*f)());
		return 1;
	}

	template<typename T>
	static int	Setter(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Set property by function error!");

		typedef void (*SetFunc)(typename LuaRefOf<T>::Type t);
		SetFunc * f = static_cast<SetFunc *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!f) luaL_error(pL, "Set property by function error! nil");

		T t = LuaGetter<T>::Do(pL, 1);
		(*f)(t);
		return 0;
	}

	template<class O>
	static int	ClassIndex(lua_State * pL) {
		if (!lua_isuserdata(pL, 1)) return 0;

		lua_getmetatable(pL, 1);	// tb, key, meta

		while (true) {
			lua_pushvalue(pL, 2);		// tb, key, meta, key
			lua_rawget(pL, -2);			// tb, key, meta, meta[key]

			if (lua_iscfunction(pL, -1) || lua_isfunction(pL, -1)) {
				lua_remove(pL, -2);		// tb, key, meta[key]
				return 1;
			}

			if (!lua_isnil(pL, -1)) {
				lua_pop(pL, 2);
				luaL_error(pL, "Index '%s' of class '%s' must be a function but current is '%s'", lua_tostring(pL, 2), LuaClassInfo<O>::Name.c_str(), lua_typename(pL, lua_type(pL, -1)));
			}

			lua_pop(pL, 1);						// tb, key, meta
			lua_pushstring(pL, "__propget");	// tb, key, meta, "__propget"
			lua_rawget(pL, -2);					// tb, key, meta, meta.__propget

			if (lua_istable(pL, -1)) {
				lua_pushvalue(pL, 2);			// tb, key, meta, meta.__propget, key
				lua_rawget(pL, -2);				// tb, key, meta, meta.__propget, meta.__propget.key
				lua_remove(pL, -2);				// tb, key, meta, meta.__propget.key

				if (lua_iscfunction(pL, -1)) {
					lua_pushvalue(pL, 1);		// tb, key, meta, meta.__propget.key, tb
					lua_remove(pL, -3);			// tb, key, meta.__propget.key, tb
					lua_call(pL, 1, 1);
					return 1;
				}

				if (lua_isnil(pL, -1)) {
					lua_pop(pL, 1);						// tb, key, meta
					lua_pushstring(pL, "__parent");		// tb, key, meta, "__parent"
					lua_rawget(pL, -2);					// tb, key, meta, meta.__parent
					lua_remove(pL, -2);					// tb, key, meta.__parent
					if (lua_istable(pL, -1)) continue;
					if (lua_isnil(pL, -1)) return 1;

					lua_pop(pL, 1);
					luaL_error(pL, "Index '%s' of class '%s' must be a function, but current is '%s'!", lua_tostring(pL, 2), LuaClassInfo<O>::Name.c_str(), lua_typename(pL, lua_type(pL, -1)));
				}
			} else {
				lua_pop(pL, 2);
				luaL_error(pL, "Class '%s' missing __propget!!!", LuaClassInfo<O>::Name.c_str());
			}
		}		
	}

	template<class O>
	static int	ClassNewIndex(lua_State * pL) {
		if (!lua_isuserdata(pL, 1)) return 0;

		lua_getmetatable(pL, 1);				// tb, key, value, meta
		while(true) {
			lua_pushstring(pL, "__propset");	// tb, key, value, meta, "__propset"
			lua_rawget(pL, -2);					// tb, key, value, meta, meta.__propset
			if (lua_istable(pL, -1)) {
				lua_pushvalue(pL, 2);			// tb, key, value, meta, meta.__propset, key
				lua_rawget(pL, -2);				// tb, key, value, meta, meta.__propset, meta.__propset.key
				lua_remove(pL, -2);				// tb, key, value, meta, meta.__propset.key

				if (lua_isfunction(pL, -1)) {
					lua_remove(pL, -2);			// tb, key, value, meta.__propset.key
					lua_pushvalue(pL, 1);		// tb, key, value, meta.__propset.key, tb
					lua_pushvalue(pL, 3);		// tb, key, value, meta.__propset.key, tb, value
					lua_call(pL, 2, 0);
					break;
				} else if (lua_isnil(pL, -1)) {
					lua_pop(pL, 1);						// tb, key, value, meta
					lua_pushstring(pL, "__parent");		// tb, key, value, meta, "__parent"
					lua_rawget(pL, -2);					// tb, key, value, meta, meta.__parent
					lua_remove(pL, -2);					// tb, key, meta.__parent
					if (!lua_istable(pL, -1)) {
						lua_pop(pL, 1);
						luaL_error(pL, "Try set index '%s' of class '%s' which is a nil value!!!", lua_tostring(pL, 2), LuaClassInfo<O>::Name.c_str());
					} else {
						continue;
					}
				}
			} else {
				lua_pop(pL, 2);
				luaL_error(pL, "Class '%s' missing __propset!!!", LuaClassInfo<O>::Name.c_str());
			}
		}

		return 0;
	}

	template<class O>
	static int	ToString(lua_State * pL) {
		O ** p = static_cast<O **>(lua_touserdata(pL, 1));
		lua_pushfstring(pL, "%s[%p]", LuaClassInfo<O>::Name.c_str(), *p);
		return 1;
	}

	template<class O>
	static int	ClassFunc(lua_State * pL) {
		if (lua_gettop(pL) < 1) luaL_error(pL, "Class '%s' call function error syntax. Please use ':' instead of '.'!", LuaClassInfo<O>::Name.c_str());
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Class '%s' call function error!", LuaClassInfo<O>::Name.c_str());

		typedef int (O::*ClassCFunc)(LuaState &);

		ClassCFunc * f = static_cast<ClassCFunc *>(lua_touserdata(pL, lua_upvalueindex(1)));
		O ** p = static_cast<O **>(lua_touserdata(pL, 1));

		if (!f) luaL_error(pL, "Class '%s' call function error! nil", LuaClassInfo<O>::Name.c_str());

		LuaState i(pL, 0);
		return ((*p)->**f)(i);
	}

	template<class O, typename T>
	static int	ClassGet(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Class '%s' get property error!", LuaClassInfo<O>::Name.c_str());
		
		O ** p = static_cast<O **>(lua_touserdata(pL, 1));
		T O::** pData = static_cast<T O::**>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!pData) luaL_error(pL, "Class '%s' get property error! nil", LuaClassInfo<O>::Name.c_str());

		LuaPusher<T>::Do(pL, (*p)->**pData);
		return 1;		
	}

	template<class O, typename T>
	static int	ClassSet(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Class '%s' set property error!", LuaClassInfo<O>::Name.c_str());
		
		O ** p = static_cast<O **>(lua_touserdata(pL, 1));
		T O::** pData = static_cast<T O::**>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!pData) luaL_error(pL, "Class '%s' set property error! nil", LuaClassInfo<O>::Name.c_str());

		(*p)->**pData = LuaGetter<T>::Do(pL, 2);
		return 0;
	}

	template<class O, typename T>
	static int	ClassGetter(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Class '%s' get property by function error!", LuaClassInfo<O>::Name.c_str());
		
		typedef T (O::*ClassGetFunc)();

		O ** p = static_cast<O **>(lua_touserdata(pL, 1));
		ClassGetFunc * f = static_cast<ClassGetFunc *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!f) luaL_error(pL, "Class '%s' get property by function error! nil", LuaClassInfo<O>::Name.c_str());

		LuaPusher<T>::Do(pL, ((*p)->**f)());
		return 1;
	}

	template<class O, typename T>
	static int	ClassSetter(lua_State * pL) {
		if (!lua_isuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Class '%s' set property by function error!", LuaClassInfo<O>::Name.c_str());
		
		typedef void (O::*ClassSetFunc)(typename LuaRefOf<T>::Type);

		O ** p = static_cast<O **>(lua_touserdata(pL, 1));
		ClassSetFunc * f = static_cast<ClassSetFunc *>(lua_touserdata(pL, lua_upvalueindex(1)));
		if (!f) luaL_error(pL, "Class '%s' set property by function error! nil", LuaClassInfo<O>::Name.c_str());

		T t = LuaGetter<T>::Do(pL, 2);
		((*p)->**f)(t);
		return 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// Implement of LuaState
///////////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaState::Is(int nIdx) {
	return LuaTypeAssert<T>::Is(Type(nIdx));
}

template<typename T>
T LuaState::Get(int nIdx) {
	return LuaGetter<T>::Do(_pL, nIdx);
}

template<typename T>
void LuaState::Push(typename LuaRefOf<T>::Type t) {
	LuaPusher<T>::Do(_pL, t);
}

///////////////////////////////////////////////////////////////////////////////
/// Implement of LuaTable
///////////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaTable::Is(int nIdx) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_rawgeti(_pL, -1, nIdx);

	bool bType = LuaTypeAssert<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 2);
	return bType;
}

template<typename T>
bool LuaTable::Is(const char * sName) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sName);

	bool bType = LuaTypeAssert<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 2);
	return bType;
}

template<typename T>
T LuaTable::Get(int nIdx) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_rawgeti(_pL, -1, nIdx);

	int nType = lua_type(_pL, -1);
	if (!LuaTypeAssert<T>::Is(nType)) {
		lua_pop(_pL, 2);
		luaL_error(_pL, "Get index '%d' from table error. '%s' expected but got '%s'!!!", nIdx, LuaTypeName<T>::Get(), lua_typename(_pL, nType));
	}

	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 2);
	return t;
}

template<typename T>
T LuaTable::Get(const char * sName) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sName);

	int nType = lua_type(_pL, -1);
	if (!LuaTypeAssert<T>::Is(nType)) {
		lua_pop(_pL, 2);
		luaL_error(_pL, "Get index '%s' from table error. '%s' expected but got '%s'!!!", sName, LuaTypeName<T>::Get(), lua_typename(_pL, nType));
	}

	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 2);
	return t;
}

template<typename T>
void LuaTable::Set(int nIdx, typename LuaRefOf<T>::Type t) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	LuaPusher<T>::Do(_pL, t);
	lua_rawseti(_pL, -2, nIdx);
	lua_pop(_pL, 1);
}

template<typename T>
void LuaTable::Set(const char * sName, typename LuaRefOf<T>::Type t) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	LuaPusher<T>::Do(_pL, t);
	lua_setfield(_pL, -2, sName);
	lua_pop(_pL, 1);
}

template<typename ... Args>
LuaState LuaTable::Call(const char * sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sFunc);
	lua_remove(_pL, -2);

	try {
		int nType = lua_type(_pL, -1);
		if (nType == LUA_TNIL) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Attempt to call function '%s' which is not exists!!!", sFunc);
		} else if (nType != LUA_TFUNCTION) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Attempt to call '%s' which is not function but '%s'!!!", sFunc, lua_typename(_pL, nType));
		}

		LuaMultPusher::Do(_pL, args...);

		if (lua_pcall(_pL, sizeof ... (Args), -1, nTop + 1) != 0) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}

		lua_remove(_pL, nTop + 1);
		return LuaState(_pL, lua_gettop(_pL) - nTop);
	} catch (...) {
		return LuaState(_pL, 0);
	}	
}

template<typename ... Args>
LuaState LuaTable::SelfCall(const char * sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_getfield(_pL, -1, sFunc);
	lua_remove(_pL, -2);

	try {
		int nType = lua_type(_pL, -1);
		if (nType == LUA_TNIL) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Attempt to call function '%s' which is not exists!!!", sFunc);
		} else if (nType != LUA_TFUNCTION) {
			lua_settop(_pL, nTop);
			luaL_error(_pL, "Attempt to call '%s' which is not function but '%s'!!!", sFunc, lua_typename(_pL, nType));
		}

		lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
		LuaMultPusher::Do(_pL, args...);

		if (lua_pcall(_pL, sizeof ... (Args) + 1, -1, nTop + 1) != 0) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}

		lua_remove(_pL, nTop + 1);
		return LuaState(_pL, lua_gettop(_pL) - nTop);
	} catch (...) {
		return LuaState(_pL, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Implement of LuaRegister
///////////////////////////////////////////////////////////////////////////////
template<typename T>
LuaRegister & LuaRegister::Property(const char * sProp, T * pData, bool bWritable) {	
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);

	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp);
	lua_pushlightuserdata(_pL, pData);
	lua_pushcclosure(_pL, &LuaMetatableProxy::Get<T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp);
	if (bWritable) {
		lua_pushlightuserdata(_pL, pData);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Set<T>, 1);
	} else {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Readonly, 1);
	}

	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

template<typename T>
LuaRegister & LuaRegister::Property(const char * sProp, T (*fGetter)(), void (*fSetter)(typename LuaRefOf<T>::Type)) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);

	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	typedef T(*Getter)();

	lua_pushstring(_pL, sProp);
	new (lua_newuserdata(_pL, sizeof(Getter))) Getter(fGetter);
	lua_pushcclosure(_pL, &LuaMetatableProxy::Getter<T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a table without metatable!!!");
	}

	lua_pushstring(_pL, sProp);
	if (fSetter) {
		typedef void (*Setter)(typename LuaRefOf<T>::Type);
		new (lua_newuserdata(_pL, sizeof(Setter))) Setter(fSetter);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Setter<T>, 1);
	} else {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Readonly, 1);
	}

	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/// Implement of LuaRegisterClass
///////////////////////////////////////////////////////////////////////////////
template<typename O>
LuaRegisterClass<O>::LuaRegisterClass(lua_State * pL, int nRef, const char * sClass, bool bNeedInit)
	: LuaTable(pL, nRef) {
	if (bNeedInit) {
		LuaClassInfo<O>::Name	= sClass;
		LuaClassInfo<O>::RefIdx	= nRef;
		LuaClassInfo<O>::Used	= true;
	}
}

template<typename O> template<typename T>
LuaRegisterClass<O> & LuaRegisterClass<O>::Property(const char * sProp, T O::*pData, bool bWritable) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);

	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a class without metatable!!!");
	}

	typedef T O::*U;

	lua_pushstring(_pL, sProp);
	new (lua_newuserdata(_pL, sizeof(U))) U(pData);
	lua_pushcclosure(_pL, &LuaMetatableProxy::ClassGet<O, T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a class without metatable!!!");
	}

	lua_pushstring(_pL, sProp);
	if (bWritable) {
		new (lua_newuserdata(_pL, sizeof(U))) U(pData);
		lua_pushcclosure(_pL, &LuaMetatableProxy::ClassSet<O, T>, 1);
	} else {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Readonly, 1);
	}

	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

template<typename O> template<typename T>
LuaRegisterClass<O> & LuaRegisterClass<O>::Property(const char * sProp, T (O::*fGetter)(), void (O::*fSetter)(typename LuaRefOf<T>::Type)) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);

	lua_getfield(_pL, -1, "__propget");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a class without metatable!!!");
	}

	typedef T (O::*ClassGetter)();

	lua_pushstring(_pL, sProp);
	new (lua_newuserdata(_pL, sizeof(ClassGetter))) ClassGetter(fGetter);
	lua_pushcclosure(_pL, &LuaMetatableProxy::ClassGetter<O, T>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);

	lua_getfield(_pL, -1, "__propset");
	if (!lua_istable(_pL, -1)) {
		lua_settop(_pL, nTop);
		luaL_error(_pL, "Try to add property on a class without metatable!!!");
	}

	lua_pushstring(_pL, sProp);
	if (fSetter) {
		typedef void (O::*ClassSetter)(typename LuaRefOf<T>::Type);
		new (lua_newuserdata(_pL, sizeof(ClassSetter))) ClassSetter(fSetter);
		lua_pushcclosure(_pL, &LuaMetatableProxy::ClassSetter<O, T>, 1);
	} else {
		lua_pushvalue(_pL, -1);
		lua_pushcclosure(_pL, &LuaMetatableProxy::Readonly, 1);
	}

	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

template<typename O>
LuaRegisterClass<O> & LuaRegisterClass<O>::Method(const char * sMethod, int (O::*fOpt)(LuaState &)) {
	typedef int (O::*ClassMemFunc)(LuaState &);
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_pushstring(_pL, sMethod);
	new (lua_newuserdata(_pL, sizeof(ClassMemFunc))) ClassMemFunc(fOpt);
	lua_pushcclosure(_pL, &LuaMetatableProxy::ClassFunc<O>, 1);
	lua_rawset(_pL, -3);
	lua_pop(_pL, 1);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/// Implement of LuaScript
///////////////////////////////////////////////////////////////////////////////
template<typename T>
bool LuaScript::Is(const char * sName) {
	lua_getglobal(_pL, sName);

	bool bType = LuaTypeAssert<T>::Is(lua_type(_pL, -1));
	lua_pop(_pL, 1);
	return bType;
}

template<typename T>
T LuaScript::Get(const char * sName) {
	lua_getglobal(_pL, sName);

	int nType = lua_type(_pL, -1);
	if (!LuaTypeAssert<T>::Is(nType)) {
		lua_pop(_pL, 1);
		luaL_error(_pL, "Get global '%s' error. '%s' expected but got '%s'!!!", sName, LuaTypeName<T>::Get(), lua_typename(_pL, nType));
	}

	T t = LuaGetter<T>::Do(_pL, -1);
	lua_pop(_pL, 1);
	return t;
}

template<typename T>
void LuaScript::Set(const char * sName, typename LuaRefOf<T>::Type t) {
	LuaPusher<T>::Do(_pL, t);
	lua_setglobal(_pL, sName);
}

template<typename ... Args>
LuaState LuaScript::Call(const char * sTable, const char * sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	try {
		return Get<LuaTable>(sTable).Call(sFunc, args...);
	} catch (...) {
		lua_settop(_pL, nTop);
		return LuaState(_pL, 0);
	}
}

template<typename ... Args>
LuaState LuaScript::SelfCall(const char * sTable, const char * sFunc, Args && ... args) {
	int nTop = lua_gettop(_pL);

	try {
		return Get<LuaTable>(sTable).SelfCall(sFunc, args...);
	} catch (...) {
		lua_settop(_pL, nTop);
		return LuaState(_pL, 0);
	}
}

template<typename T>
LuaRegisterClass<T> LuaScript::Register(const char * sClass) {
	if (LuaClassInfo<T>::Used) {
		if (LuaClassInfo<T>::Name.compare(sClass) != 0) {
			luaL_error(_pL, "Class '%s' has already registered with key '%s'!!!", sClass, LuaClassInfo<T>::Name.c_str());
		} else {
			return LuaRegisterClass<T>(_pL, LuaClassInfo<T>::RefIdx, sClass, false);
		}
	}

	if (luaL_newmetatable(_pL, sClass) == 0)
		luaL_error(_pL, "Bad key for register class. '%s' already exists with type '%s'!!!", sClass, lua_typename(_pL, lua_type(_pL, -1)));
	
	lua_pushstring(_pL, "__index");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ClassIndex<T>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__newindex");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ClassNewIndex<T>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__tostring");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ToString<T>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propget");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propset");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	return LuaRegisterClass<T>(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX), sClass, true);
}

template<typename Derived, typename Base>
LuaRegisterClass<Derived> LuaScript::Register(const char * sClass) {
	if (!LuaClassInfo<Base>::Used)
		luaL_error(_pL, "You should register base class before derived['%s']!!!", sClass);

	if (LuaClassInfo<Derived>::Used) {
		if (LuaClassInfo<Derived>::Name.compare(sClass) != 0) {
			luaL_error(_pL, "Class '%s' has already registered with key '%s'!!!", sClass, LuaClassInfo<Derived>::Name.c_str());
		} else {
			return LuaRegisterClass<Derived>(_pL, LuaClassInfo<Derived>::RefIdx, sClass, false);
		}
	}

	if (luaL_newmetatable(_pL, sClass) == 0)
		luaL_error(_pL, "Bad key for register class. '%s' already exists with type '%s'!!!", sClass, lua_typename(_pL, lua_type(_pL, -1)));
	
	lua_pushstring(_pL, "__index");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ClassIndex<Derived>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__newindex");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ClassNewIndex<Derived>);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__tostring");
	lua_pushcfunction(_pL, &LuaMetatableProxy::ToString<Derived>);
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

	return LuaRegisterClass<Derived>(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX), sClass, true);
}
