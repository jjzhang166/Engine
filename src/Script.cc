#include	<Script.h>
#include	<Logger.h>
#include	<Utils.h>

int LuaProxy::Readonly(lua_State * L) {
	return luaL_error(L, "Property '%s' is read-only!!!", lua_tostring(L, lua_upvalueindex(1)));
}

int LuaProxy::Index(lua_State * L) {
	lua_getmetatable(L, 1);	// tb, key, meta
	lua_pushvalue(L, 2);	// tb, key, meta, key
	lua_rawget(L, -2);		// tb, key, meta, meta[key]

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);					// tb, key, meta
		lua_pushstring(L, "__propget");	// tb, key, meta, '__propget'
		lua_rawget(L, -2);				// tb, key, meta, meta.__propget
		lua_pushvalue(L, 2);			// tb, key, meta, meta.__propget, key
		lua_rawget(L, -2);				// tb, key, meta, meta.__propget, meta.__propget[key]
		lua_remove(L, -2);				// tb, key, meta, meta.__propget[key]

		if (lua_iscfunction(L, -1)) {
			lua_remove(L, -2);
			lua_pushvalue(L, 2);
			lua_call(L, 1, 1);
			return 1;
		} else {
			lua_pop(L, 2);
			lua_pushnil(L);
			return 0;
		}
	} else if (lua_istable(L, -1) || lua_iscfunction(L, -1) || lua_isfunction(L, -1)) {
		lua_remove(L, -2);
		return 1;
	} else {
		lua_pop(L, 2);
		return 0;
	}
}

int LuaProxy::NewIndex(lua_State * L) {
	lua_getmetatable(L, 1);				// tb, key, value, meta
	lua_pushstring(L, "__propset");		// tb, key, value, meta, '__propset'
	lua_rawget(L, -2);					// tb, key, value, meta, meta.__propset
	lua_remove(L, -2);					// tb, key, value, meta.__propset,

	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	lua_pushvalue(L, 2);	//! tb, key, value, meta.__propset, key
	lua_rawget(L, -2);		//! tb, key, value, meta.__propset, meta.__propset.key
	lua_remove(L, -2);		//! tb, key, value, meta.__propset.key

	if (lua_iscfunction(L, -1)) {
		lua_pushvalue(L, 3);	//! tb, key, value, meta.__propset.key, value
		lua_call(L, 1, 0);
	}

	return 0;
}

int LuaProxy::Method(lua_State * L) {
	if (!lua_islightuserdata(L, lua_upvalueindex(1))) luaL_error(L, "Call method error!!!");

	typedef int(*MethodFunc)(LuaStack &);
	MethodFunc f = (MethodFunc)(lua_touserdata(L, lua_upvalueindex(1)));
	if (!f) luaL_error(L, "Try to call an undefined method!!!");

	LuaStack _(L, 0);
	return f(_);
}

LuaTable::LuaTable() : _pL(GLua.State()), _nRef(LUA_NOREF) {
	lua_newtable(_pL);
	_nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);
}

LuaTable & LuaTable::operator=(const LuaTable & r) {
	__Unref();

	if (r._pL && r._nRef != LUA_NOREF) {
		lua_rawgeti(r._pL, LUA_REGISTRYINDEX, r._nRef);
		_pL = r._pL;
		_nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);
	}

	return *this;
}

int LuaTable::Size() const {
	if (!IsValid()) return 0;
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	int n = (int)luaL_len(_pL, -1);
	lua_pop(_pL, 1);
	return n;
}

void LuaTable::__Unref() {
	if (_pL && _nRef != LUA_NOREF) luaL_unref(_pL, LUA_REGISTRYINDEX, _nRef);
	_pL = 0;
	_nRef = LUA_NOREF;
}

LuaNamespace & LuaNamespace::Method(const std::string & sMethod, int (*fOpt)(LuaStack &)) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_pushlstring(_pL, sMethod.c_str(), sMethod.size());
	lua_pushlightuserdata(_pL, (void*)fOpt);
	lua_pushcclosure(_pL, &LuaProxy::Method, 1);
	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

