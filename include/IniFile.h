#ifndef 	__ENGINE_INIFILE_HPP_INCLUDED__
#define		__ENGINE_INIFILE_HPP_INCLUDED__

#include	"Utils.h"
#include	<map>

class IniFile {
public:
	typedef std::map<std::string, std::string>	Attributes;
	typedef std::map<std::string, Attributes>	Sessions;

public:
	IniFile() {}

	/**
	 * Load an INI file in gaven path.
	 *
	 * \param	sFile	Path to find this file.
	 * \return	Is this file successfully loaded. You can use GetError() to get the detail error.
	 **/
	bool Load(const std::string & sFile);

	/**
	 * If this INI file failed to load, this will return the detail information.
	 *
	 * \return	Detail information for failure.
	 **/
	std::string	GetError();

	/**
	 * Get all sessions' name in this INI file.
	 *
	 * \return	All sessions' name in a vector.
	 **/
	std::vector<std::string> GetSessions();

	/**
	 * Get all keys in a session.
	 **/
	std::vector<std::string> GetKeys(const std::string & sSession);

	/**
	 * Get value of special configuration.
	 *
	 * \param	sSession	Session name of this configuration.
	 * \param	sKey		Key of this configuration.
	 * \param	rDefault	Default value for this configuration.
	 * \return	Value of this configuration. If the configuration does NOT exists, returns the default value.
	 **/
	template<typename T>
	T Get(const std::string & sSession, const std::string & sKey, const T & rDefault) const {
		auto itS = _mContent.find(sSession);
		if (itS == _mContent.end()) return rDefault;

		auto itK = itS->second.find(sKey);
		if (itK == itS->second.end()) return rDefault;

		return Convert<T>(itK->second);
	}

private:
	Sessions	_mContent;
	int			_nLastError;
};

#endif//!	__ENGINE_INIFILE_HPP_INCLUDED__