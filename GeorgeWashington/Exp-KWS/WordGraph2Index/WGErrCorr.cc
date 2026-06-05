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

#include "WGErrCorr.h"

#include "StringTokenizer.h"
#include <iostream>
#include <sstream>
#include <set>

using std::set;
using std::cerr;
using std::stringstream;

const uint uCstSus=1, uCstIns=1, uCstDel=1;

typedef enum { NON=-3, DEL=-2, INS=-1 } oprAndWID;

typedef struct TN {
  long int i,j;    // For adressing the node from which is arriving to
  uint cost;       // Accumulative cost
  oprAndWID iIdWord; // Note: 0>=:Sustituion
  TN(): i(-1), j(-1), cost(PUIINF), iIdWord(NON) {}
} TrellisNode;


void errorCorrecting(WG & wg, int *vWord, uint l, set<int> & stk, string &strResult) {

  uint numNodes = wg.getTotalNumberOfStates();

  const uint* vTopOrder = wg.getTopologicalOrder();
  TrellisNode **trellisMatrix = (TrellisNode **) new TrellisNode*[numNodes];
  for (uint i=0; i<numNodes; i++)
    trellisMatrix[i] = (TrellisNode *) new TrellisNode[l+1];
  
  // Initialization of the trellisMatrix node 0 0
  trellisMatrix[vTopOrder[0]][0].cost=0;

  // Initialization of the first trellisMatrix row: Insertions
  /* This is directly carried out in the normal operation of 
   * the DP algorithm */

  // Initialization of the first trellisMatrix column: Deletions
  for (uint i=0; i<numNodes; i++) {
    TrellisNode &tnS = trellisMatrix[vTopOrder[i]][0];

    State *st = wg.getState(vTopOrder[i]);    
    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc *arc = wg.getArc(st->getOUTArcID(j));
      uint uIdSt = wg.mapStateIDToTrueLoc(arc->iTarget);
      TrellisNode &tnT = trellisMatrix[uIdSt][0];
      if (tnS.cost+uCstDel<tnT.cost) {
	tnT.i=vTopOrder[i]; tnT.j=0; tnT.cost=tnS.cost+uCstDel; tnT.iIdWord=DEL;
      }
    }
  }
  
  // Dynamic programming
  for (uint tk=1; tk<=l; tk++)
    for (uint i=0; i<numNodes; i++) {
      TrellisNode &tnT = trellisMatrix[vTopOrder[i]][tk];

      // Substitution and deletion cases
      State *st = wg.getState(vTopOrder[i]);
      for (uint j=0; j<st->getNumOfInputArcs(); j++) {
	Arc *arc = wg.getArc(st->getINArcID(j));
	uint uIdSt = wg.mapStateIDToTrueLoc(arc->iSource);
	// Sustitution
	TrellisNode &tnS = trellisMatrix[uIdSt][tk-1];
	uint cost=uCstSus; if (arc->iIdWord == vWord[tk-1]) cost=0;
	if (tnS.cost+cost<=tnT.cost) {
	  tnT.i=uIdSt; tnT.j=tk-1; tnT.cost=tnS.cost+cost;
	  tnT.iIdWord=(oprAndWID)arc->iIdWord;
	}
	// Deletion
	TrellisNode &tnD = trellisMatrix[uIdSt][tk];
	if (tnD.cost+uCstDel<tnT.cost) {
	  tnT.i=uIdSt; tnT.j=tk; tnT.cost=tnD.cost+uCstDel;
	  tnT.iIdWord=DEL;
	}
      }
      
      // Insertion case
      TrellisNode &tnI = trellisMatrix[vTopOrder[i]][tk-1];
      if (tnI.cost+uCstIns<tnT.cost) {
	tnT.i=vTopOrder[i]; tnT.j=tk-1; tnT.cost=tnI.cost+uCstIns; tnT.iIdWord=INS;
      }
    }

  // Print the dynamic programming matrix
  // for (uint i=0; i<numNodes; i++) { 
  //   cerr << i << " ";
  //   for (uint tk=0; tk<=l; tk++) {
  //     TrellisNode &tn = trellisMatrix[i][tk];
  //     cerr << " (" << tn.i << "," << tn.j << ") ";
  //     if (tn.iIdWord==-2) cerr << "<DEL> ";
  //     else if (tn.iIdWord==-1) cerr << "<INS> ";
  //     else cerr << wg.getWord(tn.iIdWord);
  //     cerr << " " << tn.cost << " ";
  //   }
  //   cerr << endl;
  // }

  int iTotalCost = trellisMatrix[vTopOrder[numNodes-1]][l].cost;
  int costD=0, costI=0, costS=0, costH=0;

  string eword;
  TrellisNode *tn = &trellisMatrix[vTopOrder[numNodes-1]][l];
  while (true) {
    if (tn->iIdWord == NON) break;
    else if (tn->iIdWord == DEL) {
      eword.insert(0,"<DEL> ");
      costD += (tn->cost - trellisMatrix[tn->i][tn->j].cost);
    }
    else if (tn->iIdWord == INS) {
      eword.insert(0,"<INS> ");
      costI += (tn->cost - trellisMatrix[tn->i][tn->j].cost);
    }
    else {
      eword.insert(0,wg.getWord(tn->iIdWord)+" ");
      int dif = (tn->cost - trellisMatrix[tn->i][tn->j].cost);
      if (dif) costS++; else costH++;
    }
    tn = &trellisMatrix[tn->i][tn->j];
  }
  
  cerr << "Edited string: " << eword << endl;

  stringstream ss;
  ss << iTotalCost << " (H:" << costH << " S:" << costS;
  ss << " I:" << costI << " D:" << costD << ")";
  strResult = ss.str();
  
  for (uint i=0; i<numNodes; i++) delete [] trellisMatrix[i];
  delete [] trellisMatrix;
}



void errorCorrecting(WG &wg, const string &word, string &strResult, const string &tk) {

  StringTokenizer strToken(word," \t");
  uint len = strToken.countTokens();
  if (len==0) return;

  int *vWord = new int[len];

  string str;
  int iWid, itk, cont=0;
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((iWid=wg.getWordID(str))==-1) {
      cerr << "WARNING: String \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
    }
    vWord[cont++]=iWid;
    //cerr << str << " " << vWord[cont-1] << endl;
  }

  set<int> stk;
  strToken.addNewLine(tk);
  while (strToken.hasMoreTokens()) {
    str = strToken.nextToken();
    if ((itk=wg.getWordID(str))==-1) {
      cerr << "WARNING: Token \"" << str << "\" is out from the WG vocabulary.";
      cerr << endl;
    }
    stk.insert(itk);
  }

  cerr << " Input string: " << word << endl;

  errorCorrecting(wg,vWord,len,stk,strResult);
  delete [] vWord;
}
