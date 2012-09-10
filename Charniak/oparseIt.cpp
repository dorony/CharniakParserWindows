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
#include <fstream>
#include <iostream>
//#include "unistd.h"
#include <math.h>
#include "GotIter.h"
#include "Wrd.h"
#include "InputTree.h"
#include "Bchart.h"
#include "ECArgs.h"
#include "MeChart.h"
#include "extraMain.h"
#include "AnsHeap.h"
#include "UnitRules.h"
#include "Params.h"
#include "TimeIt.h"
#include "ewDciTokStrm.h"
#include "Link.h"
#include "utils.h"


int sentenceCount; // allow extern'ing for error messages
int printCount =0;
ewDciTokStrm* tokStream = NULL;
istream* nontokStream = NULL;
Params params;

static void printSkipped( SentRep *srp, MeChart *chart=NULL );

/* In order to print out the data in the correct order each
thread has it's own PrintStack which stores the output data
(printStrict) until it is time to print it out in order.
*/
typedef struct printStruct{
  int                sentenceCount;
  int                numDiff;
  vector<InputTree*> trees;
  vector<double>     probs;
  ECString             name;
} printStruct;

typedef list<printStruct> PrintStack;



void*
mainLoop(void* arg)
{
  double log600 = log2(600.0);
  int id = (int)arg;
  PrintStack printStack;
  for( sentenceCount = 0;  ; sentenceCount++)
    {
      SentRep* srp = new SentRep(params.maxSentLen);

     
      LARGE_INTEGER frequency, start;
      QueryPerformanceFrequency(&frequency);
      QueryPerformanceCounter(&start);
    
      if(Bchart::tokenize)
	*tokStream >> *srp;
      else 
	*nontokStream >> *srp;
      int locCount = sentenceCount;      

      int len = srp->length();
      if (len == 0) break;
      if (len > params.maxSentLen) 
	{
	  ECString msg(L"skipping sentence longer than specified limit of ");
	  msg += intToString(params.maxSentLen);
	  WARN( msg.c_str() );
	  printSkipped(srp);
	  continue;
	}
 
      // handle input containing reserved word Bchart::HEADWORD_S1; could probably do 
      // better (like undo replacement before printing) but this seems sufficient.
      int i;
      for (i = 0; i < len; ++i) 
	{
	  ECString& w = ((*srp)[i]).lexeme();
	  if (w == Bchart::HEADWORD_S1) 
	    {
	      ECString msg = ECString(L"Replacing reserved token \"") + Bchart::HEADWORD_S1;
	      msg += L"\" at index " + intToString(i) + L" of input with token \"^^^\"";
	      WARN( msg.c_str() );
	      w = L"^^^";
	    }
	}
      
      if( !params.field().in(sentenceCount) ) continue;
      MeChart*	chart = new MeChart( *srp,id ); 
      chart->parse( );
      
      Item* topS = chart->topS();
      if(!topS)
	{
	  WARN( L"Parse failed: !topS" );
	  printSkipped(srp,chart);
	  delete chart;
	  continue;
	}
      // compute the outside probabilities on the items so that we can
      // skip doing detailed computations on the really bad ones 
      chart->set_Alphas();
      
      Bst& bst = chart->findMapParse();
     if( bst.empty())
	{
	  WARN( L"Parse failed: chart->findMapParse().empty()" );
	  printSkipped(srp,chart);
	  delete chart;
	  continue;
	}
      if(Feature::isLM)
	{
	  double lgram = log2(bst.sum());
	  lgram -= (len*log600);
	  double pgram = pow(2,lgram);
	  double iptri =chart->triGram();;
	  double ltri = (log2(iptri)-len*log600);
	  double ptri = pow(2.0,ltri);
	  double pcomb = (0.667 * pgram)+(0.333 * ptri);
	  double lmix = log2(pcomb);
	  if(locCount%10==9)wcout << lgram << "\t" << ltri << "\t" << lmix << "\n";
	}
      int numVersions = 0;
      printStruct printS;
      printS.numDiff = 0;
      printS.name = srp->getName();
      printS.sentenceCount = locCount;
      Link diffs(0);
      //wcerr << "Need num diff: " << Bchart::Nth << endl;
      for(numVersions = 0 ; ; numVersions++)
	{
	  short pos = 0;
	  Val* v = bst.next(numVersions);
	  if(!v) break;
	  double vp = v->prob();
	  if(vp == 0) break;
	  if(isnan(vp)) break;
	  if(isinf(vp)) break;
	  InputTree* mapparse=inputTreeFromBsts(v,pos,*srp);
	  bool isU;
	  int cnt = 0;
	  diffs.is_unique(mapparse, isU,cnt);
	  if(cnt != len)
	    {
	      wcerr << "Bad length parse for: " << *srp << endl;
	      wcerr << *mapparse << endl;
	      assert(cnt == len);
	    }
	  if(isU)
	    {
	      printS.probs.push_back(v->prob());
	      printS.trees.push_back(mapparse);
	      printS.numDiff++;
	    }
	  else
	    {
	      delete mapparse;
	    }
	  if(printS.numDiff >= Bchart::Nth) break;
	  if(numVersions > 20000) break;
	}
      if( printS.numDiff == 0)
	{
	  WARN( L"Parse failed from 0, inf or nan probabililty" );
	  printSkipped(srp,chart);
	  delete chart;
	  continue;
	}
   
      printTime(start, frequency,"end");
      int numPrinted;

      /* put the sentence with which we just finished at the end of the printStack*/
      printStack.push_back(printS);
      PrintStack::iterator psi = printStack.begin();
      /* now look at each item from the front of the print stack
	 to see if it should be printed now */

      for( numPrinted =0; psi != printStack.end(); numPrinted++ )
	{
	  printStruct& pstr=(*psi);
	  if(pstr.sentenceCount != printCount) break;
	  if(Bchart::Nth > 1) {
	    ECString index = pstr.name.empty() ? intToString(sentenceCount)
	      : pstr.name;
	    wcout << pstr.numDiff << "\t" << index <<"\n";
	  }
	  printCount++;
	  for(i = 0 ; i < pstr.numDiff ; i++)
	    {
	      InputTree*  mapparse = pstr.trees[i];
	      assert(mapparse);
	      double logP =log2(pstr.probs[i]);
	      logP -= (srp->length()*log600);
	      if (Bchart::Nth > 1) 
		wcout << logP << "\n";
	      else if (!srp->getName().empty())	
		wcout << "<" << srp->getName() << "> "; 
	      
	      if (Bchart::prettyPrint) 
		wcout << *mapparse << "\n\n";
	      else
		{
		  mapparse->printproper(wcout);
		  wcout << "\n";
		}
	      delete mapparse;

	    }
	  psi++;
	}
      wcout << endl;
      for(i = 0 ; i < numPrinted ; i++) printStack.pop_front();
      delete chart;
      delete srp;
    }
    return 0;
}



