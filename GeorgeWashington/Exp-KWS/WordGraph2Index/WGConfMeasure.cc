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

#include "WGConfMeasure.h"

#include "LogsRobust.h"
//#include <iomanip>

double computeConfMeasure(WG & wg, int iArcId) {
  Arc *arc = wg.getArc(iArcId);
  int idW = arc->iIdWord;
  State *sti = wg.getState(arc->iSource);
  uint ti = sti->getTimeStamp();
  State *stf = wg.getState(arc->iTarget);
  uint tf = stf->getTimeStamp();

  double scrMax = 0.0;
  uint t = ti;
  while (t < tf) {
    double scr = wg.getPProbMatrix(t,idW);
    if (scrMax<scr) scrMax = scr;
    t++;
  }
  return scrMax;
}


void computeArcMaxProb(WG & wg, int iArcId, double &dMaxProb, uint &sTime, uint &fTime) {
  Arc *arc = wg.getArc(iArcId);
  int idW = arc->iIdWord;
  State *sti = wg.getState(arc->iSource);
  uint ti = sti->getTimeStamp();
  State *stf = wg.getState(arc->iTarget);
  uint tf = stf->getTimeStamp();
  uint bgTm, fnTm;

  double scrMax = 0.0;
  uint t = bgTm = fnTm = ti;
  bool bEndTime = false;
  while (t < tf) {
    double scr = wg.getPProbMatrix(t,idW);
    if (scrMax<scr) { scrMax = scr; bgTm = fnTm = t; bEndTime = false; }
    else if (scrMax==scr && !bEndTime) fnTm = t;
    else bEndTime = true;
    t++;
  }
  dMaxProb=scrMax; sTime=bgTm; fTime=fnTm;
}


/* Old implementation with left-right word-border positions computed
   exactly on the max-score */
// void computeWordMaxProb(WG & wg, int idxW, double &dMaxProb, uint &sTime, uint &fTime) {
//   uint bgTm, fnTm;
//   double scrMax = 0.0;
//   uint t = bgTm = fnTm = 0;
//   uint tf = wg.getNumberOfFrames();
//   bool bEndTime = false;
//   while (t < tf) {
//     double scr = wg.getPProbMatrix(t,idxW);
//     if (scrMax<scr) { scrMax = scr; bgTm = fnTm = t; bEndTime = false; }
//     else if (scrMax==scr && !bEndTime) fnTm = t;
//     else bEndTime = true;
//     t++;
//   }
//   dMaxProb=scrMax; sTime=bgTm; fTime=fnTm;
// }
/* New implementation with left-right word-border positions computed
   over 0.9 of the max-score */
void computeWordMaxProb(WG & wg, int idxW, double &dMaxProb, uint &sTime, uint &fTime) {
  uint mxTm, bgTm, fnTm;
  double scrMax = 0.0, scrAux = 0.0;
  uint t = mxTm = bgTm = fnTm = 0;
  uint tf = wg.getNumberOfFrames();
  while (t < tf) {
    double scr = wg.getPProbMatrix(t,idxW);
    if (scrMax < scr) { scrMax = scr; mxTm = t; }
    t++;
  }
  scrAux = THRESHMAXSRC * scrMax;
  for (t=mxTm; t>0  && wg.getPProbMatrix(t-1,idxW)>=scrAux; t--); bgTm = t;
  for (t=mxTm; t<tf && wg.getPProbMatrix(t,idxW)>=scrAux; t++); fnTm = t-1;
  dMaxProb=scrMax; sTime=bgTm; fTime=fnTm;
}


/* Using the disjoint probability formula */
double computeWordTotProb(WG & wg, int idxW) {
  const double dThrs = 1e-6;
  vector<double> vCls;
  bool bFound = true;
  double scr, scrPrv = wg.getPProbMatrix(0,idxW);
  uint t = 1, tf = wg.getNumberOfFrames();
  while (t < tf) {
    scr = wg.getPProbMatrix(t,idxW);
    if (fabs(scrPrv-scr)<dThrs) ;
    else if (scrPrv<scr) bFound = true;
    else if (bFound) {
      bFound = false;
      if (scrPrv>=dThrs) 
	vCls.push_back(scrPrv);
    }
    scrPrv = scr;
    t++;
  }
  if (bFound) vCls.push_back(scrPrv);
 
  cerr << wg.getWord(idxW) << "\tRep: " << vCls.size() << endl;
  for (uint i=0; i<vCls.size(); i++) cerr << vCls[i] << " "; cerr << endl;

  // Compute the total disjoint probability
  for (uint i=1; i<vCls.size(); i++)
    vCls[i] = vCls[i] + vCls[i-1] - vCls[i] * vCls[i-1];
  //return vCls.empty()?0.0:vCls.back();
  return vCls.back();
}


/* This really does not produce the expected results 
   It was deprecated. */
