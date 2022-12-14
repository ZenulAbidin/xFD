/*
 * Decimal - Arbitrary-Precision Decimal Library
 * Copyright (C) 2022 Ali Sherief
 * Copyright (C) 2015 Gianfranco Mariotti
 */

#ifndef TYPES_DECIMAL_H
#define TYPES_DECIMAL_H

#include <iostream>
#include <deque>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <stdint.h>

// Create an include file with this name, with the following line:
// #define __EXPLICIT__ explicit
// if you want you integrate xFD into a larger class, that has
// constructors which take integer types. This modification will
// disable the implicit type conversion from primitives to Decimal,
// so that they are passed to your class and don't trigger a compile
// error about ambiguity.

#ifdef HAVE_TYPES_EXPLICIT
#include "types/explicit.h"
#endif

#ifndef __EXPLICIT__
#define __EXPLICIT__
#endif


class Decimal;
class DecimalConstants;

using xFD = Decimal;
using xFDCon = DecimalConstants;

static inline Decimal operator"" _D(unsigned long long x);
static inline Decimal operator"" _D(long double x);
static inline Decimal operator"" _D(const char* x, size_t size);

class DecimalIllegalOperation {
public:
    DecimalIllegalOperation() {}
    DecimalIllegalOperation(const char *msg_) { msg = std::string(msg_); }
    DecimalIllegalOperation(const std::string &msg_) { msg = msg_; }
    std::string what() const { return msg; }

private:
    std::string msg;
};


class DecimalIterations {
public:
    // Represents the minimum number of error-corrected decimal places for all Decimals.
    // The number isn't actually represented in this many decimal places, and the actual
    // number is stored in Decimals.decimal.
    //
    // Any number whose absolute value is larger than the reciprocal of the smallest number
    // that is representable with this many decimal places is converted to
    // positive or negative infinity accordingly.
    int decimals;
    int E;
    int Pi;

    // Number of Newton-Rhapson iterations to run on the reciprocal of the divisor during division.
    // Zero disables the iterations.
    // It is highly recommended to set this to a number greater than zero, because several unrelated
    // functions depend on the quotient being correct (e.g. Modulus & ToHex to give the correct
    // answer for enormous (>Abs(2^64)) numbers.
    int div;
    int ln;
    int tanh;
    int sqrt;
    int trig;
    bool trunc_not_round;

    // A Decimal object can either silently generate an IEEE-754 special number
    // when otherwise-illegal operations happen, or it can throw exceptions.
    // This setting controls this behavior. It is disabled by default to prevent
    // accidential arithmetic with NaNs which are generated by the default construcor.
    //
    // This setting only affects special number generated when an exception
    // occurs (e.g. Divide/Mod by zero). Arithmetic between special numbers is
    // always done normally.
    bool throw_on_error;

    bool TOE() const { return throw_on_error; }

    DecimalIterations() {
        E = 40;
        Pi = 1; //TODO try with 2, 3, and higher
        div = 5;
        ln = 40;
        tanh = 40;
        sqrt = 40;
        decimals = 40;
        trig = 5;

        throw_on_error = true;
        trunc_not_round = false;
    }
};


/**
 * Implements an arbitrary-precision fixed-point decimal
 * with support for IEEE-754 special values
 */
class Decimal
{
private:

    // Negative zero/infinity are already differentiated by the sign variable.
    enum NumType {
        _NORMAL,
        _INFINITY,
        _NAN
    };
    char sign;
    std::deque<char> number;
    NumType type;
    // Represents the true number of decimal places in this Decimal.
    int decimals;
    DecimalIterations iterations;


    //Transformations int<-->char single digit
    inline static int CharToInt(const char& val) { return (val - '0'); };
    inline static char IntToChar(const int& val) {
        if (val < 0 || val > 9) {
            throw DecimalIllegalOperation("\"" + std::to_string(val) + "\" is not a valid decimal digit.");
        }
        return (val + '0');
    };

    //Comparator without sign, utilized by Comparators and Operations
    static int CompareNum(const Decimal& left, const Decimal& right);

