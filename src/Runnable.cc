#include	<Runnable.h>

ThreadPool::ThreadPool(int nWorkers) : _bValid(true), _bCanAdd(true) {
	for (int i = 0; i < nWorkers; ++i) {
		try {
			_qWorkers.push(new std::thread(&ThreadPool::__WorkerThread, this));
		} catch (std::runtime_error &) {
			--i;
		}
	}
}

ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> _(_iLock);
		_bValid = false;

		while (!_qJobs.empty()) {
			IRunnable * p = _qJobs.front();
			_qJobs.pop();
			delete p;
		}
	}
	
	_iSignal.notify_all();

	while (!_qWorkers.empty()) {
		std::thread * p = _qWorkers.front();
		_qWorkers.pop();
		if (p->joinable()) p->join();
		delete p;
	}
}

bool ThreadPool::AddRunnable(IRunnable * pJob) {
	{
		std::unique_lock<std::mutex> _(_iLock);
		if (!_bCanAdd) return false;
		_qJobs.push(pJob);
	}
	
	_iSignal.notify_one();
	return true;
}

void ThreadPool::WaitAll() {
	{
		std::unique_lock<std::mutex> _(_iLock);
		_bCanAdd = false;
	}

	while (!_qJobs.empty()) std::this_thread::yield();

	{
		std::unique_lock<std::mutex> _(_iLock);
		_bCanAdd = true;
	}
}

void ThreadPool::__WorkerThread() {
	while (true) {
		std::unique_lock<std::mutex> iAuto(_iLock);
		if (!_bValid) break;
		if (_qJobs.empty()) _iSignal.wait(iAuto);
		if (_qJobs.empty()) continue;

		IRunnable * p = _qJobs.front();
		_qJobs.pop();
		iAuto.unlock();

		p->Run();
		delete p;
	}
}