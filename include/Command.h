#ifndef 	__ENGINE_COMMAND_H_INCLUDED__
#define		__ENGINE_COMMAND_H_INCLUDED__

#include	<map>
#include	<string>

class Command {
public:
	Command() {}
	Command(int nArgc, char * pArgv[]) { Parse(nArgc, pArgv); }
	Command(const std::string & sCmd) { Parse(sCmd); }

	/**
	 * Parse command line parameters.
	 *
	 * \param	nArgc	Number of params pasted from main.
	 * \param	pArgv	Parameter list.
	 **/
	void	Parse(int nArgc, char * pArgv[]);

	/**
	 * Parse white-space splited command list.
	 *
	 * \param	sCmd	White-space splited command like "-std=c++11 -lpthread"
	 **/
	void	Parse(const std::string & sCmd);

	/**
	 * Check if there exists a command named after gaven key.
	 *
	 * \param	sKey	Key for this command. '-std' is a key in '-std=c++11'.
	 * \return	True for exiting.
	 **/
	bool	Has(const std::string & sKey);

	/**
	 * Get value of some parameter.
	 *
	 * \param	sKey	Key for this parameter. '-std' is a key in '-std=c++11'.
	 * \return	Value of this parameter. 'c++11' is value of '-std=c++11'.
	 **/
	std::string	Get(const std::string & sKey);

private:
	std::map<std::string, std::string>	_mCmd;
};

#endif//!	__ENGINE_COMMAND_H_INCLUDED__