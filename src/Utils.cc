#include	<Utils.h>
#include	<DateTime.h>

#if defined(_WIN32)
#	include		<Objbase.h>
#else
#	include		<cstring>
#	include		<uuid/uuid.h>
#endif

using namespace std;

template<> bool Convert<bool>(const string & s) {
	if (s.length() == 0) return false;

	std::string r = ToUpper(s);
	if (r == "TRUE" || r == "T") return true;
	else if (r == "FALSE" || r == "F") return false;
	else return atol(s.c_str()) != 0;
}

template<> int8_t Convert<int8_t>(const string & s) {
	return atol(s.c_str()) & 0xFF;
}

template<> int16_t Convert<int16_t>(const string & s) {
	return atol(s.c_str()) & 0xFFFF;
}

template<> int32_t Convert<int32_t>(const string & s) {
	return atol(s.c_str());
}

template<> int64_t Convert<int64_t>(const string & s) {
	return atoll(s.c_str());
}

template<> uint8_t Convert<uint8_t>(const string & s) {
	return (uint8_t)(atol(s.c_str()) & 0xFF);
}

template<> uint16_t Convert<uint16_t>(const string & s) {
	return (uint16_t)(atol(s.c_str()) & 0xFFFF);
}

template<> uint32_t Convert<uint32_t>(const string & s) {
	return (uint32_t)(atol(s.c_str()));
}

template<> uint64_t Convert<uint64_t>(const string & s) {
	return (uint64_t)(atoll(s.c_str()));
}

template<> float Convert<float>(const string & s) {
	return (float)(atof(s.c_str()));
}

template<> double Convert<double>(const string & s) {
	return atof(s.c_str());
}

template<> const char * Convert<const char *>(const string & s) {
	return s.c_str();
}

template<> string Convert<string>(const string & s) {
	return s;
}

template<> DateTime Convert<DateTime>(const string & s) {
	int nY, nMon, nD, nH, nM, nS;
	
#if defined(_MSC_VER)
	sscanf_s(s.c_str(), "%d-%d-%d %d:%d:%d", &nY, &nMon, &nD, &nH, &nM, &nS);
#else
	sscanf(s.c_str(), "%d-%d-%d %d:%d:%d", &nY, &nMon, &nD, &nH, &nM, &nS);
#endif

	return DateTime(nY, nMon, nD, nH, nM, nS);
}

std::string ToString(bool b) { return b ? "T" : "F"; }
std::string ToString(int8_t n) { return std::to_string(n); }
std::string ToString(int16_t n) { return std::to_string(n); }
std::string ToString(int32_t n) { return std::to_string(n); }
std::string ToString(int64_t n) { return std::to_string(n); }
std::string ToString(uint8_t n) { return std::to_string(n); }
std::string ToString(uint16_t n) { return std::to_string(n); }
std::string ToString(uint32_t n) { return std::to_string(n); }
std::string ToString(uint64_t n) { return std::to_string(n); }
std::string ToString(const std::string & s) { return s; }
std::string ToString(const char * p) { return p; }

string ToLower(const string & s) {
	string sRet = s;
	for (size_t i = 0; i < sRet.length(); ++i) {
		if (s[i] >= 'A' && s[i] <= 'Z') sRet[i] += 32;
	}
	return sRet;
}

string ToUpper(const string & s) {
	string sRet = s;
	for (size_t i = 0; i < sRet.length(); ++i) {
		if (s[i] >= 'a' && s[i] <= 'z') sRet[i] -= 32;
	}
	return sRet;
}

string Replace(const string & s, const string & sPattern, const string & sReplace) {
	string sDst;
	sDst.reserve(s.length() * 2);

	string::size_type nStart = 0;
	string::size_type nPos = s.find(sPattern, nStart);

	while (nPos != string::npos) {
		sDst.append(s.data() + nStart, nPos - nStart);
		sDst.append(sReplace);

		nStart = nPos + sPattern.length();
		nPos = s.find(sPattern, nStart);
	}

	if (nStart < s.length() - 1) sDst.append(s.begin() + nStart, s.end());
	return sDst;
}

string TrimStart(const string & s, const string & sTrim) {
	string r = s;

	string::size_type nStart = r.find_first_not_of(sTrim);
	if (nStart == string::npos) r = "";
	else r.erase(0, nStart);

	return std::move(r);
}

string TrimEnd(const string & s, const string & sTrim) {
	string r = s;

	string::size_type nEnd = r.find_last_not_of(sTrim);
	if (nEnd == string::npos) r = "";
	else r.erase(nEnd + 1);

	return std::move(r);
}

string Trim(const string & s, const string & sTrim) {
	string r = s;
	string::size_type nStart = r.find_first_not_of(sTrim);
	string::size_type nEnd = r.find_last_not_of(sTrim);

	if (nStart == string::npos) r = "";
	else r = r.substr(nStart, nEnd - nStart + 1);

	return std::move(r);
}

vector<string> Split(const string & s, const string & sDeli, bool bIgnoreEmpty /* = true */) {
	vector<string> vRet;

	string::size_type nStart = 0, nEnd = 0;
	while (nEnd < s.length()) {
		char nTemp = s[nEnd];
		if (sDeli.find(nTemp) != string::npos) {
			if (nStart != nEnd) vRet.push_back(s.substr(nStart, nEnd - nStart));
			else if(!bIgnoreEmpty) vRet.push_back("");
			++nEnd;
			nStart = nEnd;
		} else {
			++nEnd;
		}
	}

	if (nStart != nEnd)	vRet.push_back(s.substr(nStart));
	return vRet;
}

#if defined(_WIN32)
string CreateID() {
	GUID		iGuid;
	uint32_t	pData[4];
	char		pHex[64];

	::CoInitialize(NULL);
	::CoCreateGuid(&iGuid);
	::CoUninitialize();

	::memset(pHex, 0, 64);
	::memcpy(pData, &iGuid, 16);

	snprintf(pHex, 64, "%08X%08X%08X%08X", pData[0], pData[1], pData[2], pData[3]);
	return std::string(pHex);
}
#else
string CreateID() {
	uuid_t		iUUID;
	uint32_t	pData[4];
	char		pHex[64];

	uuid_generate(iUUID);
	::memset(pHex, 0, 64);
	::memcpy(pData, iUUID, 16);

	snprintf(pHex, 64, "%08X%08X%08X%08X", pData[0], pData[1], pData[2], pData[3]);
	return std::string(pHex);
}
#endif