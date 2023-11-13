#pragma once
#include <string>
#include <iomanip> //Needed for setprecision()
#include <sstream> //Needed for stringstream
#include <basetsd.h>
/**
* Takes a float and converts it into a std::string
* @param f The float to be converted
* @param precision the number of decimal digits to take from the float
* @param fixed converts the string with fixed precision if true
* @return The given float as a std::string
*/
inline std::string toString(const float& f, const int precision = 4, const bool fixed = 1)
{
    //stream to be used in the conversion
    std::stringstream ss;

    //the result to be returned at the end
    std::string res;

    //set if the conversion precision is fixed
    if (fixed)
    {
        ss << std::fixed;
    }

    //set the precision and output the float onto the stream
    ss << std::setprecision(precision) << f;

    //input the result string from the stream
    ss >> res;

    //return the result
    return res;
}

/**
* Takes a unsigned long long and converts it into a std::string
* @param f The unsigned long long to be converted
* @return The given float as a std::string
*/
inline std::string toString(const unsigned long long& f)
{
    //stream to be used in the conversion
    std::stringstream ss;

    //the result to be returned at the end
    std::string res;

    //output the unsigned long long onto the stream
    ss << f;

    //input the result string from the stream
    ss >> res;

    //return the result
    return res;
}

/**
* Takes a unsigned long and converts it into a std::string
* @param f The unsigned long to be converted
* @return The given float as a std::string
*/
inline std::string toString(const unsigned long& f)
{
    //stream to be used in the conversion
    std::stringstream ss;

    //the result to be returned at the end
    std::string res;

    //output the unsigned long long onto the stream
    ss << f;

    //input the result string from the stream
    ss >> res;

    //return the result
    return res;
}

/**
* Takes a unsigned long and converts it into a std::string
* @param f The unsigned long to be converted
* @return The given float as a std::string
*/
inline std::string toString(const int& f)
{
    //stream to be used in the conversion
    std::stringstream ss;

    //the result to be returned at the end
    std::string res;

    //output the unsigned long long onto the stream
    ss << f;

    //input the result string from the stream
    ss >> res;

    //return the result
    return res;
}

/**
* Converts a Hexadecimal HMODULE into a std::string
* @param num The hexadecimal number to convert
* @return The Hexadecimal in a std::string
*/
inline std::string toHex(const HMODULE& num)
{
    //create stream
    std::stringstream ss;

    //set format and output the number
    ss << std::setfill('0') << std::setw(sizeof(num) * 2) << std::hex << num;

    //create and get the return string
    std::string value;

    ss >> value;

    return value;
}