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
#include "InputTree.h"
#include "headFinder.h"
#include "utils.h"
#include <assert.h>
#include <set>
#include "Term.h"

  
int              InputTree::pageWidth = 75;  //used for prettyPrinting

ECString         InputTree::tempword[400];          
int              InputTree::tempwordnum= 0;

InputTree::
InputTree(InputTree* it) :
  start_(it->start_), finish_(it->finish_), word_(it->word_),
    term_(it->term_), ntInfo_(it->ntInfo_), parent_(it->parent_),
     headTree_(headTree_)
{
  InputTreesIter subti = it->subTrees().begin();
  for( ; subti != it->subTrees().end() ; subti++)
    subTrees_.push_back(*subti);
}
     
void
InputTree::
init()
{
  for(int i = 0 ; i < 128 ; i++)
    {
      tempword[i] = L"";
    }
  tempwordnum = 0;
}

InputTree::
~InputTree()
{
  InputTree  *subTree;
  InputTreesIter  subTreeIter = subTrees_.begin();
  for( ; subTreeIter != subTrees_.end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      delete subTree;
    }
}

InputTree::
InputTree(istream& is)
{
  readParse(is);
}

istream&
operator >>( istream& is, InputTree& parse )
{
  if(parse.word() != L"" || parse.term() != L""
     || parse.subTrees().size() > 0)
    error(L"Reading into non-empty parse." );
  parse.readParse(is);
  return is;
}


void
InputTree::
readParse(istream& is)
{
  int pos = 0;
  start_ = pos;
  finish_ = pos;
  num_ = -1;

  ECString temp = readNext(is);
  if(!is) return;
  if(temp != L"(")
    {
      wcerr << "Saw " << temp << endl;
      error(L"Should have seen an open paren here.");
    }
  /* get annotated symbols like NP-OBJ.  term_ = NP ntInfo_ = OBJ */
  temp = readNext(is);
  term_ = L"S1";
  if(temp != L"(")
    {
      if(temp == L"S1" || temp == L"TOP" || temp == L"ROOT")
	{
	  temp = readNext(is);
	}
      else error(L"did not see legal topmost type");
    }
  if(temp == L")") return;
  if(temp != L"(")
    {
      wcerr << "Saw " << temp << endl;
      error(L"Should have seen second open paren here.");
    }

  for (;;)
    {
      InputTree* nextTree = newParse(is, pos, this);
      subTrees_.push_back(nextTree);
      finish_ = pos;

      headTree_ = nextTree->headTree_;
      temp = readNext(is);
      if (temp==L")") break;
      if (temp!=L"(")
	{
	  wcerr << *this << endl;
	  error(L"Should have open or closed paren here.");
	}
    }
}

InputTree*
InputTree::
newParse(istream& is, int& strt, InputTree* par)
{
  int strt1 = strt;
  ECString wrd;
  ECString trm;
  ECString ntInf;
  InputTrees subTrs;
  int num = -1;

  parseTerm(is, trm, ntInf,num);
  for( ; ; )
    {
      ECString temp = readNext(is);
      if(temp == L"(")
	{
	  InputTree* nextTree = newParse(is, strt, NULL);
	  if(nextTree) subTrs.push_back(nextTree);
	}
      else if(temp == L")") break;
      else
	{
	  if(trm != L"-NONE-")
	    {
	      wrd = temp;
	      strt++;
	    }
	}
    }

  /* the Chinese treebank has a single pos for all punctuation,
     which is pretty bad for the parser, so make each punc its own pos */
  /* fixes bugs in Chinese Treebank */
  if(Term::Language == L"Ch")
    {
      if(trm == L"PU") trm = wrd;
      const Term* ctrm = Term::get(trm);
      if(!ctrm)
	{
	  wcerr << "No such term " << trm << endl;
	  assert(ctrm);
	}
      if(wrd!=L"" && !(ctrm->terminal_p()))
	{
	  wcout<<trm<<wrd<<" changed to NN"<<endl;
	  trm=L"NN";
	}
      if(wrd==L"" && ctrm->terminal_p() )
	{
	  wcout<<trm<<" changed to NP"<<endl;
	  trm=L"NP";
	}
    }

  InputTree* ans = new InputTree(strt1, strt, wrd, trm, ntInf, subTrs,
				 par, NULL);
  ans->num() = num;
  InputTreesIter iti = subTrs.begin();
  for(; iti != subTrs.end() ; iti++)
    {
      InputTree* st = *iti;
      st->parentSet() = ans;
    }
  
  if(wrd == L"" && subTrs.size() == 0) return NULL;
  if(wrd != L"")
    {
      ans->headTree() = ans;
    }
  else
    {
      int hpos = headPosFromTree(ans);
      ans->headTree() = ithInputTree(hpos, subTrs)->headTree();
    }
  //wcerr << "ANS " << ans->start() << " " << *ans << endl;
  return ans;
}

ECString&
InputTree::
readNext( istream& is ) 
{
  // if we already have a word, use it, and increment pointer to next word;
  //wcerr << "RN1 " << tempwordnum << " " << tempword[tempwordnum] << endl;
  if( tempword[tempwordnum] != L"" )
    {
      return tempword[tempwordnum++];
    }
  //wcerr << "RN2" << endl;
  // else zero out point and stuff in 
  int tempnum;
  for(tempnum = 0 ; tempword[tempnum] != L"" ; tempnum++ )
    tempword[tempnum] = L"";
  tempwordnum = 0;
  // then go though next input, separating "()[]";
  int    wordnum  = 0 ;
  ECString  temp;
  is >> temp;
  //wcerr << "In readnext " << temp << endl;
  for( tempnum = 0 ; tempnum < temp.length() ; tempnum++ )
    {
      Char tempC = temp[tempnum];
      if(tempC == '(' || tempC == ')' ||
	 tempC == '[' || tempC == ']' )
	{
	  if( tempword[wordnum] != L"" )
	    wordnum++;
	  tempword[wordnum++] += tempC;
	}
      else tempword[wordnum] += temp[tempnum];
    }
  return tempword[tempwordnum++];
}

