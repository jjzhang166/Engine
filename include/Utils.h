#ifndef		__ENGINE_UTILS_H_INCLUDED__
#define		__ENGINE_UTILS_H_INCLUDED__

#include	<cstdint>
#include	<string>
#include	<vector>

/**
 * Convert a string to special type.
 **/
template<typename T>
T Convert(const std::string & s);

/**
 * Convert special value to string.
 **/
std::string ToString(bool b);
std::string ToString(int8_t n);
std::string ToString(int16_t n);
std::string ToString(int32_t n);
std::string ToString(int64_t n);
std::string ToString(uint8_t n);
std::string ToString(uint16_t n);
std::string ToString(uint32_t n);
std::string ToString(uint64_t n);
std::string ToString(const std::string & s);
std::string ToString(const char * p);

/**
 * Get lower case of gaven string.
 *
 * \param	s	Source string to convert.
 * \return	String all in lower case.
 **/
std::string	ToLower(const std::string & s);

/**
 * Get upper case of gaven string.
 *
 * \param	s	Source string to convert.
 * \return	String all in upper case.
 **/
std::string ToUpper(const std::string & s);

/**
 * Replace. Regex expression NOT supported.
 **/
std::string Replace(const std::string & s, const std::string & sPattern, const std::string & sReplace);

/**
 * Trim string.
 */
std::string	TrimStart(const std::string & s, const std::string & sTrim);
std::string	TrimEnd(const std::string & s, const std::string & sTrim);
std::string	Trim(const std::string & s, const std::string & sTrim);

/**
 * Split string with gaven delimiters into vector<string>.
 *
 * \param	s				Source string to split.
 * \param	sDelim			Delimiters.
 * \param	bIgnoreEmpty	Should we ignore the empty string.
 * \return	String list in vector.
 **/
std::vector<std::string> Split(const std::string & s, const std::string & sDelim, bool bIgnoreEmpty = true);

/**
 * Generate an UUID string.
 **/
std::string CreateID();

#endif//!	__ENGINE_UTILS_H_INCLUDED__