double diffclock(clock_t clock1,clock_t clock2)
{
	double diffticks=clock1-clock2;
	double diffms=(diffticks*10)/CLOCKS_PER_SEC;
	return diffms;
}


int wmain(int argc, Char *argv[])
{
   ECArgs args( argc, argv );
   /* l = length of sentence to be proceeds 0-100 is default
      n = work on each #'th line.
      d = print out debugging info at level #
      t = report timings */

   params.init( args );
   TimeIt timeIt;
   ECString  path( args.arg( 0 ) );
   generalInit(path);

   ECString flnm = L"dummy";
   if(args.nargs()==2) flnm = args.arg(1);
   if(Bchart::tokenize)
     {
       tokStream = new ewDciTokStrm(flnm);
       if(args.nargs() ==1) tokStream->useCin = 1;
     }
   if(args.nargs()==2) nontokStream = new ifstream(args.arg(1).c_str());
   else nontokStream = &cin;
   mainLoop(0);
   return 0;
}

//------------------------------

static const ECString& getPOS(Wrd& w, MeChart *chart)
{
  list<float>& wpl = chart->wordPlist(&w, w.loc());      
  list<float>::iterator wpli = wpl.begin();
  float max=-1.0;
  int termInt = (int)max;
  for( ; wpli != wpl.end() ; wpli++)
    {
      int term = (int)(*wpli);
      wpli++;
      // p*(pos|w) = argmax(pos){ p(w|pos) * p(pos) } 
      double prob = *wpli * chart->pT(term); 
      if (prob > max) {
	termInt = term;
	max = prob;
      }
    }
  const Term* nxtTerm = Term::fromInt(termInt);
  return nxtTerm->name();
}

//------------------------------

static void printFlat(SentRep *srp, MeChart *chart) 
{
  bool allocated=false;
  if (chart == NULL && srp->length() < MAXSENTLEN) 
    {
      chart = new MeChart( *srp,0);
      allocated=true;
    }

  // 05/30/06 ML: use something short for pretend POS tag
  const ECString UNK=L"?"; 

  if (!srp->getName().empty())
    wcout << "<" << srp->getName() << "> "; 

  wcout << "(S1 (NOPARSE";
  for (int xx = 0; xx < srp->length(); ++xx)
    {
      Wrd& w = (*srp)[xx];
      const ECString& pos = (chart!=NULL) ? getPOS(w,chart) : UNK;
      wcout << " (" << pos << " " << w << ")";
    }
  wcout << "))\n" << std::endl;
  printCount++;

  if (allocated)
    delete chart;
}

//------------------------------

static void
printSkipped(SentRep *srp, MeChart *chart)
{
  // stderr
  if (!Bchart::silent) 
      wcerr << *srp << "\n\n";

  // stdout
  // ML 05/04/06: Ensure every input sentence produces an output parse tree,
  // at least in 1-best mode. The default tree is just a flat S.
  if(Feature::isLM)
    {
      double veryLow=-1000;
      wcout << veryLow << "\t" << veryLow << "\t" << veryLow << "\n";
    }
  if (Feature::isLM || Bchart::Nth==1) {
    printFlat(srp,chart);  
  }
}
