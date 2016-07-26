#include	<Application.h>
#include	<csignal>
#include	<thread>

#if !defined(_WIN32)
#	include		<sys/time.h>
#	include		<sys/resource.h>
#endif

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

Application::Application() : _bRun(false), _nExit(0) {
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
	{
		Command iCmd(nArgc, pArgv);
		if (!OnInit(iCmd)) return;
	}

	auto iExiter = [this](int nSig) {
		_nExit = nSig;
		_bRun = false;
	};

	Signal(SIGINT, iExiter);
	Signal(SIGTERM, iExiter);

#if !defined(_WIN32)
	signal(SIGPIPE, SIG_IGN);

	struct rlimit iCore;
	getrlimit(RLIMIT_CORE, &iCore);
	iCore.rlim_cur = iCore.rlim_max;
	setrlimit(RLIMIT_CORE, &iCore);
#endif

	_bRun = true;
	while (_bRun) {
		OnTick();
		std::this_thread::yield();
	}

	OnExit(_nExit);
}