// double computeWordExactProb(WG & wg, int idxW) {
//   const float MRG=0.05;
//   uint bgTm, fnTm;
//   double dLProd = 0.0, scrPrev = 0.0;
//   uint t = bgTm = fnTm = 0;
//   uint tf = wg.getNumberOfFrames();
//   while (t < tf) {
//     double scr = wg.getPProbMatrix(t,idxW);
//     if (scrPrev!=scr) {
//       if ((fnTm-bgTm)*100/tf > MRG)
// 	dLProd += ProbFloat2ProbLogFloat(1.0 - scrPrev);
//       scrPrev = scr;
//       bgTm = fnTm = t;
//     } else fnTm = t;
//     t++;
//   }
//   if ((fnTm-bgTm)*100/tf > MRG) dLProd += ProbFloat2ProbLogFloat(1.0 - scrPrev);
//   return (1.0 - ProbLogFloat2ProbFloat(dLProd));
// }
double computeWordExactProb(WG & wg, int idxW) {
  static double dFullFrw = wg.getState(wg.getFinalState())->getForwardProb();
  
  const uint* vTopOrder = wg.getTopologicalOrder();
  uint uTotNumOfStates = wg.getTotalNumberOfStates();
  double *vForwProb = new double[uTotNumOfStates];

  for (uint i=0; i<uTotNumOfStates; i++) {
    
    State *st = wg.getState(vTopOrder[i]); /* For i=0, this should be the initial State */
    vForwProb[vTopOrder[i]] = NDINF;
     
    for (uint j=0; j<st->getNumOfInputArcs(); j++) {
      Arc *arc = wg.getArc(st->getINArcID(j));
      State *prevSt = wg.getState(arc->iSource);

      uint uIndx = wg.mapStateIDToTrueLoc(arc->iSource);

      /* L(i) = \sum_{ (j, v, p) \in Input(i) } p * L(j) * [v != w] + p * F(j) * [v == w]
         L(j): w word forward of the preceding node j, F(j): full forward of the 
	 preceding node j */
      double dProb = (arc->iIdWord == idxW) ? prevSt->getForwardProb() : vForwProb[uIndx];
      dProb += arc->dProb;
      
      vForwProb[vTopOrder[i]] = sumLogs(vForwProb[vTopOrder[i]],dProb);
    }
  }
  double dWordFrw = vForwProb[vTopOrder[uTotNumOfStates-1]];

  //cerr << std::showpoint << std::setprecision(16) << dFullFrw << " " << dWordFrw << endl;
  delete [] vForwProb;
  return (ProbLogFloat2ProbFloat(dWordFrw - dFullFrw));
}


double computeLineWordProb(WG & wg, int idxW) {
  double scrSum = 0.0;
  for (uint t=0; t<wg.getNumberOfFrames(); t++)
    scrSum += wg.getPProbMatrix(t,idxW);
  return scrSum/wg.getNumberOfFrames();
}


double computeWordGlobalProb(WG & wg, int idxW) {
  double scrSum = 0.0;
  for (uint i=0; i<wg.getTotalNumberOfArcs(); i++) {
    Arc *arc = wg.getArc(i);
    if (arc->iIdWord!=idxW) continue;
    scrSum += arc->dPostProb;
  }
  return scrSum;
}


/* This is an implementation proposed by Enrique Vidal. It improves
   the version of the plain sum of word edge post-probabilities. Its
   is assumed that the states are sorted according their time-stamp. */
/*
double computeWordGlobalProb(WG & wg, int idxW) {

  vector<double> vCls;
  uint tl=0, tr=0;
  for (uint s=0; s<wg.getTotalNumberOfStates(); s++) {
    State *sti = wg.getState(s);    
    uint ti = sti->getTimeStamp();
    
    for (uint j=0; j<sti->getNumOfOutputArcs(); j++) {
      Arc *arc = wg.getArc(sti->getOUTArcID(j));
      if (arc->iIdWord!=idxW) continue;

      State *stf = wg.getState(arc->iTarget);
      uint tf = stf->getTimeStamp();

      if (vCls.empty()) {
	tl=ti; tr=tf; vCls.push_back(0.0);
	if (tr-tl) vCls.back()=arc->dPostProb;
	continue;
      } 

      if (ti<tr) {
	tl=ti;
	if (tf<tr) tr=tf;
	vCls.back() += arc->dPostProb;
      } else {
	tl=ti; tr=tf; vCls.push_back(0.0);
	if (tr-tl) vCls.back()=arc->dPostProb;
      }

    } 
  }

  // Compute the total disjoint probability
  for (uint i=1; i<vCls.size(); i++)
    vCls[i] = vCls[i] + vCls[i-1] - vCls[i] * vCls[i-1];

  return vCls.back();
}
*/


/* This is an implementation proposed by Enrique Vidal. It improves
   the version of the plain sum of word edge post-probabilities. Its
   is assumed that the states are sorted according to their
   time-stamp. */
