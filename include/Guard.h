#ifndef 	__ENGINE_GUARD_HPP_INCLUDED__
#define		__ENGINE_GUARD_HPP_INCLUDED__

#include	<functional>

class Guard {
public:
	Guard(std::function<void()> fOpt) : _fOpt(fOpt), _bDismiss(false) {}
	virtual ~Guard() { if (!_bDismiss && _fOpt) _fOpt(); }

	/**
	 * Dismiss a distruction callback.
	 **/
	void	Dismiss() { _bDismiss = true; }

private:
	std::function<void()>	_fOpt;
	bool					_bDismiss;
};

#endif//!	__ENGINE_GUARD_HPP_INCLUDED__