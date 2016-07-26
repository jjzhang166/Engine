#ifndef		__ENGINE_SINGLETON_HPP_INCLUDED__
#define		__ENGINE_SINGLETON_HPP_INCLUDED__

#include	<memory>

template<class O>
class ISingleton {
public:
	ISingleton() {}

	/**
	 * Get runtime singleton instance of this class.
	 */
	static O &	Instance() {
		if (_iIns.get() == nullptr) _iIns.reset(new O);
		return *(_iIns.get());
	}

	/**
	 * Release runtime singleton instance of this class.
	 **/
	static void	Release() {
		_iIns.reset(nullptr);
	}

private:
	static std::unique_ptr<O>	_iIns;
};

template<class O>
std::unique_ptr<O> ISingleton<O>::_iIns;

#endif//!	__ENGINE_SINGLETON_HPP_INCLUDED__