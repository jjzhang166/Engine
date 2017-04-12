#include	<Script.h>
#include	<Path.h>
#include	<Logger.h>
#include	<Utils.h>

#include	<algorithm>
#include	<memory>

LuaNil GLuaNil;

static int __OverridePrint(lua_State * L) {
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
}

static int __PrintErr(lua_State * L) {
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

	Logger::Instance().Log(ELog::Error, d.source, d.currentline, "%s", sOutput.c_str());
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Implements of LuaMetatableProxy
///////////////////////////////////////////////////////////////////////////////
int LuaMetatableProxy::Readonly(lua_State * pL) {
	return luaL_error(pL, "Property '%s' is read-only!!!", lua_tostring(pL, lua_upvalueindex(1)));
}

int LuaMetatableProxy::Extends(lua_State * pL) {
	const char * sName = lua_tostring(pL, -1);

	lua_getglobal(pL, sName);
	if (lua_istable(pL, -1)) {
		lua_getmetatable(pL, -1);
		lua_remove(pL, -2);

		if (lua_isnil(pL, -1)) {
			lua_pop(pL, 1);
			luaL_error(pL, "'%s' is not a registered class or namespace!!!", sName);
		}
	} else if (lua_isnil(pL, -1)) {
		lua_pop(pL, 1);
		luaL_getmetatable(pL, sName);

		if (lua_isnil(pL, -1)) {
			lua_pop(pL, 1);
			luaL_error(pL, "'%s' is not a registered class or namespace!!!", sName);
		}
	} else {
		lua_pop(pL, 1);
		luaL_error(pL, "Class or namespace '%s' not registered!!!", sName);
	}

	return 1;
}

int LuaMetatableProxy::Index(lua_State * pL) {
	lua_getmetatable(pL, 1);	// tb, key, meta
	lua_pushvalue(pL, 2);		// tb, key, meta, key
	lua_rawget(pL, -2);			// tb, key, meta, meta[key]

	if (lua_isnil(pL, -1)) {	
		lua_pop(pL, 1);						// tb, key, meta
		lua_pushstring(pL, "__propget");	// tb, key, meta, '__propget'
		lua_rawget(pL, -2);					// tb, key, meta, meta.__propget
		lua_pushvalue(pL, 2);				// tb, key, meta, meta.__propget, key
		lua_rawget(pL, -2);					// tb, key, meta, meta.__propget, meta.__propget.key
		lua_remove(pL, -2);					// tb, key, meta, meta.__propget.key
		if (lua_iscfunction(pL, -1)) {
			lua_remove(pL, -2);				// tb, key, meta.__propget.key
			lua_pushvalue(pL, 1);			// tb, key, meta.__propget.key, tb
			lua_call(pL, 1, 1);
			return 1;
		} else {
			lua_pop(pL, 2);
			return 1;
		}
	} else if (lua_istable(pL, -1) || lua_iscfunction(pL, -1)) {
		lua_remove(pL, -2);	// tb, key, meta[key]
		return 1;
	} else {
		lua_pop(pL, 2);		// tb, key
		return 0;
	}
}

int LuaMetatableProxy::NewIndex(lua_State * pL) {
	lua_getmetatable(pL, 1);				// tb, key, value, meta
	lua_pushstring(pL, "__propset");		// tb, key, value, meta, '__propset'
	lua_rawget(pL, -2);						// tb, key, value, meta, meta.__propset
	lua_remove(pL, -2);						// tb, key, value, meta.__propset,

	if (!lua_istable(pL, -1)) {
		lua_pop(pL, 1);
		return 0;
	}

	lua_pushvalue(pL, 2);	//! tb, key, value, meta.__propset, key
	lua_rawget(pL, -2);		//! tb, key, value, meta.__propset, meta.__propset.key
	lua_remove(pL, -2);		//! tb, key, value, meta.__propset.key

	if (lua_iscfunction(pL, -1)) {
		lua_pushvalue(pL, 3);	//! tb, key, value, meta.__propset.key, value
		lua_call(pL, 1, 0);
	}

	return 0;
}

int LuaMetatableProxy::Func(lua_State * pL) {
	if (!lua_islightuserdata(pL, lua_upvalueindex(1))) luaL_error(pL, "Call function error!!!");

	typedef int (*CFunc)(LuaState &);
	CFunc f = (CFunc)(lua_touserdata(pL, lua_upvalueindex(1)));
	if (!f) luaL_error(pL, "Try to call an undefined method!!!");

	LuaState i(pL, 0);
	return f(i);
}

///////////////////////////////////////////////////////////////////////////////
/// Implements of LuaState
///////////////////////////////////////////////////////////////////////////////
LuaState::~LuaState() {
	if (_nCleanTop > 0) lua_pop(_pL, _nCleanTop);
}

int LuaState::Top() {
	return lua_gettop(_pL);
}

void LuaState::Pop(int n) {
	lua_pop(_pL, n);
}

int LuaState::Type(int nIdx) {
	return lua_type(_pL, nIdx);
}

///////////////////////////////////////////////////////////////////////////////
/// Implements of LuaTable.
///////////////////////////////////////////////////////////////////////////////
LuaTable::LuaTable(const LuaTable & r) : _pL(r._pL), _nRef(LUA_NOREF) {
	lua_rawgeti(_pL, LUA_REGISTRYINDEX, r._nRef);
	_nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);
}

LuaTable::LuaTable(LuaState & r) : _pL(r._pL), _nRef(LUA_NOREF) {
	lua_newtable(_pL);
	_nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);
}

