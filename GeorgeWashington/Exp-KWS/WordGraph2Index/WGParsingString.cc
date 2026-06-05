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

#include "WGParsingString.h"
#include "StringTokenizer.h"

#include <set>
#include <iostream>

using std::cerr;
using std::set;


bool parsingString(WG & wg, int *vWord, uint l, set<int> & stk) {

  set<int> *sNodesI = new set<int>, *sNodesF = new set<int>;
  set<int> *sNodesAux;
  bool reachFinalState = false, tokenFound = false;
  uint p;

  // Starting First Pass
  sNodesI->insert(wg.getInitialState());

  p=0;
  do {
    // for p==l we are asumming to parse tokens ID set (stk)
    int idW = (p<l)?vWord[p]:-1;
    if (idW!=-1) tokenFound = false; 

    //reachFinalState = false;
    while (!sNodesI->empty()) {
      int iId=*(sNodesI->begin()); sNodesI->erase(iId);
      State *st = wg.getState(iId);
      
      if (idW==-1 && st->getID()==wg.getFinalState())
        reachFinalState = true;

      for (uint j=0; j<st->getNumOfOutputArcs(); j++) { 
	Arc *arc = wg.getArc(st->getOUTArcID(j));
	//arc->print_Test();
	if (arc->iIdWord==idW) {
	  sNodesF->insert(arc->iTarget);
          tokenFound = true;
        } else if (stk.count(arc->iIdWord)>0)
	  sNodesI->insert(arc->iTarget);
      }	
    }
    sNodesAux = sNodesI;
    sNodesI = sNodesF;
    sNodesF = sNodesAux;
    if (p<l) p++;
  } while (!sNodesI->empty() && tokenFound);

  delete [] vWord;
  delete sNodesI; delete sNodesF;

  if (tokenFound && reachFinalState) return true;
  return false;
}


