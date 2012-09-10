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
#include "GotIter.h"

GotIter::
GotIter(Edge* edge) : whereIam( edge )
{}


bool
GotIter::
next(Item*& itm)
{
  if(!whereIam || !whereIam->item()) return false;
  else
    {
      itm = whereIam->item();
      whereIam = whereIam->pred();
      return true;
    }
}

LeftRightGotIter::
LeftRightGotIter(Edge* edge)
{
  makelrgi(edge);
}

void
LeftRightGotIter::
makelrgi(Edge* ri)
{
  GotIter gi(ri);
  Item* itm;
  bool finishedRight = false;
  int spos = ri->start();
  /* gotiter return a b head c d in the order d c a b head */
  list<Item*>::iterator lri;
  list<Item*> lrlist;
  while(gi.next(itm))
    {
      //wcerr << "lrgi " << *itm << endl;
      if(finishedRight || itm->start() == spos)
	{
	  // if 1st consits is STOP(3, 3) then next can have same start pos.
	  if(itm->start() == spos && !finishedRight)
	    {
	      finishedRight = true;
	      lri = lrlist.begin();
	    }
	  lri = lrlist.insert(lri, itm);
	  lri++;
	}
      else lrlist.push_front(itm);
    }
  lri = lrlist.begin();
  int i = 0;
  for( ; lri != lrlist.end() ; lri++)
    {
      assert(i < 400);
      lrarray[i] = (*lri);
      i++;
    }
  size_ = i;
  pos_ = 0;
}


bool
LeftRightGotIter::
next(Item*& itm)
{
  if(pos_ >= size_) return false;
  assert(pos_ < 400);
  itm = lrarray[pos_];
  pos_++;
  return true;
}

bool
SuccessorIter::
next(Edge*& edge)
{
  if(edgeIter == edge_->sucs_.end()) return false;
  edge = *edgeIter;
  //wcerr << "Si: " << *edge_ << " has " << *edge << endl; 
  edgeIter++;
  return true;
}
    
NeedmeIter::
NeedmeIter(Item* itm)
{
  //stack.reserve(64000);
  stack.assign(itm->needme().begin(),itm->needme().end());
}

bool
NeedmeIter::
next(Edge*& e)
{
  if (stack.size() == 0) return false;

  e = stack.back();
  stack.pop_back();
  stack.insert(stack.end(),e->sucs().begin(),e->sucs().end());
  return true;
}
	
        
bool
MiddleOutGotIter::
next(Item*& itm, int& dir)
{
  //if(pos_ > 20) wcerr << "MOGI pos " << pos_ << " " << size_ << " " << *firstRight_<<endl;
  if(pos_ < 0) return false;
  assert(pos_ < 256);
  itm = lrarray[pos_];
  //if(pos_ > 20) wcerr << "MOGI itm " << *itm << endl;
  dir = dir_;
  if(pos_ == size_-1)
    {
      dir = 0;
      dir_ = 1;
    }
  if(itm == firstRight_)
    {
      dir = 2;
      dir_ = 2;
    }
  pos_--;
  //if(pos_ > 20) wcerr << "AA" << endl;
  return true;
}
  
MiddleOutGotIter::
MiddleOutGotIter(Edge* e)
{
  GotIter gi(e);
  Item *itm;
  bool startRight = false;
  int spos = e->start();
  /* gotiter return a b head c d in the order d c a b head */
  int i = 0;
  while(gi.next(itm))
    {
      assert(i < 256);
      lrarray[i] = itm;
      //wcerr << "lrgi " << *itm << endl;
      if(itm->start() == spos && !startRight)
        {
          startRight = true;
          assert(i > 0);
          firstRight_ = lrarray[i-1];
        }
      i++;
    }
  size_ = i;
  pos_ = i-1;
  //if(i > 20) wcerr << "MOGII " << size_ << " " << *firstRight_ << endl;
}