    //Operations without sign and decimals, utilized by Operations
    static Decimal Sum(const Decimal& left, const Decimal& right);
    static Decimal Subtract(const Decimal& left, const Decimal& right);
    static Decimal Multiply(const Decimal& left, const Decimal& right);

    void SpecialClear() {
        iterations = DecimalIterations();
        decimals = 0;
        number.clear();
    }

public:
    //Constructors
    Decimal() {
        sign='\0';
        decimals = 0;
        iterations = DecimalIterations();
        type=NumType::_NAN;
    }
    Decimal(const DecimalIterations& its) {
        sign='\0';
        decimals = 0;
        iterations = its;
        type=NumType::_NAN;
    }
    __EXPLICIT__ Decimal(const char* strNum) {
        *this=strNum;
        type=NumType::_NORMAL;
    }
    __EXPLICIT__ Decimal(std::string strNum) {
        *this=strNum;
        type=NumType::_NORMAL;
    }
    __EXPLICIT__ Decimal(char Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(unsigned char Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = '+';
    }
    __EXPLICIT__ Decimal(short Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(unsigned short Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = '+';
    }
    __EXPLICIT__ Decimal(int Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(unsigned int Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = '+';
    }
    __EXPLICIT__ Decimal(long Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(unsigned long Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = '+';
    }
    __EXPLICIT__ Decimal(long long Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(unsigned long long Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = '+';
    }
    __EXPLICIT__ Decimal(float Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(double Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }
    __EXPLICIT__ Decimal(long double Num) {
        *this=Num;
        type=NumType::_NORMAL;
        sign = (Num < 0) ? '-' : '+';
    }

    static Decimal Inf() {
        Decimal a;
        a.type = NumType::_INFINITY;
        a.sign = '+';
        return a;
    }

    // Do NOT put a leading 0x or 0X.
    static Decimal FromHex(const std::string& hex);

    static Decimal NaN() { return Decimal(); }

    bool IsInf() const {
        return type == NumType::_INFINITY;
    }

    bool IsNaN() const {
        return type == NumType::_NAN;
    }

    Decimal operator()(const DecimalIterations& _iterations = DecimalIterations()) const {
        Decimal a = *this;
        a.iterations = _iterations;
        if (a.iterations.decimals < a.decimals) {
            a.iterations.decimals = a.decimals;
        }
        return a;
    }

    //Assignment operators
    Decimal& operator=(const char* strNum);
    Decimal& operator=(std::string strNum);
    Decimal& operator=(char Num);
    Decimal& operator=(unsigned char Num);
    Decimal& operator=(short Num);
    Decimal& operator=(unsigned short Num);
    Decimal& operator=(int Num);
    Decimal& operator=(unsigned int Num);
    Decimal& operator=(long Num);
    Decimal& operator=(unsigned long Num);
    Decimal& operator=(long long Num);
    Decimal& operator=(unsigned long long Num);
    Decimal& operator=(float Num);
    Decimal& operator=(double Num);
    Decimal& operator=(long double Num);

    //Operations
    friend Decimal operator+(const Decimal& left_, const Decimal& right_);
    friend Decimal operator+(const Decimal& left, const char& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const unsigned char& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const short& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const unsigned short& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const int& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const unsigned int& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const long& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const unsigned long& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const float& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const double& right)
    { return left + Decimal(right); }
    friend Decimal operator+(const Decimal& left, const long double& right)
    { return left + Decimal(right); }

    Decimal& operator+=(const Decimal& right) {
        *this = *this + right;
        return *this;
    }
    Decimal& operator+=(const char& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const unsigned char& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const short& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const unsigned short& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const int& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const unsigned int& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const long& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const unsigned long& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const float& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const double& right) {
        *this = *this + Decimal(right);
        return *this;
    }
    Decimal& operator+=(const long double& right) {
        *this = *this + Decimal(right);
        return *this;
    }

    friend Decimal operator-(const Decimal& left_, const Decimal& right_);
    friend Decimal operator-(const Decimal& left, const char& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const unsigned char& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const short& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const unsigned short& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const int& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const unsigned int& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const long& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const unsigned long& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const float& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const double& right)
    { return left - Decimal(right); }
    friend Decimal operator-(const Decimal& left, const long double& right)
    { return left - Decimal(right); }

    Decimal& operator-=(const Decimal& right) {
        *this = *this - right;
        return *this;
    }
    Decimal& operator-=(const char& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const unsigned char& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const short& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const unsigned short& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const int& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const unsigned int& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const long& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const unsigned long& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const float& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const double& right) {
        *this = *this - Decimal(right);
        return *this;
    }
    Decimal& operator-=(const long double& right) {
        *this = *this - Decimal(right);
        return *this;
    }

    friend Decimal operator*(const Decimal& left, const Decimal& right);
    friend Decimal operator*(const Decimal& left, const char& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const unsigned char& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const short& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const unsigned short& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const int& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const unsigned int& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const long& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const unsigned long& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const float& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const double& right)
    { return left * Decimal(right); }
    friend Decimal operator*(const Decimal& left, const long double& right)
    { return left * Decimal(right); }

    Decimal& operator*=(const Decimal& right) {
        *this = *this * right;
        return *this;
    }
    Decimal& operator*=(const char& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const unsigned char& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const short& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const unsigned short& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const int& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const unsigned int& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const long& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const unsigned long& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const float& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const double& right) {
        *this = *this * Decimal(right);
        return *this;
    }
    Decimal& operator*=(const long double& right) {
        *this = *this * Decimal(right);
        return *this;
    }

    friend Decimal operator/(const Decimal& left, const Decimal& right);
    friend Decimal operator/(const Decimal& left, const char& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const unsigned char& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const short& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const unsigned short& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const int& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const unsigned int& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const long& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const unsigned long& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const float& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const double& right)
    { return left / Decimal(right); }
    friend Decimal operator/(const Decimal& left, const long double& right)
    { return left / Decimal(right); }

    Decimal& operator/=(const Decimal& right) {
        *this = *this / right;
        return *this;
    }
    Decimal& operator/=(const char& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const unsigned char& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const short& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const unsigned short& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const int& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const unsigned int& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const long& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const unsigned long& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const float& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const double& right) {
        *this = *this / Decimal(right);
        return *this;
    }
    Decimal& operator/=(const long double& right) {
        *this = *this / Decimal(right);
        return *this;
    }

    static Decimal Divide(const Decimal& left, const Decimal& right);
    static Decimal Mod(const Decimal& left, const Decimal& right);

    friend Decimal operator%(const Decimal& left, const Decimal& right);
    friend Decimal operator%(const Decimal& left, const char& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const unsigned char& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const short& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const unsigned short& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const int& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const unsigned int& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const long& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const unsigned long& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const float& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const double& right)
    { return left % Decimal(right); }
    friend Decimal operator%(const Decimal& left, const long double& right)
    { return left % Decimal(right); }

    Decimal& operator%=(const Decimal& right) {
        *this = *this % right;
        return *this;
    }
    Decimal& operator%=(const char& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const unsigned char& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const short& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const unsigned short& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const int& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const unsigned int& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const long& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const unsigned long& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const float& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const double& right) {
        *this = *this % Decimal(right);
        return *this;
    }
    Decimal& operator%=(const long double& right) {
        *this = *this % Decimal(right);
        return *this;
    }


    Decimal& operator++(int i) { *this = *this+1; return *this; };
    Decimal& operator++() { *this = *this+1; return *this; };
    Decimal& operator--(int i) { *this = *this-1; return *this; };
    Decimal& operator--() { *this = *this-1; return *this; };


    //Comparators
    bool operator== (const Decimal& right) const;
    bool operator== (const char& right) const
    {return *this == Decimal(right); }
    bool operator== (const unsigned char& right) const
    {return *this == Decimal(right); }
    bool operator== (const short& right) const
    {return *this == Decimal(right); }
    bool operator== (const unsigned short& right) const
    {return *this == Decimal(right); }
    bool operator== (const int& right) const
    {return *this == Decimal(right); }
    bool operator== (const unsigned int& right) const
    {return *this == Decimal(right); }
    bool operator== (const long& right) const
    {return *this == Decimal(right); }
    bool operator== (const unsigned long& right) const
    {return *this == Decimal(right); }
    bool operator== (const float& right) const
    {return *this == Decimal(right); }
    bool operator== (const double& right) const
    {return *this == Decimal(right); }
    bool operator== (const long double& right) const
    {return *this == Decimal(right); }

    bool operator!= (const Decimal& right) const;
    bool operator!= (const char& right) const
    {return *this != Decimal(right); }
    bool operator!= (const unsigned char& right) const
    {return *this != Decimal(right); }
    bool operator!= (const short& right) const
    {return *this != Decimal(right); }
    bool operator!= (const unsigned short& right) const
    {return *this != Decimal(right); }
    bool operator!= (const int& right) const
    {return *this != Decimal(right); }
    bool operator!= (const unsigned int& right) const
    {return *this != Decimal(right); }
    bool operator!= (const long& right) const
    {return *this != Decimal(right); }
    bool operator!= (const unsigned long& right) const
    {return *this != Decimal(right); }
    bool operator!= (const float& right) const
    {return *this != Decimal(right); }
    bool operator!= (const double& right) const
    {return *this != Decimal(right); }
    bool operator!= (const long double& right) const
    {return *this != Decimal(right); }

    bool operator> (const Decimal& right) const;
    bool operator> (const char& right) const
    {return *this > Decimal(right); }
    bool operator> (const unsigned char& right) const
    {return *this > Decimal(right); }
    bool operator> (const short& right) const
    {return *this > Decimal(right); }
    bool operator> (const unsigned short& right) const
    {return *this > Decimal(right); }
    bool operator> (const int& right) const
    {return *this > Decimal(right); }
    bool operator> (const unsigned int& right) const
    {return *this > Decimal(right); }
    bool operator> (const long& right) const
    {return *this > Decimal(right); }
    bool operator> (const unsigned long& right) const
    {return *this > Decimal(right); }
    bool operator> (const float& right) const
    {return *this > Decimal(right); }
    bool operator> (const double& right) const
    {return *this > Decimal(right); }
    bool operator> (const long double& right) const
    {return *this > Decimal(right); }

    bool operator>= (const Decimal& right) const;
    bool operator>= (const char& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const unsigned char& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const short& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const unsigned short& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const int& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const unsigned int& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const long& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const unsigned long& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const float& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const double& right) const
    {return *this >= Decimal(right); }
    bool operator>= (const long double& right) const
    {return *this >= Decimal(right); }

    bool operator< (const Decimal& right) const;
    bool operator< (const char& right) const
    {return *this < Decimal(right); }
    bool operator< (const unsigned char& right) const
    {return *this < Decimal(right); }
    bool operator< (const short& right) const
    {return *this < Decimal(right); }
    bool operator< (const unsigned short& right) const
    {return *this < Decimal(right); }
    bool operator< (const int& right) const
    {return *this < Decimal(right); }
    bool operator< (const unsigned int& right) const
    {return *this < Decimal(right); }
    bool operator< (const long& right) const
    {return *this < Decimal(right); }
    bool operator< (const unsigned long& right) const
    {return *this < Decimal(right); }
    bool operator< (const float& right) const
    {return *this < Decimal(right); }
    bool operator< (const double& right) const
    {return *this < Decimal(right); }
    bool operator< (const long double& right) const
    {return *this < Decimal(right); }

    bool operator<= (const Decimal& right) const;
    bool operator<= (const char& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const unsigned char& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const short& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const unsigned short& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const int& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const unsigned int& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const long& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const unsigned long& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const float& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const double& right) const
    {return *this <= Decimal(right); }
    bool operator<= (const long double& right) const
    {return *this <= Decimal(right); }

    //Stream Operators
    friend std::ostream& operator<<(std::ostream& out, const Decimal& right);
    friend std::istream& operator>>(std::istream& in, Decimal& right);

    //Transformation and Error-Check Methods
    bool FitsChar8() const;
    bool FitsUChar8() const;
    bool FitsShort16() const;
    bool FitsUShort16() const;
    bool FitsInt32() const;
    bool FitsUInt32() const;
    bool FitsLong64() const;
    bool FitsULong64() const;
    bool FitsLongLong64() const;
    bool FitsULongLong64() const;
    bool FitsFloat() const;
    bool FitsDouble() const;
    bool FitsLongDouble() const;

    char ToChar8() const;
    unsigned char ToUChar8() const;
    short ToShort16() const;
    unsigned short ToUShort16() const;
    int ToInt32() const;
    unsigned int ToUInt32() const;
    long ToLong64() const;
    unsigned long ToULong64() const;
    long long ToLongLong64() const;
    unsigned long long ToULongLong64() const;
    float ToFloat() const;
    double ToDouble() const;
    long double ToLongDouble() const;

    operator char() { return ToChar8(); }
    operator unsigned char() { return ToUChar8(); }
    operator short() { return ToShort16(); }
    operator unsigned short() { return ToUShort16(); }
    operator int() { return ToInt32(); }
    operator unsigned int() { return ToUInt32(); }
    operator long() { return ToLong64(); }
    operator unsigned long() { return ToULong64(); }
    operator long long() { return ToLongLong64(); }
    operator unsigned long long() { return ToULongLong64(); }
    operator float() { return ToFloat(); }
    operator double() { return ToDouble(); }
    operator long double() { return ToLongDouble(); }
    operator std::string() { return ToString(); }


    inline int8_t ToInt8_T() const {return (int8_t) this->ToChar8(); }
    inline uint8_t ToUInt8_T() const {return (uint8_t) this->ToUChar8(); }
    inline int16_t ToInt16_T() const {return (int16_t) this->ToShort16(); }
    inline uint16_t ToUInt16_T() const {return (uint16_t) this->ToUShort16(); }
    inline int32_t ToInt32_T() const {return (int32_t) this->ToInt32(); }
    inline uint32_t ToUInt32_T() const {return (uint32_t) this->ToUInt32(); }
    inline int64_t ToInt64_T() const {return (int64_t) this->ToLong64(); }
    inline uint64_t ToUInt64_T() const {return (uint64_t) this->ToULong64(); }

    std::string ToString() const;
    std::string ToFixedString() const;
    std::string ToHex(bool lowercase=false) const;

    bool GetThrowOnError() const { return iterations.throw_on_error; }
    void SetThrowOnError(bool toe) { iterations.throw_on_error = toe; }

    void SetPrecision(int prec);    //Approximate number or Increase number decimals

    void LeadTrim();    //Remove number leading zeros, utilized by Operations without sign
    void TrailTrim();     //Remove number non significant trailing zeros

    //Math/Scientific methods
    
    static Decimal Pow(const Decimal& x);
    static Decimal Pow(const Decimal& x, const Decimal& y);
    static Decimal Ln(const Decimal& x);
    static Decimal Log(const Decimal& b, const Decimal& x);
    static Decimal Log10(const Decimal& x);
    static Decimal Log2(const Decimal& x);

    Decimal operator^(const Decimal& x) {
        return xFD::Pow(*this, x);
    }

    static Decimal Sqrt(const Decimal& x) {
        return xFD::Pow(x, 0.5_D);
    }

    static Decimal Sin(const Decimal& x);
    static Decimal Cos(const Decimal& x);
    static Decimal Tan(const Decimal& x);
    static Decimal Cot(const Decimal& x);
    static Decimal Sec(const Decimal& x);
    static Decimal Csc(const Decimal& x);

    static Decimal Asin(const Decimal& x);
    static Decimal Acos(const Decimal& x);
    static Decimal Atan(const Decimal& x);
    // Atan2 is used in calculating the angle in polar coordinates.
    static Decimal Atan2(const Decimal& x, const Decimal& y);

    // "Hypotenuse" or the length of the line projecting from the unit circle
    // in polar coordinates.
    static Decimal Hypot(const Decimal& x, const Decimal& y) {
        return xFD::Sqrt(x*x + y*y);
    }

    static Decimal Acot(const Decimal& x);
    static Decimal Asec(const Decimal& x);
    static Decimal Acsc(const Decimal& x);

    static Decimal Sinh(const Decimal& x);
    static Decimal Cosh(const Decimal& x);
    static Decimal Tanh(const Decimal& x);
    static Decimal Coth(const Decimal& x);
    static Decimal Sech(const Decimal& x);
    static Decimal Csch(const Decimal& x);

    static Decimal Asinh(const Decimal& x);
    static Decimal Acosh(const Decimal& x);
    static Decimal Atanh(const Decimal& x);
    static Decimal Acoth(const Decimal& x);
    static Decimal Asech(const Decimal& x);
    static Decimal Acsch(const Decimal& x);

    static Decimal Erf(const Decimal& x);


    static Decimal Abs(const Decimal& x);
    static Decimal Sign(const Decimal& x);
    static Decimal TrigPhaseCorrect(const Decimal& x);
    Decimal operator-() const;
    static Decimal Factorial(const Decimal& x);

    static Decimal Binomial(const Decimal& x, const Decimal& y, const Decimal& n);
    static Decimal nPr(const Decimal& n, const Decimal& k);
    static Decimal nCr(const Decimal& n, const Decimal& k);

    static Decimal Floor(const Decimal& x); 
    static Decimal Ceil(const Decimal& x) {
        return xFD::Floor(x) + 1_D;
    }
    static Decimal Round(const Decimal& x, int places = 0);
    Decimal Inc();
    Decimal Dec();

    //Miscellaneous Methods
    inline int Decimals() const { return decimals; };
    inline int Ints() const { return number.size()-decimals; };
    inline bool IsInt() const { return decimals == 0; }
    inline int MemorySize() const { return sizeof(*this)+number.size()*sizeof(char); };
    std::string Exp() const;

};


// NOTICE: This operator can convert SIGNED 64-bit integers to Decimal. This is required in
// order to represent negative constants correctly. If you need to convert a literal >2^63
// to a Decimal, use the string constructor instead.
static inline Decimal operator"" _D(unsigned long long x)
{
    return Decimal(static_cast<long long>(x));
}

static inline Decimal operator"" _D(long double x)
{
    return Decimal(x);
}

static inline Decimal operator"" _D(const char* x, size_t size)
{
    std::string s(x, size);
    return Decimal(s);
}

class DecimalConstants {
public:
    Decimal pE; // e
    Decimal pPi; // Pi
    Decimal p_1Pi; // 1/Pi
    Decimal pPi2;  // Pi/2
    Decimal pPi4;  // Pi/4
    Decimal pLn2;  // ln(2)
    Decimal pLn10; // ln(10)
    Decimal p_2Pi; // 2/Pi
    Decimal p_2SqrtPi;  // 2/sqrt(Pi)
    Decimal pLog2E; // Log2(e)
    Decimal pLog10E; // Log10(e)
    Decimal pSqrt2; // sqrt(2)
    Decimal p_1Sqrt2; // 1/sqrt(2)

private:
    DecimalIterations iterations;
    void GenE();

    void GenPi() {
        pPi = 1_D/p_1Pi;
    };

    void Gen_1Pi();

    void GenPi2() {
        pPi2 = pPi/2_D;
    }
    void GenPi4() {
        pPi4 = p_1Pi/4_D;
    }
    void GenLn2() {
        pLn2 = xFD::Ln(2_D);
    }
    void GenLn10() {
        pLn10 = xFD::Ln(10_D);
    }

    void Gen_2Pi() {
        p_2Pi = p_1Pi * 2_D;
    }

    void Gen_2SqrtPi() {
        p_2SqrtPi = 2_D / xFD::Sqrt(pPi);
    }

    void GenLog2E() {
        pLog2E = xFD::Log(2_D, pE);
    }
    void GenLog10E() {
        pLog10E = xFD::Log(10_D, pE);
    }
    void GenSqrt2() {
        pSqrt2 = xFD::Sqrt(2_D);
    }
    void Gen_1Sqrt2() {
        p_1Sqrt2 = 1_D/pSqrt2;
    }

    Decimal ImprovisedSqrt(const Decimal& x) const;


public:

    /**
     * Calculates $e$ using the infinite series:
     *
     * $e = \sum_{n=0}^{\inf}{\frac{1}{n!}}
     *
     * @param iterations        the number of iterations for calculation.
     *                          Higher iterations give more digits of precision.
     */
    static Decimal E() {
        DecimalConstants c;
        return c.pE;
    }


    /**
     * Calculates $\frac{1}{\pi}$ using an optimized Chudnovsky algorithm.
     * See https://en.wikipedia.org/wiki/Chudnovsky_algorithm for details.
     *
     * @param iterations        the number of iterations for calculation.
     *                          Higher iterations give more digits of precision.
     */
    static Decimal Pi() {
        DecimalConstants c;
        return c.pPi;
    }

    /**
     * Calculates $\pi$.
     *
     * @param iterations        the number of iterations for calculation.
     *                          Higher iterations give more digits of precision.
     *
     * @see _1Pi
     */
    static Decimal _1Pi() {
        DecimalConstants c;
        return c.p_1Pi;
    }

    static Decimal Pi2() {
        DecimalConstants c;
        return c.pPi2;
    }

    static Decimal Pi4() {
        DecimalConstants c;
        return c.pPi4;
    }

    static Decimal Ln2() {
        DecimalConstants c;
        return c.pLn2;
    }

    static Decimal Ln10() {
        DecimalConstants c;
        return c.pLn10;
    }

    static Decimal _2Pi() {
        DecimalConstants c;
        return c.p_2Pi;
    }

    static Decimal _2SqrtPi() {
        DecimalConstants c;
        return c.p_2SqrtPi;
    }

    static Decimal Log2E() {
        DecimalConstants c;
        return c.pLog2E;
    }

    static Decimal Log10E() {
        DecimalConstants c;
        return c.pLog10E;
    }

    static Decimal Sqrt2() {
        DecimalConstants c;
        return c.pSqrt2;
    }

    static Decimal _1Sqrt2() {
        DecimalConstants c;
        return c.p_1Sqrt2;
    }

    DecimalIterations Iterations() const { return iterations; }
    void SetIterations(const DecimalIterations& iterations) {
        this->iterations = iterations;
        GenE();
        GenPi();
        Gen_1Pi();
        GenPi2();
        GenPi4();
        GenLn2();
        GenLn10();
        Gen_2Pi();
        Gen_2SqrtPi();
        GenLog2E();
        GenLog10E();
        GenSqrt2();
        Gen_1Sqrt2();
    }

    DecimalConstants(const DecimalIterations iterations = DecimalIterations()) {
        SetIterations(iterations);
    }
};

class DecimalSequence {
    public:
        int iterations;
        DecimalSequence() {}
        virtual Decimal pTerm(const Decimal& n) const = 0;
};

    /**
     * Class for calculating the Bernoullis $B_n$ (This is *not* $B_2n$!)
     *
     * This calculation utilizes the algorithm given in the research paper:
     * Chowla and Hartung, An "exact" formula for the nth Bernoulli number,
     * Acta Arithmetica 22 (1972) 113-115.
     *
     * Stirling approximations are not used because they aren't accurate enough.
     *
     * @param n                 the sequence term of the desired Bernoulli number
     */
class SeqBernoulli: public DecimalSequence {
    public:
        SeqBernoulli() { iterations = 40; }

        Decimal pTerm(const Decimal& n) const override;

        /**
         * Calculates the n'th Bernoulli number.
         *
         * @param n     The term number to get. Must be an integer.
         */
        static Decimal Term(const Decimal& n) {
            SeqBernoulli a;
            return a.pTerm(n);
        }
};

#endif /* TYPES_DECIMAL_H */