WG* parsingStringToGraph(WG & wg, int *vWord, uint l, set<int> & stk) {

  set<int> *sNodesI = new set<int>, *sNodesF = new set<int>;
  set<int> *sNodesAux;
  WG *pwg = new WG();
  uint p;

  // Starting First Pass
  State ist; ist.cpStatePart(*(wg.getState(wg.getInitialState())));
  pwg->addState(ist);
  pwg->setInitialState(ist.getID());
  //ist.print_Test(); //exit(0);
  sNodesI->insert(wg.getInitialState());

  p=0;
  do {    
    // for p==l we are asumming to parse tokens ID set (stk)
    int idW = (p<l)?vWord[p]:-1;

    while (!sNodesI->empty()) {
      int iId=*(sNodesI->begin()); sNodesI->erase(iId);
      State *st = wg.getState(iId);
 
      for (uint j=0; j<st->getNumOfOutputArcs(); j++) { 
	Arc *arc = wg.getArc(st->getOUTArcID(j));
	//arc->print_Test();

	if (arc->iIdWord==idW || stk.count(arc->iIdWord)>0) {
	  State nst; nst.cpStatePart(*(wg.getState(arc->iTarget)));
	  pwg->addState(nst);
	  if (arc->iTarget==wg.getFinalState())
	    pwg->setFinalState(arc->iTarget);
	  //nst.print_Test();

	  Arc narc; narc.cpArcPart(*arc);
	  //narc.print_Test();
	  pwg->addArc(narc,wg.getWord(arc->iIdWord));
	  //(pwg->getArc(narc.iId))->print_Test(); exit(0);
	  if (stk.count(arc->iIdWord)>0) sNodesI->insert(arc->iTarget);
	  else sNodesF->insert(arc->iTarget);
	}

      }	
    }
    // Get Word IDs from the new pwg WG 
    if (p<l) vWord[p]=pwg->getWordID(wg.getWord(idW));
    sNodesAux = sNodesI;
    sNodesI = sNodesF;
    sNodesF = sNodesAux;
    if (p<l) p++;
  } while (!sNodesI->empty());
  //itk=pwg->getWordID(wg.getWord(itk));

  // Get Token IDs from the new pwg WG 
  set<int> saux;
  set<int>::const_iterator it = stk.begin();
  for (it=stk.begin(); it!=stk.end(); it++) {
    int itk=pwg->getWordID(wg.getWord(*it));
    if (itk<0) continue;
    saux.insert(itk);
  }

  if (pwg->getFinalState()<0) {
    delete [] vWord;
    delete sNodesI; delete sNodesF;
    delete pwg;
    cerr << "ERROR: It was not possible to parse the string." << endl;
    return NULL;
  }
  // return pwg;


  // Starting Second Pass
  WG *rpwg = new WG();

  State fst; fst.cpStatePart(*(pwg->getState(pwg->getFinalState())));
  rpwg->addState(fst);
  rpwg->setFinalState(fst.getID());
  //fst.print_Test(); //exit(0);
  sNodesI->insert(pwg->getFinalState());

  p=l;
  do {
    //cerr << p << " " << (qNodesI->size()) << endl;
    int idW = (p>0)?vWord[p-1]:-1;

    while (!sNodesI->empty()) {
      int iId=*(sNodesI->begin()); sNodesI->erase(iId);     
      State *st = pwg->getState(iId);

      //st->print_Test();
      for (uint j=0; j<st->getNumOfInputArcs(); j++) {
	Arc *arc = pwg->getArc(st->getINArcID(j));
	//arc->print_Test();

	if (arc->iIdWord==idW || saux.count(arc->iIdWord)>0) {
	  State nst; nst.cpStatePart(*(pwg->getState(arc->iSource)));
	  rpwg->addState(nst);
	  if (arc->iSource==pwg->getInitialState())
	    rpwg->setInitialState(arc->iSource);
	  //nst.print_Test();

	  Arc narc; narc.cpArcPart(*arc);
	  //narc.print_Test();
	  rpwg->addArc(narc,pwg->getWord(arc->iIdWord));
	  //(rpwg->getArc(narc.iId))->print_Test(); exit(0);
	  if (saux.count(arc->iIdWord)) sNodesI->insert(arc->iSource);
	  else sNodesF->insert(arc->iSource);
	}

      }
    }
    // Get Word IDs from the new rpwg WG
    if (p>0) vWord[p-1]=rpwg->getWordID(pwg->getWord(idW));
    sNodesAux = sNodesI;
    sNodesI = sNodesF;
    sNodesF = sNodesAux;
    if (p>0) p--;
  } while (!sNodesI->empty());

  // Get Token IDs from the new rpwg WG
  saux.clear(); it = stk.begin();
  for (it=stk.begin(); it!=stk.end(); it++) {
    int itk=rpwg->getWordID(wg.getWord(*it));
    if (itk<0) continue;
    saux.insert(itk);
  }

  //cerr << l << " " << p << endl;
  delete pwg;
  if (rpwg->getInitialState()<0) {
    delete [] vWord;
    delete sNodesI; delete sNodesF;
    delete rpwg;
    cerr << "ERROR: It was not possible to parse the string." << endl;
    return NULL;
  }
  //return rpwg;


  // Starting Third Pass
  WG *fpwg = new WG();

  ist.cpStatePart(*(rpwg->getState(rpwg->getInitialState())));
  fpwg->addState(ist);
  fpwg->setInitialState(ist.getID());
  //ist.print_Test(); //exit(0);
  sNodesI->insert(rpwg->getInitialState());

  p=0;
  do {    
    // for p>=l we are asumming to parse tokens ID set (stk)
    int idW = (p<l)?vWord[p]:-1;

    while (!sNodesI->empty()) {
      int iId=*(sNodesI->begin()); sNodesI->erase(iId);
      State *st = rpwg->getState(iId);
 
      for (uint j=0; j<st->getNumOfOutputArcs(); j++) { 
	Arc *arc = rpwg->getArc(st->getOUTArcID(j));
	//arc->print_Test();

	if (arc->iIdWord==idW || saux.count(arc->iIdWord)>0) {
	  State nst; nst.cpStatePart(*(rpwg->getState(arc->iTarget)));
	  fpwg->addState(nst);
	  if (arc->iTarget==rpwg->getFinalState())
	    fpwg->setFinalState(arc->iTarget);
	  //nst.print_Test();

	  Arc narc; narc.cpArcPart(*arc);
	  //narc.print_Test();
	  fpwg->addArc(narc,rpwg->getWord(arc->iIdWord));
	  //(fpwg->getArc(narc.iId))->print_Test(); exit(0);
	  if (saux.count(arc->iIdWord)>0) sNodesI->insert(arc->iTarget);
	  else sNodesF->insert(arc->iTarget);
	}
      }
    }
    sNodesAux = sNodesI;
    sNodesI = sNodesF;
    sNodesF = sNodesAux;
    if (p<l) p++;
  } while (!sNodesI->empty());

  delete [] vWord;
  delete rpwg;
  delete sNodesI; delete sNodesF;
  // if (fpwg->getFinalState()<0) {
  //   delete fpwg;
  //   cerr << "ERROR: It was not possible to parse the string." << endl;
  //   return NULL;
  // }

  // Set the parameters of the WG
  fpwg->setWGParameters( wg.getWGParam_lmScl(), \
                         wg.getWGParam_wIPen(), \
			 wg.getWGParam_prnScl(),\
			 wg.getWGParam_accScl(),\
			 wg.getWGParam_WgDisp(),\
			 wg.getWGParam_tmScl()  \
		       );

  return fpwg;
}



/* --------------------------------------------------------------- */
/* Public Functions */
/* --------------------------------------------------------------- */

bool parsingString(WG & wg, const string & word, const string & tk) {

  StringTokenizer strToken(word,"= \t");
  uint len = strToken.countTokens();
  int *vWord = new int[len];

  set<int> stk;
  string str;
  int iWid, itk, cont=0;
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((iWid=wg.getWordID(str))==-1) {
      delete [] vWord;
      cerr << "ERROR: String \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
      return false;
    }
    vWord[cont++]=iWid;
    //cerr << str << " " << vWord[cont-1] << endl;
  }

  strToken.addNewLine(tk);
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((itk=wg.getWordID(str))==-1) {
      cerr << "WARNING: Token \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
    }
    stk.insert(itk);
  }
  
  return parsingString(wg,vWord,len,stk);
}

WG* parsingStringToGraph(WG & wg, const string & word, const string & tk) {

  StringTokenizer strToken(word,"= \t");
  uint len = strToken.countTokens();
  int *vWord = new int[len];

  set<int> stk;
  string str;
  int iWid, itk, cont=0;
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((iWid=wg.getWordID(str))==-1) {
      delete [] vWord;
      cerr << "ERROR: String \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
      return NULL;
    }
    vWord[cont++]=iWid;
    //cerr << str << " " << vWord[cont-1] << endl;
  }

  strToken.addNewLine(tk);
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((itk=wg.getWordID(str))==-1) {
      cerr << "WARNING: Token \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
    }
    stk.insert(itk);
  }
  
  return parsingStringToGraph(wg,vWord,len,stk);
}
