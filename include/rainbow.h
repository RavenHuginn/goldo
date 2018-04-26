//#############################################################################
//rainbow.h
//(c) Zhe - zhe@@@lanthaneum.com
//USAGE >>
//
//std::cout 
//	<< grey << "@#$%&" << white << "@#$%&" << navy << "@#$%&" << green << "@#$%&" << teal << "@#$%&" << maroon 
//	<< "@#$%&" << purple << "@#$%&" << blue << "@#$%&" << lime << "@#$%&" << cyan << "@#$%&" << red << "@#$%&" 
//	<< magenta << "@#$%&" << khaki << "@#$%&" << yellow << "@#$%&" << std::endl;
//#############################################################################
#pragma once

#include <iosfwd>
#include <iostream>
/*
enum colors_enum
{
	Black       = 0,
	Grey        = FOREGROUND_INTENSITY,
	
	Silver      = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE,
	White       = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	Navy        = FOREGROUND_BLUE,
	Green       = FOREGROUND_GREEN,
	Teal        = FOREGROUND_GREEN | FOREGROUND_BLUE,
	Maroon      = FOREGROUND_RED,
	Purple      = FOREGROUND_RED   | FOREGROUND_BLUE,
	Blue        = FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
	Lime        = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	Cyan        = FOREGROUND_GREEN | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
	Red         = FOREGROUND_RED   | FOREGROUND_INTENSITY,
	Magenta     = FOREGROUND_RED   | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
	Khaki       = FOREGROUND_RED   | FOREGROUND_GREEN,
	Yellow      = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
};
*/
//#############################################################################
inline std::ostream& grey(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::White);
    return s;
}
//#############################################################################
inline std::ostream& white(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightWhite);
    return s;
}
//#############################################################################
inline std::ostream& navy(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Blue);
    return s;
}
//#############################################################################
inline std::ostream& green(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Green);
    return s;
}
//#############################################################################
inline std::ostream& teal(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Cyan);
    return s;
}
//#############################################################################
inline std::ostream& purple(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Magenta);
    return s;
}
//#############################################################################
inline std::ostream& maroon(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Red);
    return s;
}
//#############################################################################
inline std::ostream& blue(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightBlue);
    return s;
}
//#############################################################################
inline std::ostream& lime(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightGreen);
    return s;
}
//#############################################################################
inline std::ostream& cyan(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightCyan);
    return s;
}
//#############################################################################
inline std::ostream& red(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightRed);
    return s;
}
//#############################################################################
inline std::ostream& magenta(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightMagenta);
    return s;
}
//#############################################################################
inline std::ostream& khaki(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::Yellow);
    return s;
}
//#############################################################################
inline std::ostream& yellow(std::ostream &s)
{
	Common::Console::setTextColor(Common::Console::Color::BrightYellow);
    return s;
}
//#############################################################################

/*
struct color {
    color(WORD attribute):m_color(attribute){};
    WORD m_color;
};

template <class _Elem, class _Traits>
std::basic_ostream<_Elem,_Traits>& 
      operator<<(std::basic_ostream<_Elem,_Traits>& i, color& c)
{
    HANDLE hStdout=GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTextAttribute(hStdout,c.m_color);
    return i;
}*/
//////////////////
