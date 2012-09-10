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


#ifndef PARAMS_H
#define PARAMS_H
      
#include "Field.h"
#include "ECArgs.h"

class Params 
{
public:

  static const int DEFAULT_SENT_LEN = 100;

    Params(): 	    
      file(0),
      maxSentLen(DEFAULT_SENT_LEN),
      stdInput_(false),
      outputData_(false),
      fileString_(),
      numString_(),
      whichSent_( 0 ),
      ofTotal_( 1 ),
      field_( 0 )
	{}

    void	    init( ECArgs& args );
    const Char *    file;
    const ECString&   fileString()
		    {  return fileString_;  }
    const ECString&   numString()
		    {   return numString_;  }
    const int	    whichSent()
		    {   return whichSent_;   }
    const int	    ofTotal()
		    {   return ofTotal_;   }
    const Field&    field() const
		    {   return *field_;   }
    bool&      stdInput() { return stdInput_; }
    bool&      outputData() { return outputData_; }
    int        maxSentLen;
private:
    bool       stdInput_;
    bool       outputData_;
    ECString	    fileString_;
    ECString          numString_;
    int             whichSent_;
    int             ofTotal_;
    Field *         field_;
};

#endif /* ! PARAMS_H */
