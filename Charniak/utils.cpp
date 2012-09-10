/*
 * Copyright 1997, Brown University, Providence, RI.
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
#include "utils.h"
#include <iostream>
#include <assert.h>
#include <stdlib.h> // abort(), exit(), rand()
#include <algorithm>

extern int sentenceCount; // from parseIt.C

void 
warn( const Char *filename, int filelinenum, const Char *msg )
{
  wcerr <<  "Warning [" << filename << ":" << filelinenum << "]";
  wcerr << " Sentence " << sentenceCount << ": " << msg << endl;
}

void 
error( const Char *filename, int filelinenum, const Char *msg )
{
  wcerr <<  "Warning [" << filename << ":" << filelinenum << "]";
  wcerr << " Sentence " << sentenceCount << ": " << msg << endl;
  abort();
  exit( 1 );
}

Char*
toLower(const Char* str, Char* temp)
{
  int l = wcslen(str);
  assert(l < 512);
  for(int i = 0 ; i <= l ; i++)
    {
      Char n = str[i];
      int ni = (int)n;
      if(ni >= 65 && ni <= 90)
	{
	  temp[i] = (Char)(ni+32);
	}
      else temp[i] = n;
    }
  return temp;
}


ECString
intToString(int i)
{
  Char temp[16];
  sprintf(temp, L"%i", i); 
  ECString ans(temp);
  return ans;
}

bool 
vECfind(const ECString& s, ECStrings& sts)
{
  return ( find(sts.begin(),sts.end(),s) != sts.end() );
}

void printTime(LARGE_INTEGER start, LARGE_INTEGER frequency, std::string message)
{
       LARGE_INTEGER end;
     QueryPerformanceCounter(&end);
     double interval = static_cast<double>(end.QuadPart - start.QuadPart) * 1000 / frequency.QuadPart ;
     
     wcout << interval <<"\n";
  //  wcout << message << ":"<< interval <<"\n";
}

/*
UNUSED ---------------------------------------

double 
ran()
{
    return (rand() * 4.656612875245796E-10);
}

ECString lastCharacter(const ECString& s)
{
        ECString f;
        int len=s.length();
        assert(s!="");
        int c=(int)s[len-1];
        if (c<0||c>127) {
                assert(len>=2);
                f=f+s[len-2]+s[len-1];
        }else f=f+s[len-1];
        return f;
}
*/
#if 0 // can't comment out code below since it uses '*/'

void
ignoreComment(ifstream& inpt)
{
  ECString nxt;
  char a;
  inpt.get(a);
  if(a == '/')
    {
      char b = inpt.peek();
      if(b == '*')
	{
	  while(inpt)
	    {
	      inpt >> nxt;
	      if(nxt == "*/") break;
	    }
	  return;
	}
    }
  inpt.putback(a);
  return;
}
	  
--------------------------------------- 
#endif // 0
