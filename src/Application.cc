#include	<Application.h>
#include	<Path.h>
#include	<DateTime.h>

#include	<csignal>
#include	<thread>

#if !defined(_WIN32)
#	include		<sys/time.h>
#	include		<sys/resource.h>
#	include		<unistd.h>
#endif

extern void AutoNetworkBreath();

struct AppSignalDispatcher {
	static Application * pIns;
	static void	OnSignal(int nSig);
};

Application * AppSignalDispatcher::pIns = nullptr;
void AppSignalDispatcher::OnSignal(int nSig) {
	if (!pIns) return;

	auto it = pIns->_mSignalHanders.find(nSig);
	if (it == pIns->_mSignalHanders.end()) return;

	it->second(nSig);
}

Application::Application() : _bRun(false), _nExit(0), _nPerFrame(0) {
	if (!AppSignalDispatcher::pIns)
		AppSignalDispatcher::pIns = this;
	else
		throw std::runtime_error("There is more than one application instance!");	
}

void Application::Signal(int nSig, std::function<void (int)> fOpt) {
	if (_mSignalHanders.find(nSig) == _mSignalHanders.end())
		signal(nSig, &AppSignalDispatcher::OnSignal);

	_mSignalHanders[nSig] = fOpt;
}

void Application::Start(int nArgc, char * pArgv[]) {
	if (nArgc > 0) Path::Change(Path::PurePath(pArgv[0]));
	bool bDaemon = false;

	{
		Command iCmd(nArgc, pArgv);
		bDaemon = iCmd.Has("--daemon");
		if (!OnInit(iCmd)) return;
	}

#if defined(_WIN32)
	(void)bDaemon;
#else
	signal(SIGPIPE, SIG_IGN);

	struct rlimit iCore;
	getrlimit(RLIMIT_CORE, &iCore);
	iCore.rlim_cur = iCore.rlim_max;
	setrlimit(RLIMIT_CORE, &iCore);

	if (bDaemon) (void)daemon(0, 0);
#endif

	auto iExiter = [this](int nSig) {
		_nExit = nSig;
		_bRun = false;
	};

	Signal(SIGINT, iExiter);
	Signal(SIGTERM, iExiter);

	_bRun = true;

	if (_nPerFrame > 0) {
		uint64_t nStart, nLeft;
		while (_bRun) {
			nStart = Tick();
			AutoNetworkBreath();
			OnBreath();
			nLeft = _nPerFrame - Tick() + nStart;
			if (nLeft > 0) std::this_thread::sleep_for(std::chrono::milliseconds(nLeft));
			else std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	} else {
		while (_bRun) {
			AutoNetworkBreath();
			OnBreath();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	
	OnExit(_nExit);
}

void Application::LockFPS(int nFPS) {
	_nPerFrame = (uint64_t)(1000.0f / nFPS);
}
