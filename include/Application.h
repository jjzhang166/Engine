#ifndef		__ENGINE_APPLICATION_H_INCLUDED__
#define		__ENGINE_APPLICATION_H_INCLUDED__

#include	"Command.h"
#include	<atomic>
#include	<functional>

#define		RUN_APP(AppClass)	int main(int nArgc, char * pArgv[]) { \
	AppClass iApp; \
	iApp.Start(nArgc, pArgv); \
	return 0; \
}

class Application {
public:
	Application();
	virtual ~Application() {}

	/**
	 * Register signal handler
	 *
	 * \param	nSig	Signal identifier.
	 * \param	fOpt	Handler callbacks.
	 **/
	void Signal(int nSig, std::function<void (int)> fOpt);

	/**
	 * Start this application. Use RUN_APP(AppClass) instead.
	 *
	 * \param	nArgc	Number of command line params.
	 * \param	pArgv	Params in strings.
	 **/
	virtual void	Start(int nArgc, char * pArgv[]);

	/**
	 * Initialization for this application.
	 *
	 * \param	rCmd	Command line params.
	 * \return	True for initialized successfully, false will termination this application.
	 **/
	virtual bool	OnInit(Command & rCmd) { return true; }

	/**
	 * Tick update in main thread.
	 **/
	virtual void	OnTick() {}

	/**
	 * Action to do before real exit.
	 *
	 * \param	nCode	The way to exit passed by raise(nExitCode), abort(nCode) or exit(nCode);
	 **/
	virtual void	OnExit(int nCode) {}

private:
	std::atomic<bool>							_bRun;
	int											_nExit;
	std::map<int, std::function<void (int)>>	_mSignalHanders;

	friend struct AppSignalDispatcher;
};

#endif//!	__ENGINE_APPLICATION_H_INCLUDED__