/* if we see NP-OBJ make NP a, and -OBJ b */
void
InputTree::
parseTerm(istream& is, ECString& a, ECString& b, int& num)
{
  ECString temp = readNext(is);
  if(temp == L"(" || temp == L")") error(L"Saw paren rather than term");
  int len = temp.length();
  size_t pos;
  pos = temp.find(L"^");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos+1, len-pos-1);
      a = na;
      // wcerr <<"NB " << na << " " << nb << endl;
      num = atoi(nb.c_str());
    }
  else a = temp;
  pos = a.find(L"-");
  /* things like -RCB- will have a - at position 0 */
  if(pos < len && pos > 0)
    {
      ECString na(a, 0, pos);
      ECString nb(a, pos, len-pos);
      a = na;
      len = pos;
      b = nb;
    }
  else
    {
      b = L"";
    }
  pos = a.find(L"=");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos, len-pos);
      a = na;
      len = pos;
      b += nb;
    }
  pos = a.find(L"|");
  if(pos < len && pos > 0)
    {
      ECString na(temp, 0, pos);
      ECString nb(temp, pos, len-pos);
      a = na;
      b += nb;
    }
}	   
	   
ostream&
operator <<( ostream& os, const InputTree& parse )
{
  parse.prettyPrint( os, 0, false );
  return os;
}

void 
InputTree::
printproper( ostream& os ) const
{
  if( word_.length() != 0 )
    {
      os << "(" << term_ << " " << word_ << ")";
    }
  else
    {
      os << "(";
      os <<  term_ << ntInfo_;
      ConstInputTreesIter  subTreeIter= subTrees_.begin();
      InputTree  *subTree;
      for( ; subTreeIter != subTrees_.end() ; subTreeIter++ )
	{
	  subTree = *subTreeIter;
	  os << " ";
	  subTree->printproper( os );
	}
      os << ")";
    }
}

void 
InputTree::
prettyPrint(ostream& os, int start, bool startingLine) const              
{
  if(start >= pageWidth) //if we indent to much, just give up and print it.
    {
      printproper(os);
      return;
    }
  if(startingLine)
    {
      os << "\n";
      int numtabs = start/8;
      int numspace = start%8;
      int i;
      for( i = 0 ; i < numtabs ; i++ ) os << "\t"; //indent;
      for( i = 0 ; i < numspace ; i++ ) os << " "; //indent;
    }
  /* if there is enough space to print the rest of the tree, do so */
  if(spaceNeeded() <= pageWidth-start || word_ != L"")
    {
      printproper(os);
    }
  else
    {
      os << "(";
      os << term_ << ntInfo_;
      os << " ";
      /* we need 2 extra spaces, for "(", " "  */
      int newStart = start + 2 + term_.length() + ntInfo_.length();
      //move start to right of space after term_ for next thing to print
      start++; //but for rest just move one space in.
      ConstInputTreesIter  subTreeIter = subTrees_.begin();
      InputTree  *subTree;
      int whichSubTree = 0;
      for( ; subTreeIter != subTrees_.end() ; subTreeIter++ )
	{
	  subTree = *subTreeIter;
	  if(whichSubTree++ == 0)
	    {
	      subTree->prettyPrint(os, newStart, false);
	    }
	  else
	    {
	      subTree->prettyPrint(os, start, true);
	    }
	}
      os << ")";
    }
}

/* estimates how much space we need to print the rest of the currently
   print tree */
int
InputTree::
spaceNeeded() const
{
  int needed = 1; // 1 for blank;    
  int wordLen = word_.length();
  needed += wordLen;
  needed += 3; //3 for () and " ";
  needed += term_.length();
  needed += ntInfo_.length();
  if(word_ != L"") return needed;
  ConstInputTreesIter  subTreeIter = subTrees_.begin();
  InputTree  *subTree;
  for( ; subTreeIter != subTrees_.end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      needed += subTree->spaceNeeded();
    }
  return needed;
}

void
InputTree::
make(list<ECString>& words)
{
  if(word_ != L"")
    {
      words.push_back(word_);
    }
  else
    {
      ConstInputTreesIter subTreeIter = subTrees().begin();
      InputTree  *subTree;
      for(; subTreeIter != subTrees().end() ; subTreeIter++)
	{
	  subTree = *subTreeIter;
	  subTree->make(words);
	}
    }
}

void
InputTree::
makePosList(vector<ECString>& words)
{
  if(word_ != L"")
    {
      words.push_back(term_);
    }
  else
    {
      ConstInputTreesIter subTreeIter = subTrees().begin();
      InputTree  *subTree;
      for(; subTreeIter != subTrees().end() ; subTreeIter++)
	{
	  subTree = *subTreeIter;
	  subTree->makePosList(words);
	}
    }
}



InputTree*
ithInputTree(int i, const list<InputTree*> l)
{
  list<InputTree*>::const_iterator li = l.begin();
  for(int j = 0 ; j < i ; j++)
    {
      assert(li != l.end());
      li++;
    }
  return *li;
}
