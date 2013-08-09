#if !defined(_CONVERT_H)
#define _CONVERT_H

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

template<typename T>
T ConvertStringToNumber(const string& str)
{
    istringstream ss(str);
    T number = 0;

    ss >> number;

    if (ss.fail( ))
    {
    	exit(1);
        //throw invalid_argument("ConvertStringToNumber:" + str);
    }

    return number;
}

template<typename T>
string ConvertNumberToString (const T number)
{
    ostringstream ss;

    ss << number;

    return ss.str();
}



#endif //_CONVERT_H