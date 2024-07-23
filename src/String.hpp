#pragma once
#include <algorithm>
#include <sstream>
#include <string.h>
inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
};

inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
};

namespace String
{
	static void trim(std::string& s) {
		rtrim(s);
		ltrim(s);
	};
};