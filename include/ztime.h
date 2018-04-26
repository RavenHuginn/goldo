//#############################################################################
//ztime.h
//(c) Zhe - zhe@@@soldo.in
//#############################################################################
#pragma once

#include <ctime>
///////////////////////////////////////////////////////////////////////////////
std::string z_get_now_time_str(const std::string _fmt) {
	char timeString[20];
	time_t now = time(0);
	struct tm tstruct;
    tstruct = *localtime(&now);
	
	if (std::strftime(timeString, sizeof(timeString), _fmt.c_str(), &tstruct) == 0) 
		{
			throw std::runtime_error("time buffer is too small");
		}
	return std::string(timeString);
}
////////////////////////////////////////////////////////////////////////////////

