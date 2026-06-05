/*****************************************************************************/
/*! \author  Alejandro H. Toselli <ahector@prhlt.upv.es>
 *  \version 10.0
 *  \date    2015
 */

/* Copyright (C) 2014 by Pattern Recognition and Human Language
   Technology Research Center, Technological Institute of Computer Science,
   Valencia University of Technology, Valencia (Spain).

   Permission to use, copy, modify, and distribute this software and
   its documentation for any purpose and without fee is hereby
   granted, provided that the above copyright notice appear in all
   copies and that both that copyright notice and this permission
   notice appear in supporting documentation.  This software is
   provided "as is" without express or implied warranty.
*/

#include "WGN-Best.h"
#include "WGConfMeasure.h"

#include <algorithm>
#include <iomanip>

using std::showpoint;
using std::scientific;
using std::setprecision;
using std::ios;
using std::setw;
using std::fixed;

class TreeNode {
 public:
  int idSt;             // State ID
  int idInpArc;         // Arriving Arc ID to this node
  double faux;          // Accumulative probability
  double g;             // optimistic target function
  TreeNode *parent;     // Pointer to the parent node
  uint num_connections; // Number of connected son-nodes
  //bool operator()(const elem &a, const elem &b) const { return a.g<b.g; }
  bool operator()(const TreeNode *a, const TreeNode *b) const { 
    return a->g<b->g;
  }
  TreeNode(): idSt(-1), idInpArc(-1), faux(NDINF), g(NDINF), parent(NULL), num_connections(0) {}
  ~TreeNode() {}
};


TreeNode * addNodeToTree(WG &wg, int idInpArc, TreeNode *parent) {
  if ((parent==NULL && idInpArc>=0)) return NULL;
  Arc *arc = wg.getArc(idInpArc);
  if (parent && (arc==NULL)) return NULL;
  
  TreeNode *tn = new TreeNode;
  if (parent==NULL) {
    tn->idSt = wg.getInitialState();
    tn->faux = 0.0;
  } else {
    tn->idSt = arc->iTarget;
    tn->faux = parent->faux + arc->dProb;
    parent->num_connections++;
  }
  tn->idInpArc = idInpArc;
  tn->parent = parent;
  State *st = wg.getState(tn->idSt);
  tn->g = tn->faux + st->getMaxOUTArcProbAcc();
  
  return tn;
}


void delPathTree(TreeNode *node) {
  if ((node==NULL) || (node->num_connections>0)) return;
  TreeNode *pNode = node->parent;
  delete node; node=NULL;
  if (pNode && (--(pNode->num_connections) == 0)) delPathTree(pNode);
}


void pathRecover(WG &wg, const TreeNode *node, ostream &fd) {
  if (!node) return;
  TreeNode *pNode = node->parent;
  if (pNode) pathRecover(wg,pNode,fd);
  if (node->idInpArc >= 0) {
    Arc *arc=wg.getArc(node->idInpArc);
    fd << setw(6) << wg.getState(arc->iSource)->getTimeStamp() << " ";
    fd << setw(6) << wg.getState(arc->iTarget)->getTimeStamp() << " "; 
    fd << setw(17) << wg.getWord(arc->iIdWord) << " ";
    fd << setw(6) << "(" << arc->iPronunc << ") ";
    fd << setw(9) << showpoint << setprecision(3) << arc->dProb << " ";
    fd << setw(9) << showpoint << setprecision(3) << node->faux << " ";
    fd << setw(9) << showpoint << setprecision(6) << arc->dPostProb << " ";
    fd << setw(9) << showpoint << setprecision(6) << computeConfMeasure(wg,arc->iId) << " ";
    fd << setw(9) << showpoint << setprecision(3) <<
      arc->dAcProb * wg.getWGParam_accScl() * wg.getWGParam_WgDisp() << " ";
    fd << setw(9) << showpoint << setprecision(3) <<
      (arc->dPrProb * wg.getWGParam_prnScl() + arc->dLmProb * wg.getWGParam_lmScl() + wg.getWGParam_wIPen()) * wg.getWGParam_WgDisp();
    fd << endl;
  }
}


