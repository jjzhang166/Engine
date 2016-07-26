#ifndef		__ENGINE_PATH_H_INCLUDED__
#define		__ENGINE_PATH_H_INCLUDED__

#include	<functional>
#include	<string>

struct Path {

	/**
	 * Check does gaven path exists.
	 *
	 * \param	sPath	Path to check.
	 * \return	True for existing.
	 **/
	static bool Exists(const std::string & sPath);

	/**
	 * Create a path. Parent directory MUST exists!!!
	 *
	 * \param	sPath	Path to create.
	 * \return	True for success.
	 **/
	static bool Create(const std::string & sPath);

	/**
	 * Get current working directory.
	 **/
	static std::string Current();

	/**
	 * Change current working directory.
	 *
	 * \param	sPath	Path to change to.
	 * \return	True for success.
	 **/
	static bool	Change(const std::string & sPath);

	/**
	 * Get full path.
	 *
	 * \param	sPath	Path to convert.
	 * \return	Full path
	 **/
	static std::string FullPath(const std::string & sPath);

	/**
	 * Get directory part of a path.
	 *
	 * \param	sPath	Orignal path.
	 * \return	Directory part of this path.
	 **/
	static std::string PurePath(const std::string & sPath);

	/**
	 * Get file name of a path.
	 *
	 * \param	sPath	Orignal path.
	 * \return	file name of this path.
	 **/
	static std::string PureFile(const std::string & sPath);

	/**
	 * Traverse all files under gaven path.
	 *
	 * \param	sPath		Path to traverse.
	 * \param	fOpt		Action to do with this file.
	 * \param	bRecursive	Should we go into the sub directories.
	 **/
	static void Traverse(const std::string & sPath, std::function<void (const std::string &)> fOpt, bool bRecursive = true);
};

#endif//!	__ENGINE_PATH_H_INCLUDED__