#include	<Command.h>
#include	<Utils.h>

void Command::Parse(int nArgc, char * pArgv[]) {
	_mCmd.clear();

	for (int nIdx = 0; nIdx < nArgc; ++nIdx) {
		if (pArgv[nIdx] && pArgv[nIdx][0]) {
			std::string sParam(pArgv[nIdx]);
			size_t nPos = sParam.find_first_of("=");
			size_t nLen = sParam.length();
			if (nPos == std::string::npos)
				_mCmd[sParam] = "";
			else if (nPos == 0)
				continue;
			else if (nPos == nLen - 1)
				_mCmd[sParam.substr(0, nLen - 1)] = "";
			else
				_mCmd[sParam.substr(0, nPos)] = sParam.substr(nPos + 1, nLen - 1 - nPos);
		}
	}
}

void Command::Parse(const std::string & sCmd) {
	_mCmd.clear();

	std::vector<std::string> vParams = Split(sCmd, " \t");
	for (size_t i = 0; i < vParams.size(); ++i) {
		std::string sParam(vParams[i]);
		size_t nPos = sParam.find_first_of("=");
		size_t nLen = sParam.length();
		if (nPos == std::string::npos)
			_mCmd[sParam] = "";
		else if (nPos == 0)
			continue;
		else if (nPos == nLen - 1)
			_mCmd[sParam.substr(0, nLen - 1)] = "";
		else
			_mCmd[sParam.substr(0, nPos)] = sParam.substr(nPos + 1, nLen - 1 - nPos);
	}
}

bool Command::Has(const std::string & sKey) {
	return _mCmd.find(sKey) != _mCmd.end();
}

std::string Command::Get(const std::string & sKey) {
	auto it = _mCmd.find(sKey);
	if (it == _mCmd.end()) return "";
	return it->second;
}