double* computeWordGlobalProb(WG & wg) {
  //return NULL;
  double *vConf = new double[wg.getVocSize()];
 
  uint cnt=0;
  Arc **vArcs = new Arc*[wg.getTotalNumberOfArcs()];
  for (uint s=0; s<wg.getTotalNumberOfStates(); s++) {
    State *sti = wg.getState(s);
    for (uint j=0; j<sti->getNumOfOutputArcs(); j++)
      vArcs[cnt++] = wg.getArc(sti->getOUTArcID(j));
  }
  //return vConf;
  vector<double> vCls; 
  uint tl=0, tr=0;
  for (uint v=1; v<=wg.getVocSize(); v++) {
    vCls.clear();
    for (uint i=0; i<cnt; i++) {
      Arc *arc = vArcs[i];
      if (arc->iIdWord!=(int)v) continue;

      State *sti = wg.getState(arc->iSource);    
      uint ti = sti->getTimeStamp();
      State *stf = wg.getState(arc->iTarget);
      uint tf = stf->getTimeStamp();
	
      if (vCls.empty()) {
	tl=ti; tr=tf; vCls.push_back(0.0);
	if (tr-tl) vCls.back()=arc->dPostProb;
	continue;
      } 
	
      if (ti<tr) {
	tl=ti;
	if (tf<tr) tr=tf;
	vCls.back() += arc->dPostProb;
      } else if (tr-tl) {
	tl=ti; tr=tf; vCls.push_back(arc->dPostProb);
      }	
    }
 
    // Compute the total disjoint probability
    for (uint i=1; i<vCls.size(); i++)
      vCls[i] = vCls[i] + vCls[i-1] - vCls[i] * vCls[i-1];
    cerr << wg.getWord((int)v) << "\tRep: " << vCls.size() << endl;
    vConf[v-1]=vCls.back();
  }
  delete [] vArcs;
  
  return vConf;
}

/* This is an implementation proposed by Enrique Vidal. It improves
   the version of the plain sum of word edge post-probabilities. Its
   is assumed that the states are sorted according to their
   time-stamp. This is actually "computeWordGlobalProb" extended to
   produce the N-best word localizations in the line. */
vector<multiScore>* computeMultiWordMaxProb(WG & wg) {
  //return NULL;
  vector<multiScore> *vConf = new vector<multiScore>[wg.getVocSize()];
 
  uint cnt=0;
  Arc **vArcs = new Arc*[wg.getTotalNumberOfArcs()];
  for (uint s=0; s<wg.getTotalNumberOfStates(); s++) {
    State *sti = wg.getState(s);
    for (uint j=0; j<sti->getNumOfOutputArcs(); j++)
      vArcs[cnt++] = wg.getArc(sti->getOUTArcID(j));
  }

  double scrAux;
  uint t;
  for (uint v=1; v<=wg.getVocSize(); v++) {
    
    vector<multiScore> & vCls = vConf[v-1];
    for (uint i=0; i<cnt; i++) {
      Arc *arc = vArcs[i];
      if (arc->iIdWord!=(int)v) continue;

      State *sti = wg.getState(arc->iSource);    
      uint ti = sti->getTimeStamp();
      State *stf = wg.getState(arc->iTarget);
      uint tf = stf->getTimeStamp() - 1;
	
      if (vCls.empty()) {
	vCls.push_back({0.0,ti,tf});
	if (tf>ti) vCls.back().dPostProb = arc->dPostProb;
	continue;
      } 
	
      if (ti<vCls.back().fTime) {
	vCls.back().sTime = ti;
	if (tf<vCls.back().fTime) vCls.back().fTime = tf;
	vCls.back().dPostProb += arc->dPostProb;
      } else if (tf>ti) {
        // Compute a fine detection of word bounds
        scrAux = THRESHMAXSRC * vCls.back().dPostProb;
        for (t=vCls.back().sTime; t>0 && wg.getPProbMatrix(t-1,(int)v)>=scrAux; t--); vCls.back().sTime = t;
	for (t=vCls.back().fTime; t<wg.getNumberOfFrames() && wg.getPProbMatrix(t,(int)v)>=scrAux; t++); vCls.back().fTime = t-1;
        // Set a new cluster
	vCls.push_back({arc->dPostProb,ti,tf});
      }
    }
    // Compute a fine detection of word bounds
    scrAux = THRESHMAXSRC * vCls.back().dPostProb;
    for (t=vCls.back().sTime; t>0 && wg.getPProbMatrix(t-1,(int)v)>=scrAux; t--); vCls.back().sTime = t;
    for (t=vCls.back().fTime; t<wg.getNumberOfFrames() && wg.getPProbMatrix(t,(int)v)>=scrAux; t++); vCls.back().fTime = t-1;

  }
  delete [] vArcs;
  
  return vConf;
}
