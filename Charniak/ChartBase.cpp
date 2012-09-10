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
#include "ChartBase.h"
#include <math.h>
#include "GotIter.h"
#include "InputTree.h"

const double ChartBase::badParse = -1.0L;
int ChartBase::ruleiCountTimeout_ = 360000;
int ChartBase::poppedTimeout_ = 50000;
float ChartBase::endFactor = 1.2;
float ChartBase::midFactor = 0.88334;
int      ChartBase::numItemsToDelete[MAXNUMTHREADS] = {0,0,0,0};
vector<Item*>    ChartBase::itemsToDelete[MAXNUMTHREADS];
int      ChartBase::itemsToDeletesize[MAXNUMTHREADS] = {0,0,0,0};
bool     ChartBase::guided = false;

bool
ChartBase::
finalPunc(const Char* wrd)
{
  ECString wd(wrd);
  ECStringsIter ei = Term::Colons.begin();
  for( ; ei!= Term::Colons.end() ; ei++) if(wrd == *ei) return true;
  ei = Term::Finals.begin();
  for( ; ei!= Term::Finals.end() ; ei++) if(wrd == *ei) return true;
  return false;
}

float
ChartBase::
endFactorComp(Edge* dnrl)
{
  int start = dnrl->start();
  int finish = dnrl->loc();
  int effVal = effEnd(finish);
  ECString trmNm(dnrl->lhs()->name());
  const Term* trm = Term::get(trmNm);
  if((trm->isRoot() || trm->isS()) && finish == wrd_count_
     && start == 0)
    return endFactor;
  else if(effVal==1)
    return endFactor;
  else if(effVal == 0) return midFactor;
  else return .95;  //if effVal == 2, currently not used;
}

int
ChartBase::
effEnd(int pos)
{
  bool ans;
  if(pos > endPos) return 0;
  if(pos == endPos) return 1;  //in case no final punc;
  const Char* wrd = sentence_[pos].lexeme().c_str();
  if(finalPunc(wrd)) ans = 1;
  else if(pos > wrd_count_ -3) ans = 0;
  else if(!wcscmp(wrd,L","))
    {
      if(sentence_[pos+1].lexeme() == L"''")
	ans = 1; // ,'' acts like end of sentence;
      else ans = 0;  //ans = 2 for alt version???
    }
  else ans = 0;
  return ans;
}

Item*
ChartBase::
addtochart(const Term* trm)
{
  if(numItemsToDelete[thrdid] >= itemsToDeletesize[thrdid])
    {
      Item* dummy = new Item(trm, 0, 0);
      itemsToDelete[thrdid].push_back(dummy);
      itemsToDeletesize[thrdid]++;
    }
  Item* ans = itemsToDelete[thrdid][numItemsToDelete[thrdid]++];
  ans->set(trm,0);
  return ans;
}

ChartBase::
ChartBase(SentRep & sentence,int id)
: 
  thrdid(id),
  sentence_( sentence ),
  crossEntropy_(0.0L), 
  wrd_count_(0),
  popedEdgeCount_(0),
  ruleiCounts_(0)
{
#ifdef DEBUG
    extern int	rulei_high_water;
    rulei_high_water = 0;
#endif /* DEBUG */
    numItemsToDelete[id] = 0;
    wrd_count_ = sentence.length();
    regs = new Items*[wrd_count_+1];
    for (int i = 0; i <= wrd_count_; i++)
    {
        regs[i] = new Items[wrd_count_+1];
    }
    endPos = wrd_count_;
    const Char* endwrd = NULL;
    if(wrd_count_ > 0) endwrd = sentence_[wrd_count_-1].lexeme().c_str();
    if(endwrd  && finalPunc(endwrd)) endPos = wrd_count_-1;
    else if(wrd_count_ > 2)
      {
	endwrd = sentence[wrd_count_-2].lexeme().c_str();
	if(finalPunc(endwrd)) endPos = wrd_count_-2;
	else
	  {
	    endwrd = sentence[wrd_count_-3].lexeme().c_str();
	    if(finalPunc(endwrd)) endPos = wrd_count_-3;
	  }
      }
}

// virtual
ChartBase::
~ChartBase()
{
  register int    i, j;

  for (i = 0; i <= wrd_count_; i++)
    {
      while(!waitingEdges[0][i].empty()) waitingEdges[0][i].pop_front();
      while(!waitingEdges[1][i].empty()) waitingEdges[1][i].pop_front();
      for (j = 0; j < wrd_count_; j++)
	{
	  free_chart_items(regs[i][j]);
	}
      delete[] regs[i];
    }
}

void
ChartBase::
free_edges(list<Edge*>& edges)
{
  list<Edge*>::iterator lei = edges.begin();
  for( ; lei != edges.end() ; lei++)
    delete (*lei);
}

