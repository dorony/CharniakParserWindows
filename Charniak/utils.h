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

#ifndef UTILS_H 
#define UTILS_H

#include "ECString.h"
#include <vector>

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#define WARN( msg ) warn( __WFILE__, __LINE__, msg )
#define ERROR( msg ) error( __WFILE__, __LINE__, msg )

void warn( const Char *filename, int filelinenum, const Char *msg );
void error( const Char *filename, int filelinenum, const Char *msg );
static void error(const Char *s) { ERROR( s ); } // backwards compatibility

Char* toLower(const Char* str, Char* temp);
ECString intToString(int i);

typedef vector<ECString> ECStrings;
typedef ECStrings::iterator ECStringsIter;
bool vECfind(const ECString& st, ECStrings& sts);

void printTime(LARGE_INTEGER start, LARGE_INTEGER frequency, std::string message);

/* UNUSED
double ran();
enum DGStatus  { OK, NOT_OK };
ECString lastCharacter(const ECString& s);
void ignoreComment(ifstream& input);
*/

/* UNIMPLMENTED
ECString firstCharacter(const ECString& s);
*/
#endif /* ! UTILS_H */

