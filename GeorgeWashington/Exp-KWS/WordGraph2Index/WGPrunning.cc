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

#include "WGPrunning.h"

#include <set>
#include <iostream>

using std::cerr;
using std::set;


/* --------------------------------------------------------------- */
/* Public Functions */
/* --------------------------------------------------------------- */

WG* reduceSize(WG & wg, double thrs) {

  set<int> *sNodesI = new set<int>, *sNodesF = new set<int>;
  set<int> *sNodesAux;
  WG *rwg = new WG();
  
  /* Set the parameters of the WG.
   * This must be done before starting to add arcs to the new WG */
  rwg->setWGParameters( wg.getWGParam_lmScl(), \
                        wg.getWGParam_wIPen(), \
                        wg.getWGParam_prnScl(),\
                        wg.getWGParam_accScl(),\
                        wg.getWGParam_WgDisp(),\
                        wg.getWGParam_tmScl()  \
                      );

  /* Search for the path with minimum likelihood,
   * but with its sign reversed. */
  //wg.viterbiForward(false); 
  //double dProbMin = wg.getState(wg.getFinalState())->getMaxINArcProbAcc() * (-1);
  //cerr << "VitProbMin: " << dProbMin << endl;
  //wg.resetProb();
  
  wg.viterbiForward(); wg.viterbiBackward();
  double dProbVit = wg.getState(wg.getFinalState())->getMaxINArcProbAcc();
  //cerr << "VitProbMax: " << dProbVit << endl;

  /* Look for the arc whose score of the maximum
   * likelihhood path that pass through is minimum */
  double dProbMin = PDINF;
  for (uint j=0; j<wg.getTotalNumberOfArcs(); j++) {
    Arc *arc = wg.getArc(j);
    State *st = wg.getState(arc->iSource);
    State *nst = wg.getState(arc->iTarget);
    double dProb = st->getMaxINArcProbAcc() + arc->dProb + nst->getMaxOUTArcProbAcc(); 
    if ( dProbMin > dProb ) dProbMin = dProb;
  }
  //cerr << "ProbMin: " << dProbMin << endl;

  // Starting
  State ist; ist.cpStatePart(*(wg.getState(wg.getInitialState())));
  rwg->addState(ist);
  rwg->setInitialState(ist.getID());
  //ist.print_Test(); //exit(0);
  sNodesI->insert(wg.getInitialState());

  while (!sNodesI->empty()) {
    int iId=*(sNodesI->begin()); sNodesI->erase(iId);
    State *st = wg.getState(iId);
 
    for (uint j=0; j<st->getNumOfOutputArcs(); j++) { 
      Arc *arc = wg.getArc(st->getOUTArcID(j));
      //arc->print_Test();
      
      State *nst = wg.getState(arc->iTarget);
      double dProb = st->getMaxINArcProbAcc() + arc->dProb + nst->getMaxOUTArcProbAcc();
      //cerr << "dProb: " << dProb << endl;
      
      if ( (dProb-dProbMin) >= thrs*(dProbVit-dProbMin) ) {

	//cerr << "dProb: " << dProb << endl;
	State auxSt; auxSt.cpStatePart(*nst);
	rwg->addState(auxSt);
	//nst.print_Test();
	
	if (nst->getID()==wg.getFinalState())
	  rwg->setFinalState(nst->getID());
	
	Arc auxArc; auxArc.cpArcPart(*arc);
	//narc.print_Test();
	rwg->addArc(auxArc,wg.getWord(arc->iIdWord));
	//(rwg->getArc(narc.iId))->print_Test(); exit(0);
	
	sNodesF->insert(nst->getID());
      }    
    }
    sNodesAux = sNodesI;
    sNodesI = sNodesF;
    sNodesF = sNodesAux;   
  }
  
  if (rwg->getFinalState()<0) {
    delete sNodesI; delete sNodesF;
    delete rwg;
    cerr << "ERROR: Word-Graph over prunning ..." << endl;
    return NULL;
  }
  
  return rwg;
}
