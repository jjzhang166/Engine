#ifndef		__ENGINE_RUNNABLE_H_INCLUDED__
#define		__ENGINE_RUNNABLE_H_INCLUDED__

#include	<condition_variable>
#include	<mutex>
#include	<queue>
#include	<thread>

/**
 * A IRunnable object defineds a certain job and holds parameters needed by this
 * job. It does not own thread by self, but picked by some thread to do this job.
 *
 * Useage:
 *
 * class DemoTask : public IRunnable {
 * public:
 *     DemoTask(int n) : _n(n) {}
 *
 * 	   virtual void Run() {
 *	       for (int i = 0; i < n; ++i) {
 *	           std::cout << "Print " << n << std::endl;
 *         }
 *     }
 * };
 *
 * int main() {
 *	   Runnable<DemoTask> iTest(12);   //! Create a container that has 12 workers to do job.
 *     
 *     for (int i = 1; i < 100; ++i) {
 *         iTest.Create(i);
 *     }
 *
 *	   iTest.WaitAll();     //! If we do NOT do this, some job will be dropped directly and never run.
 *     return 0;
 * }
 **/
class IRunnable {
public:
	IRunnable() {}
	virtual ~IRunnable() {}

	/**
	 * Jobs to do
	 **/
	virtual void	Run() = 0;
};

/**
 * Thread pool. Most of time, you would take Runnable<T> instead of ThreadPool.
 **/
class ThreadPool {
public:
	ThreadPool(int nWorkers);
	virtual ~ThreadPool();

	/**
	 * Add a job into queue. Always use 'new' to create this job, because
	 * after finish, 'pJob' will be delete by ThreadPool.
	 *
	 * \param	pJob	Pointer to this job. Should be returned by new operator.
	 * \return	If successfully added.
	 **/
	bool	AddRunnable(IRunnable * pJob);

	/**
	 * Wait for all jobs finished. This will disable AddRunnable()
	 **/
	void	WaitAll();

private:
	void	__WorkerThread();

private:
	bool						_bValid;
	bool						_bCanAdd;
	std::queue<IRunnable *>		_qJobs;
	std::queue<std::thread *>	_qWorkers;
	std::mutex					_iLock;
	std::condition_variable		_iSignal;
};

/**
 * Container for IRunnable. Hold a thread-pool by itself to run jobs.
 **/
template<typename O>
class Runnable : public ThreadPool {
public:
	Runnable(int nWorkers) : ThreadPool(nWorkers) {}

	/**
	 * Create a new IRunnable and push it to queue to run.
	 *
	 * \param	args...	Parameters needed by constructor.
	 * \return	If successfully added.
	 **/
	template<typename ... Args>
	bool	Create(Args && ... args) {
		O * p = new O(args...);
		if (!p) return false;
		AddRunnable(p);
		return true;
	}
};

#endif//!	__ENGINE_RUNNABLE_H_INCLUDED__