LuaVM::LuaVM() : _pL(luaL_newstate()) {
	luaL_openlibs(_pL);

	/// Build in lua-CJson module.
	extern int luaopen_cjson(lua_State *);
	luaopen_cjson(_pL);

	/// Hook error using Logger.
	lua_atpanic(_pL, [](lua_State * L) -> int {
		if (lua_gettop(L) > 0) {
			Logger::Instance().Log(ELog::Error, "@SCRIPT", -1, lua_tostring(L, -1));
			lua_pop(L, 1);
		} else {
			Logger::Instance().Log(ELog::Error, "@SCRIPT", -1, "Unknown error in script!!");
		}

		lua_gc(L, LUA_GCCOLLECT, 0);
		throw std::runtime_error("Script error! Just for long jump");
		return 0;
	});

	/// add _G.extends method for extends C++ namespace or class.
	lua_pushcfunction(_pL, [](lua_State * L) -> int {
		const char * sName = lua_tostring(L, -1);

		lua_getglobal(L, sName);
		if (lua_istable(L, -1)) {
			lua_getmetatable(L, -1);
			lua_remove(L, -2);
			if (lua_istable(L, -1)) return 1;
		} else if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			luaL_getmetatable(L, sName);
			if (lua_istable(L, -1)) return 1;
		}

		lua_pop(L, 1);
		luaL_error(L, "Class or namespace '%s' has NOT registered!!!", sName);
		return 0;
	});
	lua_setglobal(_pL, "extends");

	/// Override print using Logger interface.
	lua_pushcfunction(_pL, [](lua_State * L) -> int {
		std::string sOutput;
		sOutput.reserve(2048);

		int n = lua_gettop(L);  /* number of arguments */
		int i;

		lua_Debug d;
		memset(&d, 0, sizeof(d));

		lua_getstack(L, 1, &d);
		lua_getinfo(L, "Sln", &d);
		lua_getglobal(L, "tostring");

		for (i = 1; i <= n; i++) {
			const char *s;
			size_t l;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tolstring(L, -1, &l);  /* get result */
			if (s == NULL)
				return luaL_error(L, "'tostring' must return a string to 'print'");

			if (i > 1) sOutput.push_back('\t');
			sOutput.append(s, l);
			lua_pop(L, 1);  /* pop result */
		}

		Logger::Instance().Log(ELog::Info, d.source, d.currentline, "%s", sOutput.c_str());
		return 0;
	});
	lua_setglobal(_pL, "print");

	/// add _G.print_err for print error in Lua.
	lua_pushcfunction(_pL, [](lua_State * L) -> int {
		std::string sOutput;
		sOutput.reserve(2048);

		int n = lua_gettop(L);  /* number of arguments */
		int i;

		lua_Debug d;
		memset(&d, 0, sizeof(d));

		lua_getstack(L, 1, &d);
		lua_getinfo(L, "Sln", &d);
		lua_getglobal(L, "tostring");

		for (i = 1; i <= n; i++) {
			const char *s;
			size_t l;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tolstring(L, -1, &l);  /* get result */
			if (s == NULL)
				return luaL_error(L, "'tostring' must return a string to 'print'");

			if (i > 1) sOutput.push_back('\t');
			sOutput.append(s, l);
			lua_pop(L, 1);  /* pop result */
		}

		Logger::Instance().Log(ELog::Info, d.source, d.currentline, "%s", sOutput.c_str());
		return 0;
	});
	lua_setglobal(_pL, "print_err");
}

LuaVM & LuaVM::Instance() {
	static LuaVM * GIns = nullptr;
	if (GIns == nullptr) GIns = new LuaVM();
	return *GIns;
}

void LuaVM::DoFile(const std::string & sFile, bool bRequre /* = false */) {
	if (sFile.length() <= 4) return;

	if (bRequre) {
		std::string sTemp(sFile);
		std::string sExt = ToUpper(sTemp.substr(sTemp.size() - 4));

		if (sExt == ".LUA") sTemp = sTemp.substr(0, sTemp.size() - 4);
		
		std::string sModule = "";
		size_t nLen = 0;

		for (size_t i = 0; i < sTemp.size(); ++i) {
			char c = sTemp[i];

			if (c == '\\' || c == '/' || c == '.') {
				if (nLen > 0 && sModule[nLen - 1] == '.') continue;
				sModule.push_back('.');
				nLen++;
			} else {
				sModule.push_back(c);
				nLen++;
			}
		}

		std::string sCode = "require '" + sModule + "'";
		Run(sCode);
	} else {
		int nTop = lua_gettop(_pL);
		
		lua_getglobal(_pL, "debug");
		lua_getfield(_pL, -1, "traceback");
		lua_remove(_pL, -2);

		try {
			if (0 != luaL_loadfile(_pL, sFile.c_str()) || 0 != lua_pcall(_pL, 0, LUA_MULTRET, nTop + 1)) {
				lua_remove(_pL, nTop + 1);
				lua_error(_pL);
			}
		} catch (...) {}

		lua_settop(_pL, nTop);
	}
}

void LuaVM::Run(const std::string & sCode) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	try {
		if (0 != luaL_loadstring(_pL, sCode.c_str()) || 0 != lua_pcall(_pL, 0, 0, nTop + 1)) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}
	} catch (...) {}

	lua_settop(_pL, nTop);
}

LuaNamespace LuaVM::Register(const std::string & sNamespace) {
	lua_getglobal(_pL, sNamespace.c_str());
	if (!lua_isnil(_pL, -1)) return LuaNamespace(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX));

	lua_pop(_pL, 1);
	lua_newtable(_pL);
	lua_newtable(_pL);

	lua_pushcfunction(_pL, &LuaProxy::Index);
	lua_setfield(_pL, -2, "__index");

	lua_pushcfunction(_pL, &LuaProxy::NewIndex);
	lua_setfield(_pL, -2, "__newindex");

	lua_newtable(_pL);
	lua_setfield(_pL, -2, "__propget");

	lua_newtable(_pL);
	lua_setfield(_pL, -2, "__propset");

	lua_setmetatable(_pL, -2);
	lua_setglobal(_pL, sNamespace.c_str());
	lua_getglobal(_pL, sNamespace.c_str());
	return LuaNamespace(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX));
}
