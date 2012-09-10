/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "stdafx.h"
#include "auxify.h"
#include <iostream>
#include "Term.h"
#include "ECString.h"

Char* 	suffixes[] = {
L"'VE",
L"'M",
L"'LL",
L"'D",
L"'S",
L"'RE",
0
};

Char* 	auxgs[] = {
L"BEIN",
L"HAVING",
L"BEING",
0
};


Char* 	auxs[] = {
L"MAHT",
L"SHULD",
L"WILL",
L"WAS",
L"OUGHTA",
L"AHM",
L"NEED",
L"MAYE",
L"WILLYA",
L"WHADDYA",
L"HATH",
L"HAVE",
L"WERE",
L"IS",
L"HAS",
L"MUST",
L"DID",
L"HAD",
L"DO",
L"MIGHT",
L"WOULD",
L"SHALL",
L"SHOULD",
L"OUGHT",
L"COULD",
L"DOES",
L"HAFTA",
L"BE",
L"KIN",
L"CAN",
L"ART",
L"BEEN",
L"DONE",
L"ARE",
L"DOO",
L"MAY",
L"AM",
0
};

bool
hasAuxSuf( ECString word )
{
    size_t pos = word.find_first_of(L"\'");
    if(pos == -1) return false;
    ECString apostrophe = word.substr(pos, word.length()-pos);
    for( int i = 0; suffixes[i]; i++)
    {
	if( apostrophe == suffixes[i] ) 
	    return true;
    }
    return false;
}

bool
isAux( ECString word )
{
    for( int i = 0; auxs[i]; i++)
    {
	if( word == auxs[i] )
	    return true;
    }
    return false;
}

bool
isAuxg( ECString word )
{
    for( int i = 0; auxgs[i]; i++)
    {
	if( word == auxgs[i] ) 
	    return true;
    }
    return false;
}

Char* verbs[] = {
L"VB",
L"VBD",
L"VBG",
L"VBN",
L"VBP",
L"VBZ",
0
};

bool
isVerb( ECString tag )
{
    for( int i = 0; verbs[i]; i++)
	if( tag == verbs[i] ) 
	    return true;
    return false;
}

Char*
toUpper(const Char* str, Char* temp)
{
  int l = wcslen(str);
  assert(l < 128);
  for(int i = 0 ; i <= l ; i++)
    {
      Char n = str[i];
      int ni = (int)n;
      if(ni >= 97 && ni <= 122)
	{
	  temp[i] = (Char)(ni-32);
	}
      else temp[i] = n;
    }
  return temp;
}

ECString
auxify(ECString wM, ECString trmM)
{
  Char temp[128];
  ECString w = toUpper(wM.c_str(),temp);
  ECString trm = toUpper(trmM.c_str(),temp);
  if( isVerb( trm ) )
    {
      //wcout << "saw verb " << trm << " " << wM << endl;
      if( isAux( w ) || hasAuxSuf( w ) )
	{
	  //wcout << "was aux " << w << endl;
	  return L"AUX";
	}
      else if( isAuxg( w ) )
	{
	  //wcout << "was auxg " << w << endl;
	  return L"AUXG";
	}
    }
  if(trm == L"BES" || trm == L"HVS")  //??? strange tags in switchboard
    {
      assert(w == L"'S" || w == L"-S");
      return L"AUX";
    }
  return trmM;
}
