#include "stdafx.h"
#include "Link.h"
#include "InputTree.h"
#include "Term.h"

Link*
Link::
do_link(int tint, bool& ans)
{
  LinksIter li = links_.begin();
  for( ; li != links_.end() ; li++)
    {
      Link* slink = (*li);
      if(slink->key() == tint)
	{
	  ans = false;
	  return slink;
	}
    }
  ans = true;
  Link* nlink = new Link(tint);
  //wcerr << "LN " << tint << endl;
  links_.push_back(nlink);
  return nlink;
}

Link*
Link::
is_unique(InputTree* tree, bool& ans, int& cnt)
{
  //wcerr << "IU " << *tree << endl;
  Link* nlink;
  const Term* trm = Term::get(tree->term());
  nlink = do_link(trm->toInt(), ans);
  if(trm->terminal_p())
    {
      cnt++;
      return nlink;
    }
  InputTreesIter iti = tree->subTrees().begin();
  for( ; iti != tree->subTrees().end() ; iti++)
    {
      nlink = nlink->is_unique((*iti), ans,cnt);
    }
  nlink = nlink->do_link(DUMMYVAL, ans);
  return nlink;
}
    
      
      