LuaTable & LuaTable::operator=(const LuaTable & r) {
	Unref();

	if (r._pL && r._nRef != LUA_NOREF) {
		lua_rawgeti(r._pL, LUA_REGISTRYINDEX, r._nRef);
		_pL = r._pL;
		_nRef = luaL_ref(_pL, LUA_REGISTRYINDEX);
	}

	return *this;
}

void LuaTable::Push() {
	if (_pL && _nRef != LUA_NOREF)
		lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	else
		lua_pushnil(_pL);
}

void LuaTable::Unref() {
	if (_pL && _nRef != LUA_NOREF) {
		luaL_unref(_pL, LUA_REGISTRYINDEX, _nRef);
		_pL = 0;
		_nRef = LUA_NOREF;
	}
}

int LuaTable::Size() {
	if (_pL && _nRef != LUA_NOREF) {
		lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
		int n = (int)luaL_len(_pL, -1);
		lua_pop(_pL, 1);
		return n;
	} else {
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Implements of LuaRegister.
///////////////////////////////////////////////////////////////////////////////
LuaRegister & LuaRegister::Method(const char * sMethod, int (*fOpt)(LuaState &)) {
	int nTop = lua_gettop(_pL);

	lua_rawgeti(_pL, LUA_REGISTRYINDEX, _nRef);
	lua_pushstring(_pL, sMethod);
	lua_pushlightuserdata(_pL, (void *)fOpt);
	lua_pushcclosure(_pL, &LuaMetatableProxy::Func, 1);
	lua_rawset(_pL, -3);
	lua_settop(_pL, nTop);

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/// Implements of LuaScript.
///////////////////////////////////////////////////////////////////////////////
LuaScript::LuaScript() : _pL(luaL_newstate()) {
	lua_atpanic(_pL, [](lua_State * pL) -> int {
		if (lua_gettop(pL) > 0) {
			Logger::Instance().Log(ELog::Error, "@SCRIPT", -1, lua_tostring(pL, -1));
			lua_pop(pL, 1);
		} else {
			Logger::Instance().Log(ELog::Error, "@SCRIPT", -1, "Unknown error!");
		}

		lua_gc(pL, LUA_GCCOLLECT, 0);
		throw std::runtime_error("Script error ! Just for long jump!");
		return 0;
	});

	luaL_openlibs(_pL);

	lua_pushcfunction(_pL, &LuaMetatableProxy::Extends);
	lua_setglobal(_pL, "Extends");

	lua_pushcfunction(_pL, &__OverridePrint);
	lua_setglobal(_pL, "print");

	lua_pushcfunction(_pL, &__PrintErr);
	lua_setglobal(_pL, "print_err");
}

LuaScript::~LuaScript() {
	if (_pL) lua_close(_pL);
}

LuaScript & LuaScript::Instance() {
	static std::unique_ptr<LuaScript> _iIns;
	if (!_iIns.get()) _iIns.reset(new LuaScript);
	return *(_iIns.get());
}

void LuaScript::Require(const char * sFile) {
	if (strlen(sFile) <= 4) return;

	std::string sModule(sFile);
	std::string sExt = ToUpper(sModule.substr(sModule.size() - 4));

	if (sExt == ".LUA") sModule = sModule.substr(0, sModule.size() - 4);
	sModule = Replace(sModule, "\\", ".");
	sModule = Replace(sModule, "/", ".");
	sModule = Replace(sModule, "..", ".");

	std::string sCode = "require '" + sModule + "';";
	Run(sCode.c_str());
}

void LuaScript::DoFile(const char * sFile) {
	int nTop = lua_gettop(_pL);
	std::string sPath = Path::FullPath(sFile);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	try {
		if (0 != luaL_loadfile(_pL, sFile) || 0 != lua_pcall(_pL, 0, LUA_MULTRET, nTop + 1)) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}

		lua_remove(_pL, nTop + 1);
	} catch (...) {}

	lua_settop(_pL, nTop);
}

void LuaScript::Run(const char * sCode) {
	int nTop = lua_gettop(_pL);

	lua_getglobal(_pL, "debug");
	lua_getfield(_pL, -1, "traceback");
	lua_remove(_pL, -2);

	try {
		if (0 != luaL_loadstring(_pL, sCode) || 0 != lua_pcall(_pL, 0, 0, nTop + 1)) {
			lua_remove(_pL, nTop + 1);
			lua_error(_pL);
		}
	} catch (...) {}

	lua_settop(_pL, nTop);
}

LuaState LuaScript::Stack() {
	return LuaState(_pL, 0);
}

void LuaScript::GC() {
	lua_gc(_pL, LUA_GCCOLLECT, 0);
}

LuaRegister LuaScript::Register(const char * sNamespace) {
	lua_getglobal(_pL, sNamespace);
	if (!lua_isnil(_pL, -1)) return LuaRegister(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX));

	lua_pop(_pL, 1);
	lua_newtable(_pL);
	lua_newtable(_pL);

	lua_pushstring(_pL, "__index");
	lua_pushcfunction(_pL, &LuaMetatableProxy::Index);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__newindex");
	lua_pushcfunction(_pL, &LuaMetatableProxy::NewIndex);
	lua_rawset(_pL, -3);

	lua_pushstring(_pL, "__propget");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);
	
	lua_pushstring(_pL, "__propset");
	lua_newtable(_pL);
	lua_rawset(_pL, -3);

	lua_setmetatable(_pL, -2);
	lua_setglobal(_pL, sNamespace);
	lua_getglobal(_pL, sNamespace);
	return LuaRegister(_pL, luaL_ref(_pL, LUA_REGISTRYINDEX));
}