void branchAndBound(WG & wg, int nbest, float dBeamScr, ostream &fd, uint verb) {

  // Perform Viterbi backward probability computation
  //wg.viterbiBackward(); It has already done in the main.cc
  
  fd.setf(ios::fixed); // set number of decimals to 2
  double f=NDINF;

  vector<TreeNode*> vHeapBrBo;
  make_heap(vHeapBrBo.begin(),vHeapBrBo.end(),TreeNode());
  
  // Insert initial node in the heap
  TreeNode* node = addNodeToTree(wg, -1, NULL);
  vHeapBrBo.push_back(node);
  push_heap(vHeapBrBo.begin(),vHeapBrBo.end(),TreeNode());

  fd << "#!MLF!#" << endl;
  fd << "\"" << wg.getName() << "\"" << endl;
  fd << "#   Ti" << setw(7) << " Tf" << setw(18) << " Word" << setw(9) << " (Pron)";
  fd << setw(10) << " Lkh" << setw(10) << " Acc-Lkh" << setw(10) << " Pos-Prb";
  fd << setw(10) << " Cnf-Msr" << setw(10) << " Mph-Lkh";
  fd << setw(12) << " Lm-Lkh\n#" << endl;

  int nb = 0;  
  bool fstNBest = true;
  while (!vHeapBrBo.empty() && (nb<nbest || nbest<0)) {
    
    if (verb==3)
      cerr << "Current Heap Size: " << vHeapBrBo.size() << endl;

    int idSt = vHeapBrBo.front()->idSt;
    //int idInpArc = vHeapBrBo[0]->idInpArc;
    //double faux = vHeapBrBo[0]->faux;
    //double g = vHeapBrBo[0]->g;
    TreeNode* parent = vHeapBrBo.front();

    //cout << idSt << " " << faux << " " << g << " " << parent << endl;

    // Take out the node from the heap
    pop_heap(vHeapBrBo.begin(),vHeapBrBo.end(),TreeNode());
    vHeapBrBo.pop_back();

    //if (g < f) continue;

    if (idSt == wg.getFinalState()) {
      if (parent->faux > f) {
	if (nb>0) fd << "///" << endl;
	pathRecover(wg,parent,fd);
	fd << "Tot-Lkh: " << showpoint << setprecision(6) << parent->faux;
	fd << "    Pos-Prb: " << scientific << setprecision(6) << wg.normProbWhtRespTotalProb(parent->faux);
	fd << fixed << endl;
	nb++;
	if (fstNBest) {
	  fstNBest=false;
	  if (dBeamScr < 0.0) f = parent->faux + dBeamScr;
	  //cerr << "y " << dBeamScr << "   f " << f << endl;
	}
      }
      delPathTree(parent);
    } else {
      State *st = wg.getState(idSt);      
      TreeNode **nodesCollector = new TreeNode*[st->getNumOfOutputArcs()];
      for (uint j=0; j<st->getNumOfOutputArcs(); j++) nodesCollector[j] = NULL;
      
      for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
	int idArc = st->getOUTArcID(j);
	TreeNode* node = addNodeToTree(wg, idArc, parent);
	if (node->g > f) {
	  // Insert node in the heap
	  vHeapBrBo.push_back(node);
	  push_heap(vHeapBrBo.begin(),vHeapBrBo.end(),TreeNode());
	} else nodesCollector[j] = node;
      }

      // Delete gathered nodes whose node->g are lower than f
      for (uint j=0; j<st->getNumOfOutputArcs(); j++)
        if (nodesCollector[j]) delPathTree(nodesCollector[j]);
      delete [] nodesCollector;
    }
  }

  fd << "." << endl;
  // Delete remaining nodes in the heap in case there is one
  if (!vHeapBrBo.empty()) {
    vector<TreeNode*>::reverse_iterator rit;
    for (rit=vHeapBrBo.rbegin();rit!=vHeapBrBo.rend();rit++) delPathTree(*rit);
  }
}