void
ChartBase::
set_Alphas()
{
  Item           *snode = get_S();
  double         tempAlpha[400]; //400 has no particular meaning, just large enough.
  
  if( !snode || snode->prob() == 0.0 )
    {
      WARN( L"estimating the counts on a zero-probability sentence" );
      return;
    }
  double sAlpha = 1.0/snode->prob();
  snode->poutside() = sAlpha;
  
  /* for each position in the 2D chart, starting at top*/
  /* look at every bucket of length j */
  for (int j = wrd_count_-1 ; j >= 0 ; j--)
    {
      for (int i = 0 ; i <= wrd_count_ - j ; i++)
	{
	  Items il = regs[j][i];
	  list<Item*>::iterator ili = il.begin();
	  Item* itm;
	  for(; ili != il.end(); ili++ )
	    {
	      itm = *ili;
	      if(itm != snode) itm->poutside() = 0; //init outside probs to 0;
	    }
	  
	  bool valuesChanging = true;
	  /* do alpha calulcations until values settle down */
	  ili = il.begin();
	  while(valuesChanging)
	    {
	      valuesChanging = false;
	      int            tempPos = 0;  //position in tempAlpha;
	      ili = il.begin();
	      for(; ili != il.end(); ili++ )
		{
		  itm = *ili;
		  if(itm == snode) continue;
		  double itmalpha = 0;

		  NeedmeIter nmi(itm);
		  Edge* e;
		  while( nmi.next(e) )
		    {
		      const Item* lhsItem = e->finishedParent();
		      if(lhsItem) itmalpha += lhsItem->poutside()
			                        * e->prob();
		    }
		  assert(tempPos < 400);
		  double val = itmalpha/itm->prob();
		  tempAlpha[tempPos++] = val;
		} 
	      /* at this point the new alpha values are stored in tempAlpha */
	      int temppos = 0;
	      ili = il.begin();
	      for(; ili != il.end(); ili++ )
		{
		  itm = *ili;
		  if(itm == snode) continue;
		  /* the start symbol for the entire sentence has poutside =1*/
		  if(i == 0 && j ==wrd_count_-1 &&
		     itm->term()->isRoot())
		    itm->poutside() = sAlpha;
		  else
		    {
		      double oOutside = itm->poutside();
		      double nOutside = tempAlpha[temppos];
		      if(nOutside == 0)
			{
			  if(oOutside != 0) error(L"Alpha went down");
			}
		      else if(oOutside/nOutside < .95)
			{
			  itm->poutside() = nOutside;
			  valuesChanging = true;
			  //wcerr << "alpha*beta " << *itm << " = "
			  //<< (itm->poutside() * itm->prob()) << endl;
			}
		    }
		  temppos++;
		}
	      if(temppos != tempPos)
		{
		  wcerr << "temppos = " << temppos << " and tempPos = "
		    << tempPos << " ";
		  error(L"Funnly situation in setAlphas");
		}
	    }
	}
    }
}

void
ChartBase::
free_chart_items(Items& itms)
{
    Item          *temp;

    while( !itms.empty() )
      {
	temp = itms.front();
	//temp->check();
	itms.pop_front();
	
	//if(!temp->term()->terminal_p()) delete temp;
      }    
}


Item *
ChartBase::
get_S() const
{
  const Term *    sterm = Term::rootTerm;
  Item           *itm;

  Items il = regs[wrd_count_ - 1][0];
  Items::iterator ili = il.begin();
  for(; ili != il.end(); ili++ )
    {
      itm = *ili;
      if( itm->term() == sterm )
	return itm;
    }
  return NULL;
}


void
ChartBase::
setGuide(InputTree* tree)
{
  if(!tree) return;
  int trm = Term::get(tree->term())->toInt();
  guide[tree->start()][tree->finish()].push_back(trm);
  InputTreesIter iti = tree->subTrees().begin();
  for( ; iti!= tree->subTrees().end() ; iti++)
    setGuide(*iti);
}

bool
ChartBase::
inGuide(int st, int ed, int trm)
{
  vector<short>& vs = guide[st][ed];
  vector<short>::iterator vsi = vs.begin();
  for(;vsi != vs.end();vsi++)if((*vsi) == trm) return true;
  return false;
}
   
bool
ChartBase::
inGuide(Edge* e)
{
  if(!inGuide(e->start(),e->loc(),e->lhs()->toInt())) return false;
  LeftRightGotIter g1(e);
  Item* itm;
  if(g1.size() == 1) return true;
  //wcerr << "IG " << *e << endl;
  while(g1.next(itm))
    {
      int i;

      Item* itm2;
      int st =itm->start();
      //if(itm->term() == Term::stopTerm) continue;
      //wcerr << "POS " << g1.pos() << *(g1.index(g1.pos())) << endl;
      for(i = g1.pos()+1 ; i < g1.size() ; i++)
	{
	  itm2 = g1.index(i);
	  //wcerr << "II " << *itm << " " << *itm2 << endl;
	  if(itm2->term()->terminal_p())
	    {
	      if(guide[itm2->start()][itm2->finish()].size() >1) return false;
	    }
	  int fn = itm2->finish();
	  if(st == e->start() && fn == e->loc()) continue;
	  if(!guide[st][fn].empty()) return false;
	}
    }
  return true;
}
