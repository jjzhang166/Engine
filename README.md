#Engine
C++服务器编程底层库

## 特点
1. Windows，Linux双平台（Windows下为静态库，主要方便开发者调试；Linux下为动态库，用于生产环境部署）
2. 基本包含集成服务器常用模块（数学、文件系统、配置、日志、网络、脚本、时间、多线程、Redis等）
3. 二次开发无平台配置，无其他依赖
4. 基于C++11开发

## 使用

1. Windows下直接使用VS2015打开工程编译生成：`build/Engine.lib`与`build/Engined.lib`静态库
2. Linux下使用make生成`libengine.so`动态库

> 注：Linux下建议使用GCC的`-Wl,-rpath,XXX`连接选项指定运行期动态连接库的优先查找目录，以方便分发部署

## 说明
1. Network模型：Windows使用select，Linux下使用epoll.
2. Zip使用[miniz](https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/miniz/miniz_v115_r4.7z) v1.15 r4.
3. [Lua](http://www.lua.org/ftp/lua-5.3.3.tar.gz) v5.3.3.
4. Hiredis基于官方[hiredis](https://github.com/redis/hiredis) v0.13.3修改使之支持Windows平台。Redis类为Hiredis + libev.
5. [Libev](http://dist.schmorp.de/libev/libev-4.22.tar.gz) v4.22.

## 模块

### 程序模型

```cpp
#include	<Application.h>

class GameApp : public Application {
public:
	GameApp() {}

	virtual bool	OnInit(Command & rCmd) override {
		if (rCmd.Has('--debug')) {
			Logger::Instance().Init("game", "logs", 1024 * 1024 * 4, ELog::Debug);
		} else {
			Logger::Instance().Init("game", "logs", 1024 * 1024 * 4, ELog::Info);
		}

		/// 填写其他初始化逻辑，当返回false时程序直接退出
		return true;
	}

	virtual void	OnTick() override {
		/// 这里填写需要每帧更新的逻辑
	}
};

RUN_APP(GameApp)
```

### 网络模型

> 头文件 : Network.h
客户端继承ISocket，服务器继承IServerSocket
需要在主线程中调用Breath()来触发一次累计接收信息的处理

以客户端为例：
```cpp

#include	<Network.h>
#include	<Singleton.h>

class ServerConnect : public ISocket, public ISingleton<ServerConnect> {
public:
	ServerConnect() {}

	...
}

/// 在主线程的初始化
bool GameApp::OnInit(Command & rCmd) {
	...

	ServerConnect::Instance().Connect(...);

	...
}

/// 在主线程Tick中调用
void GameApp::OnTick() {
	ServerConnect::Instance().Breath();
}

```

### 脚本

> 1. 设计原则：Lua只负责逻辑，对象生存管理交由C++（可以注册管理到Lua）
2. 涉及到Get操作，需要try...catch以捕获类型异常（C++注册到Lua的接口内Get不需要，调用函数不需要）
3. Property可以为地址方式，也可以为Getter(T (void))、Setter(void (reference type T))方式注册
4. Method必须为`int (*f)(LuaState &)`

```cpp
#include	<Script.h>

/// 注册公共变量或函数到Lua
GLua.Register("GameSetting")	//! 所有下面注册的属性或函数放在GameSetting中
	.Property("nPlayerCounter", &GPlayerCount, false)	//! 以地址方式注册属性，同时设置不可写
	.Property("nTime", &GetTime)	//! 以Getter方式注册属性，同时不注册属性的写方法（不可写）
	.Method("GetAById", &GetAById);	//! 注册全局Lua方法

/// 注册C++类到Lua
GLua.Register<A>("LuaA")
	.Property("nId", &A::nId, false)
	.Property("sName", &A::GetName, &A::SetName)
	.Method("Msg", &A::SendMessage);

try {
	A * p = GLua.Get<A *>("me");	// 需要使用try，因为可能类型不匹配
} catch (...) {}

if (GLua.Is<A *>("me")) {} // 不需要try
GLua.Set<A *>("me", new A) // 不需要try
GLua.Call("GameSetting", "GetAById", 100); // 不需要try

int GetAById(LuaState & r) {
	int n = r.Get<int>(1);	// 不需要try
	...
}
```