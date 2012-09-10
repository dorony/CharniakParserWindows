#include "stdafx.h"
#include "ScoreTree.h"
#include "InputTree.h"
#include "Term.h"

bool
ScoreTree::
lexact2(InputTree* tree)
{
  int strt = equivInt(tree->start());
  int fnsh = equivInt(tree->finish());
  ECString trm = tree->term();
  if(trm == L"PRT") trm = L"ADVP";
  int indx = Term::get(trm)->toInt();
  indx = indx*100 + strt;
  indx = indx*500 + fnsh;
  //wcerr << "LE20 " << indx << " " << trm << " " << start_ << " " << strt << " " << fnsh << endl;
  multiset<int>::iterator fnd = trips.find(indx);
  if(fnd == trips.end()) return false;
  trips.erase(fnd);
  //wcerr << "LE22 " << trips.size() << endl;
  return true;
}



void
ScoreTree::
recordGold(InputTree* tree, ParseStats& parseStats)
{
  if( tree->subTrees().empty() )
    {
      return;
    }
  ECString trm = tree->term();
  if(trm != L"" && trm != L"S1")
    {
      /* now this simply records how many applicable constits there are */
      if(trm == L"PRT") trm = L"ADVP";
      int indx = Term::get(trm)->toInt();
      indx  = indx*100 + equivInt(tree->start());
      indx = indx*500 + equivInt(tree->finish());
      //wcerr << "ADD " << indx << " " << trm << " "
      //   << equivInt(start_) << " " << equivInt(finish_) << endl;
      trips.insert(indx);
      //wcerr << "New in g " << *this << endl;
      parseStats.numInGold++;
    }
  InputTree                      *subTree;
  InputTreesIter  subTreeIter = tree->subTrees().begin();
  for( ; subTreeIter != tree->subTrees().end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      recordGold(subTree, parseStats );
    }
}



void
ScoreTree::
precisionRecall( InputTree* tree, ParseStats& parseStats )
{
  bool lexact1 = false;
  if( tree->subTrees().empty() ) return;

  InputTree                      *subTree;
  InputTreesIter  subTreeIter = tree->subTrees().begin();
  for( ; subTreeIter != tree->subTrees().end() ; subTreeIter++ )
    {
      subTree = *subTreeIter;
      precisionRecall(subTree, parseStats );
    }
  if( tree->term() == L"S1" ) return;
  parseStats.numInGuessed++;
  if( lexact2(tree) ) parseStats.numCorrect++;
}


int
ScoreTree::
equivInt(int x)
{
  int equivpos = equivPos[x];
  //wcerr << "EI " << x << " " << equivpos << endl;
  if(equivpos < 0) return x;
  else return equivInt(equivpos);
}

int
ScoreTree::
puncEquiv(int i, vector<ECString>& sr)
{
  if(i == 0) return i;
  if(scorePunctuation(sr[i-1])) return equivInt(i-1);
  return i;
}
  

void
ScoreTree::
setEquivInts(vector<ECString>& sr)
{
  int i;
  int len = sr.size();
  equivPos[0] = -1;
  for(i = 1 ; i <= len ; i++)
    {
      equivPos[i] = -1;
      int puncequiv = puncEquiv(i,sr);
      if(puncequiv < i) equivPos[i] = puncequiv;
    }
}

bool
ScoreTree::
scorePunctuation( const ECString trmString )
{
  const Term* trm = Term::get(trmString);
  if(!trm)
    {
      wcerr << "No such term: " << trmString << endl;
      assert(trm);
    }
  if(trm->isParen()) return false;
  return trm->isPunc();
}
