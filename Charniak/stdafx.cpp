// stdafx.cpp : source file that includes just the standard includes
// Charniak.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

int isnan(double x) { return x != x; }
int isinf(double x) { return !isnan(x) && isnan(x - x); }
double log2( double n )  
{  
    // log(n)/log(2) is log2.  
    return log( n ) / log((float)2);  
}
