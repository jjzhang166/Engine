#include	<IniFile.h>
#include	<Guard.h>
#include	<cstdio>
#include	<cstring>

namespace {
	enum State {
		Success = 0,
		Param,
		FileAccess,
		LostSession,
		LostKey,
		SessionSyntax,
		KeySyntax,
		ValueSyntax,
		BraceMismatch,
		QuoteMismatch,
		EqualMismatch,
	};

	static char * __INISKIP(char * p) {
		if (!p || !(*p)) return p;
		
		char * r = p;
		char c = *r;

		while (c && (c == ' ' || c == '\r' || c == '\n' || c == '\t')) {
			++r;
			c = *r;
		}

		return r;
	}

	static int __INISESSION(char * pSrc, std::string & sStore) {
		char * p = pSrc;
		bool bUseQuote = false;

		while (*p) {
			char nCh = *p;

			if (bUseQuote) {
				if (nCh == '"') {
					if (sStore.empty()) return State::SessionSyntax;

					p = __INISKIP(p + 1);
					if (!*p) return State::BraceMismatch;
					if (*p != ']') return State::SessionSyntax;

					p = __INISKIP(p + 1);
					if (*p && *p != '#' && *p != ';') return State::SessionSyntax;

					return 0;
				} else {
					sStore.push_back(nCh);
				}
			} else if (nCh == '"') {
				if (!sStore.empty()) return State::SessionSyntax;
				bUseQuote = true;
			} else if (nCh == '#' || nCh == ';' || nCh == '[' || nCh == '=') {
				return State::SessionSyntax;
			} else if (nCh == ' ' || nCh == '\t') {
				p = __INISKIP(p + 1);
				if (*p && *p != ']') return State::SessionSyntax;
				continue;
			} else if (nCh == ']') {
				if (sStore.empty()) return State::LostSession;

				p = __INISKIP(p + 1);
				if (*p && *p != '#' && *p != ';') return State::SessionSyntax;
				return 0;
			} else {
				sStore.push_back(nCh);
			}

			++p;
		}

		return State::SessionSyntax;
	}

	static int __INIATTRIBUTE(char ** pSrc, std::string & sStore) {
		char * p = *pSrc;
		bool bUseQuote = false;

		while (*p) {
			char nCh = *p;

			if (bUseQuote) {
				if (nCh == '"') {
					if (sStore.empty()) return State::KeySyntax;

					p = __INISKIP(p + 1);
					if (!*p) return State::EqualMismatch;
					if (*p != '=') return State::KeySyntax;

					*pSrc = p + 1;
					return 0;
				} else {
					sStore.push_back(nCh);
				}
			} else if (nCh == '"') {
				if (!sStore.empty()) return State::KeySyntax;
				bUseQuote = true;
			} else if (nCh == '#' || nCh == ';' || nCh == '[' || nCh == ']') {
				return State::KeySyntax;
			} else if (nCh == ' ' || nCh == '\t') {
				p = __INISKIP(p + 1);
				if (*p && *p != '=') return State::KeySyntax;
				continue;
			} else if (nCh == '=') {
				if (sStore.empty()) return State::LostKey;
				*pSrc = p + 1;
				return 0;
			} else {
				sStore.push_back(nCh);
			}

			++p;
		}

		return State::KeySyntax;
	}

	static int __INIVALUE(char * pSrc, std::string & sStore) {
		char * p = pSrc;
		bool bUseQuote = false;

		while (*p) {
			char nCh = *p;

			if (bUseQuote) {
				if (nCh == '"') {
					p = __INISKIP(p + 1);
					if (*p && *p != '#' && *p != ';') return State::ValueSyntax;
					return 0;
				} else {
					sStore.push_back(nCh);
				}
			} else if (nCh == '"') {
				if (!sStore.empty()) return State::ValueSyntax;
				bUseQuote = true;
			} else if (nCh == '[' || nCh == ']' || nCh == '=') {
				return State::ValueSyntax;
			} else if (nCh == '#' || nCh == ';') {
				return 0;
			} else if (nCh == ' ' || nCh == '\t') {
				p = __INISKIP(p + 1);
				if (*p && *p != '#' && *p != ';') return State::ValueSyntax;
				return 0;
			} else {
				sStore.push_back(nCh);
			}

			++p;
		}

		if (bUseQuote) return State::QuoteMismatch;
		return 0;
	}
}

bool IniFile::Load(const std::string & sFile) {
	_nLastError = 0;

	if (sFile.empty()) {
		_nLastError = State::Param;
		return false;
	}

	FILE * pFile = NULL;
	if (!(pFile = fopen(sFile.c_str(), "r"))) {
		_nLastError = State::FileAccess;
		return false;
	}

	Guard iGuard([pFile](){ if (pFile) fclose(pFile); });
	char pBuf[2048];
	std::string sLine, sSession, sAttribute, sValue;

	uint8_t pBomb[3] = { 0, 0, 0 };
	int nCount = fread(pBomb, 1, 3, pFile);
	if (nCount < 3 || !(pBomb[0] == 0xEF && pBomb[1] == 0xBB && pBomb[2] == 0xBF)) fseek(pFile, 0, SEEK_SET);

	while (!feof(pFile)) {
		::memset(pBuf, 0, 2048);
		fgets(pBuf, 2048, pFile);

		sLine = Trim(pBuf, " \t\r\n");
		if (sLine.empty()) continue;

		::memset(pBuf, 0, 2048);
		::memcpy(pBuf, sLine.c_str(), sLine.length());

		char nCh = *pBuf;
		char * p = pBuf;

		if (nCh == '#' || nCh == ';') {
			continue;
		} else if (nCh == '[') {
			++p;
			sSession.clear();
			if ((_nLastError = __INISESSION(p, sSession)) != 0) return false;
		} else if (sSession.empty()) {
			_nLastError = State::LostSession;
			return false;
		} else {
			sAttribute.clear();
			sValue.clear();

			if ((_nLastError = __INIATTRIBUTE(&p, sAttribute)) != 0) return false;
			p = __INISKIP(p);

			if ((_nLastError = __INIVALUE(p, sValue)) != 0) return false;
			if (sValue.empty()) continue;
			
			auto itS = _mContent.find(sSession);
			if (itS == _mContent.end()) {
				Attributes iAttr;
				iAttr[sAttribute] = sValue;
				_mContent[sSession] = iAttr;
			} else {
				itS->second[sAttribute] = sValue;
			}
		}
	}

	return true;
}

std::string IniFile::GetError() {
	switch (_nLastError) {
	case State::Success:
		return "Success";
	case State::Param:
		return "Param for IniFile::Load error";
	case State::FileAccess:
		return "Can NOT access gaven file";
	case State::LostSession:
		return "Lost session";
	case State::LostKey:
		return "Lost key";
	case State::SessionSyntax:
		return "Session syntax error";
	case State::KeySyntax:
		return "Key syntax error";
	case State::ValueSyntax:
		return "Value syntax error";
	case State::BraceMismatch:
		return "Brace mismatch";
	case State::QuoteMismatch:
		return "Quote mismatch";
	case State::EqualMismatch:
		return "Equal mismatch";
	default:
		return "Unknown error";
	}
}

std::vector<std::string> IniFile::GetSessions() {
	std::vector<std::string> vRet;
	for (auto it = _mContent.begin(); it != _mContent.end(); ++it)
		vRet.push_back(it->first);
	return std::move(vRet);
}