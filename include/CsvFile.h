#ifndef		__ENGINE_CSVFILE_H_INCLUDED__
#define		__ENGINE_CSVFILE_H_INCLUDED__

#include	"Utils.h"

class CsvFile {
public:
	CsvFile() {}

	/**
	 * Load a CSV file.
	 *
	 * \param	sFile		Path to find this file.
	 * \param	nHeader		Which line is header.
	 * \param	nDelimiter	Data delimiter. Tab-key as default.
	 * \return	True for loaded successfully.
	 **/
	bool Load(const std::string & sFile, int nHeader = 0, char nDelimiter = '\t');

	/**
	 * Get number of valid records in this file.
	 **/
	size_t Count() { return _vData.size(); }

	/**
	 * Get all headers
	 **/
	std::vector<std::string> Headers() { return _vHeader; }

	/**
	 * Get data in CSV file.
	 *
	 * \param	nIdx		Record index.
	 * \param	sKey		Key of value in this record
	 * \param	tDefault	Default value if undefined.
	 * \return	Value of this data.
	 **/
	template<typename T>
	T Get(int nIdx, const std::string & sKey, const T & tDefault) const {
		if (nIdx < 0 || (size_t)nIdx >= _vData.size()) return tDefault;
		for (size_t nCol = 0; nCol < _vHeader.size(); ++nCol) {
			if (_vHeader[nCol].compare(sKey) == 0) {
				std::string sData = _vData[nIdx][nCol];
				if (sData.empty()) return tDefault;
				return Convert<T>(sData);
			}
		}
		return tDefault;
	}

private:
	std::vector<std::string>				_vHeader;
	std::vector<std::vector<std::string>>	_vData;
};

#endif//!	__ENGINE_CSVFILE_H_INCLUDED__