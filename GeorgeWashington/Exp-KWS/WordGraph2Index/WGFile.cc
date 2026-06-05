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

#include "WGFile.h"
#include "StringTokenizer.h"
#include "WGConfMeasure.h"

#include <set>
#include <iomanip>

using std::set;
using std::cerr;
using std::showpoint;
using std::scientific;
using std::setprecision;
using std::ios;

/////////////////////////////////////
// WGFile CLASS METHODS
/////////////////////////////////////

/* Set grammar scale factor, word insertion penalty, 
   morphologic scale factor and pronunciation scale factor */
void WGFile::setWGParameters(float lmsf, float wip, float psf, float asf, float wgd, float tscl) {
  lmScl = lmsf;
  wIPen = wip;
  prnScl = psf;
  accScl = asf;
  wgDisp = wgd;
  tmScl = tscl;
}

/* Method to read one "valid" line for the grammar file, ruling
   out blank lines, remarks, etc. */
string WGFile::readLine() {
  string aux("");
  while (getline(*ifd,aux)) {
    uLine++;
    // deteccion de comentarios
    size_t sPos = aux.find_first_not_of(" \t\r");
    if (sPos==string::npos || (sPos==aux.find_first_of("#"))) {
      if (verbosity==2) cerr << "Comments: " << aux << endl;
      continue;
    }
    if (verbosity==2) cerr << "Valid Read Line: " << aux << endl;
    return aux;
  }
  aux.clear();
  return aux;
}


/* Method to read all the states of the grammar file in HTK format */
bool WGFile::readElements_HTK(WG *wg, const string & dummyTk) {
  cerr.setf(ios::fixed); cerr.precision(2); // set number of decimals to 2
  string line, token, wtoken;
  bool bFound, arcsLabelled = true; // Asumming that the word-labels are
                                    // placed in the arcs
  // Store the dummy labels into a set
  set<string> sDumTk;
  //StringTokenizer strToken(dummyTk,"= \t");
  StringTokenizer strToken(dummyTk," \t","=");
  while (strToken.hasMoreTokens()) sDumTk.insert(strToken.nextToken());

  line=readLine();
  strToken.addNewLine(line);
  token=strToken.nextToken();
  
  // Read HEADER of the SLF file
  // Look for the header specifying total number of nodes and arcs 
  bFound=false;
  uint uTNofS=0, uTNofA=0; 
  while (line!="") {

    if (lowerCase(token)=="lmscale" && lmScl==UNSET_PAR) {
      lmScl=strToken.nextFloatToken();
      cerr << "WARNING: Grammar Scale Factor is set to: " << lmScl << endl;
    }
    else if (lowerCase(token)=="wdpenalty" && wIPen==UNSET_PAR) {
      wIPen=strToken.nextFloatToken();
      cerr << "WARNING: Word Insertion Penalty is set to: " << wIPen << endl;
    }
    else if (lowerCase(token)=="prscale" && prnScl==UNSET_PAR) {
      prnScl=strToken.nextFloatToken();
      cerr << "WARNING: Pronunciation Scale Factor is set to: " << prnScl << endl;
    }
    else if (lowerCase(token)=="acscale" && accScl==UNSET_PAR) {
      accScl=strToken.nextFloatToken();
      cerr << "WARNING: Morphologic Scale Factor is set to: " << accScl << endl;
    }
    else if (token=="N") uTNofS=strToken.nextIntToken();
    else if (token=="L") {
      uTNofA=strToken.nextIntToken();
      bFound=true;
      break;
    }   
    while (!strToken.hasMoreTokens()) {
      line=readLine();
      strToken.addNewLine(line);
    }
    token=strToken.nextToken();
  }
  if (!bFound) {
    cerr << "ERROR!!! Expecting N= total number of nodes in line " << uLine << endl;
    return false;
  }
  if (verbosity==1) 
    cerr << "Info: Nº Nodes = " << uTNofS << "     Nº Arcs = " << uTNofA << endl;
  
  // In case there isn't "wdpenalty", "wdpenalty", "acscale" or "prscale" in the input file
  if (lmScl==UNSET_PAR) lmScl=1.0;
  if (wIPen==UNSET_PAR) wIPen=0.0;
  if (prnScl==UNSET_PAR) prnScl=1.0;
  if (accScl==UNSET_PAR) accScl=1.0;

  // Set the parameters of the WG
  wg->setWGParameters(lmScl,wIPen,prnScl,accScl,wgDisp,tmScl);
 
  uint i=0;
  while (i<uTNofS) {
    line=readLine();
    strToken.addNewLine(line);
    /* Problem when the word of the node is '=' symbol */
    //     if (strToken.countTokens()!=4) {
    //       cerr << "ERROR!!! Format in line " << iLine << endl;
    //       return true;
    //     }
    if (strToken.nextToken()=="I") {
      State st;
      st.setID(strToken.nextIntToken());

      wtoken="";
      while (strToken.hasMoreTokens()) {
	token=strToken.nextToken();
	
	if (token=="t") {
	  float fts = strToken.nextFloatToken();
	  st.setTimeStamp(uint(fts*tmScl+0.5)); // To round-up to the near value
	} else if (token=="W") {
	  if (arcsLabelled) cerr << "WARNING: Word Labels are located in the WG Nodes" << endl;
	  arcsLabelled = false;
	  wtoken=strToken.nextToken();
	} else if (token=="v") {
	  st.setPronunc(strToken.nextIntToken());
	} else {
	  cerr << "ERROR!!! Format syntax in line " << uLine << endl;
	  return false;
	}
      }
      
      if (wg->getTotalNumberOfStates()<uTNofS && \
          wg->getTotalNumberOfStates()==(uint)st.getID())
        wg->addState(st,wtoken);
      else {
	cerr << "ERROR!!! Inconsistent storage of node " << i << endl;
	return false;
      }
      i++;
    } else {
      cerr << "ERROR!!! Expecting I= number of node in line " << uLine << endl;
      return false;
    }
  }

  uint j=0;
  while (j<uTNofA) {
    line=readLine();
    strToken.addNewLine(line);
    // if (strToken.countTokens()!=8) {
    //   cerr << "ERROR!!! Format in line " << uLine << endl;
    //   return true;
    // }
    if (strToken.nextToken()=="J") {
      Arc arc;
      arc.iId=strToken.nextIntToken();

      if (strToken.nextToken()=="S")
	arc.iSource=strToken.nextIntToken();
      else {
	cerr << "ERROR!!! Format syntax in line " << uLine << endl;
	return false;
      }

      if (strToken.nextToken()=="E") {
	arc.iTarget=strToken.nextIntToken();
      } else {
	cerr << "ERROR!!! Format syntax in line " << uLine << endl;
	return false;
      }

      bool isDummyToken = false;
      if (!arcsLabelled) {
	State *st = wg->getState(arc.iTarget);
	if (sDumTk.count(wg->getWord(st->getIDword()))) isDummyToken = true;
      }

      wtoken="";
      while (strToken.hasMoreTokens()) {
	token=strToken.nextToken();

	if (token=="W") {
	  wtoken=strToken.nextToken();
	  if (sDumTk.count(wtoken)) isDummyToken = true;
	} else if (token=="v") {
	  arc.iPronunc = strToken.nextIntToken();
	} else if (token=="a") {
	  float faux = strToken.nextFloatToken();
	  arc.dAcProb = faux;
	  //arc.dAcProb=(isDummyToken)?0.0:faux * accScl * wgDisp;
	} else if (token=="r") {
	  float faux = strToken.nextFloatToken();
	  arc.dPrProb = faux;
	  //arc.dAcProb=(faux * prnScl * wgDisp);
	} else if (token=="l") {
	  float faux = strToken.nextFloatToken();
	  arc.dLmProb = faux;
	  //arc.dLmProb=(faux * lmScl + wIPen) * wgDisp;
	} else if (token=="d") {
	  /* Not implemented yet! */
	  strToken.nextToken();
	} else {
	  cerr << "ERROR!!! Format syntax in line " << uLine << endl;
	  return false;
	}
      }

      if (wg->getTotalNumberOfArcs()>=uTNofA || \
          /*wg->getTotalNumberOfArcs()!=(uint)arc.iId ||*/ \
          (!wg->addArc(arc,wtoken,isDummyToken))) {
	cerr << "ERROR!!! Inconsistent storage of arc " << j << endl;
	return false;
      }
      j++;
    } else {
      cerr << "ERROR!!! Expecting J= arc ID in line " << uLine << endl;
      return false;
    }
  }

  if (verbosity==1)
    cerr << "Info: Vocabulary Size = " << wg->getVocSize() << endl;

  const uint *to = wg->getTopologicalOrder();
  wg->setInitialState(to[0]);
  wg->setFinalState(to[uTNofS-1]);

  return true;
}



/* Read WG File in HTK format */
bool WGFile::readFile_HTK(istream & fd, WG *wg, const string & dummyTk) {
  uLine=0;
  ifd=&fd;
  if (!(readElements_HTK(wg,dummyTk))) return false;
  //wgVoc.print(cout); exit(-1);
  return true;
}


/* Print the WG to outstream in HTK format */
void WGFile::print_HTK(WG *wg, ostream & fd) {
  fd.setf(ios::fixed); fd.precision(6); // set number of decimals to 2
  fd << "VERSION=1.0" << endl;
  fd << "lmscale=" << wg->getWGParam_lmScl();
  fd << "  wdpenalty=" << wg->getWGParam_wIPen() << endl;
  fd << "prscale=" << wg->getWGParam_prnScl() << endl;
  fd << "acscale=" << wg->getWGParam_accScl() << endl;
  fd << "N=" << wg->getTotalNumberOfStates() << " L=" << wg->getTotalNumberOfArcs() << endl;
  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {
    State *st = wg->getState(i);
    fd << "I=" << i; //st->getID();
    if (st->getIDword()!=-1) fd << "\tW=" << wg->getWord(st->getIDword());
    fd << "\tt=" << showpoint << st->getTimeStamp()/wg->getWGParam_tmScl() << endl;
  }
  uint n=0;
  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {
    State *st = wg->getState(i);
    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc* arc = wg->getArc(st->getOUTArcID(j));
      fd << "J=" << n++ << "\tS=" << wg->mapStateIDToTrueLoc(arc->iSource);
      fd << "\tE=" << wg->mapStateIDToTrueLoc(arc->iTarget);
      if (arc->iIdWord!=-1) fd << "\tW=" << wg->getWord(arc->iIdWord);
      fd << "\tv=" << arc->iPronunc;
      fd << "\ta=" << arc->dAcProb;
      fd << "\tl=" << arc->dLmProb;
      fd << "\tr=" << arc->dPrProb << endl;
    }
  }
}


/* Print the WG to outstream in DOT format */
void WGFile::print_DOT(WG *wg, ostream & fd) {

  // Determine the best path in the WG
  set<uint> bestPathSt, bestPathArc;
  int stID = wg->getInitialState();
  while (stID!=wg->getFinalState()) {
    bestPathSt.insert(stID);
    //cerr << bestPath.size() << " " << stID << " " << wg->getFinalState() << endl;
    State *st=wg->getState(stID);
    int arcID = st->getMaxOUTArcID();
    if (arcID<0) {bestPathSt.clear(); bestPathArc.clear(); break;}
    bestPathArc.insert(arcID);
    Arc *arc = wg->getArc(arcID);
    stID = arc->iTarget;
  }

  //fd << "digraph \"" << wg->getName() << "\" {" << endl;
  fd << "digraph {" << endl;
  fd << "fontname=\"Helvetica\"" << endl; 
  fd << "fontsize=24" << endl;
  fd << "label=\"" << wg->getName() << "\";" << endl;
  //fd << "size=\"10,7.5\";" << endl;
  //fd << "size=\"8.5,11\";" << endl;
  //fd << "size=\"8.5\";" << endl;
  fd << "ratio=auto;" << endl;
  fd << "rankdir=LR;" << endl;
  //fd << "orientation=landscape;" << endl;
  fd << "orientation=portrait;" << endl;
  fd << "node [shape=ellipse, fontsize=24, fontname=\"Helvetica\", style=\"setlinewidth(2)\"];" << endl;
  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {
    State *st = wg->getState(i);

    if (st->getID()==wg->getInitialState()) {
        fd << '"' << st->getID() << '"';
        fd << " [style=filled,color=\".7 .3 1.0\",label=\"";
        fd << st->getID() << "\\nt=" << st->getTimeStamp() << "\"];" << endl;
    } else if (st->getID()==wg->getFinalState()) {
        fd << '"' << st->getID() << '"' << " [style=filled,color=\".7 .3 1.0\",label=\"";
        fd << st->getID() << "\\nt=" << st->getTimeStamp() << "\"];" << endl;
    } else {
      if (bestPathSt.count(st->getID())>0) {
        fd << '"' << st->getID() << '"' << " [label=\"";
        fd << st->getID() << "\\nt=" << st->getTimeStamp() << "\",color=\"red\", style=\"setlinewidth(8)\"];" << endl;
      } else {
        fd << '"' << st->getID() << '"' << " [label=\"";
        fd << st->getID() << "\\nt=" << st->getTimeStamp() << "\"];" << endl;
      }
    }
    
    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc* arc = wg->getArc(st->getOUTArcID(j));
      fd << '"' << arc->iSource << "\"\t->\t\"";
      fd << arc->iTarget << "\"\t" << "[label=\"\\\"";
      //if (arc->IdWord()!=-1) {
      fd << wg->getWord(arc->iIdWord) << "\\\" / (" << arc->dPostProb << ")\", fontsize=24, arrowsize=2, fontname=\"Helvetica\",";
      if (bestPathArc.count(st->getOUTArcID(j))>0)
        fd << "style=\"setlinewidth(8)\",color=\"red\",fontcolor=\"red\"];" << endl;
      else
        fd << "style=\"setlinewidth(2)\"];" << endl;
      //fd << arc->dProb << ")\"];" << endl;
    }
  }
  fd << "}" << endl;
}

/* Print the WG in TEST format */
void WGFile::print_TEST(WG *wg, ostream & fd) {
  fd.setf(ios::fixed); // set number of decimals to 2
  fd << "Name=\"" << wg->getName() << "\"" << endl;
  fd << "#Nodes=" << wg->getTotalNumberOfStates() << " #Arcs=" << wg->getTotalNumberOfArcs() << endl;
  fd << "InitialNode=" << wg->getInitialState() << " FinalNode=" << wg->getFinalState() << endl;

  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {

    State *st = wg->getState(i);
    fd << "\nNode ID=" << st->getID();
    if (st->getIDword()!=-1)
      fd << "\tW=" << wg->getWord(st->getIDword()) << "(" << st->getPronunc() << ")";
    fd << "\tt=" << st->getTimeStamp() << endl;
    fd << "Forward-Prob = " << st->getForwardProb();
    fd << "\tBackward-Prob = " << st->getBackwardProb() << endl;

    fd << "  #InpArcs=" << st->getNumOfInputArcs() << endl;
    for (uint j=0; j<st->getNumOfInputArcs(); j++) {
      Arc* arc = wg->getArc(st->getINArcID(j));
      fd << "    Arc ID=" << arc->iId << "\tIdx=" << arc->iIdxInp << "\tS=" << arc->iSource;
      fd << "\tE=" << arc->iTarget;
      if (arc->iIdWord!=-1)
	fd << "\tW=" << wg->getWord(arc->iIdWord) << "(" << arc->iPronunc << ")";
      fd << "\tProbT=" << showpoint << setprecision(3) << arc->dProb;
      fd << "\tPostProb=" << arc->dPostProb;
      fd << "\tConfMeas=" << computeConfMeasure(*wg,arc->iId);
      fd << "\tProbAcc=" << st->getINArcProbAcc(j);
      if (st->getINArcID(j) == st->getMaxINArcID()) fd << " *" << endl;
      else fd << endl;
    }

    fd << "  #OutArcs=" << st->getNumOfOutputArcs() << endl;    
    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc* arc = wg->getArc(st->getOUTArcID(j));
      fd << "    Arc ID=" << arc->iId << "\tIdx=" << arc->iIdxOut << "\tS=" << arc->iSource;
      fd << "\tE=" << arc->iTarget;
      if (arc->iIdWord!=-1)
	fd << "\tW=" << wg->getWord(arc->iIdWord) << "(" << arc->iPronunc << ")";
      fd << "\tProbT=" << arc->dProb;
      fd << "\tPostProb=" << arc->dPostProb;
      fd << "\tConfMeas=" << computeConfMeasure(*wg,arc->iId);
      fd << "\tProbAcc=" << st->getOUTArcProbAcc(j);
      if (st->getOUTArcID(j) == st->getMaxOUTArcID()) fd << " *" << endl;
      else fd << endl;
    }
  }
}

/* Print the WG related information */
void WGFile::print_WGStat(WG *wg, ostream & fd) {
  fd.setf(ios::fixed); // set number of decimals to 2
  fd << "    Total Number of States: " << wg->getTotalNumberOfStates() << endl;
  fd << "      Total Number of Arcs: " << wg->getTotalNumberOfArcs() << endl;
  fd << "    Total Number of Frames: " << wg->getNumberOfFrames() << endl;
  fd << "           Vocabulary Size: " << wg->getVocSize() << endl;
  uint uMxInpDegr = 0, uMxOutDegr = 0;
  double dBrnFctPerWrd = 0.0;
  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {
    State *st = wg->getState(i);
    uint uwd = st->getNumOfInputArcs();
    if (uwd>uMxInpDegr) uMxInpDegr = uwd;
    uwd = st->getNumOfOutputArcs();
    if (uwd>uMxOutDegr) uMxOutDegr = uwd;
    set<int> sDifWrds;
    for (uint j=0; j<uwd; j++) {
      Arc *arc = wg->getArc(st->getOUTArcID(j));
      sDifWrds.insert(arc->iIdWord);
    }
    if (uwd) dBrnFctPerWrd += (double)uwd/sDifWrds.size();
  }
  fd << " Maximum Node Input Degree: " << uMxInpDegr << endl;
  fd << "Maximum Node Output Degree: " << uMxOutDegr << endl;
  fd << "Av-Brn-Fct Per Word & Node: " << showpoint << dBrnFctPerWrd/wg->getTotalNumberOfStates() << endl;
  fd << "        Av-#Arcs per Frame: " << showpoint << wg->getEdgesPerFrame() << endl;
  //wg->compNumOfDepartingPaths();
  //State *stI = wg->getState(wg->getInitialState());
  //fd << "     Total Number of Paths: " << scientific << stI->getNumOfDepartingPaths() << endl << endl;
  wg->compNumOfArrivingPaths();
  State *stF = wg->getState(wg->getFinalState());
  fd << "     Total Number of Paths: " << scientific << stF->getNumOfArrivingPaths() << endl << endl;
  fd << "# ---------------------------------------------" << endl;
  fd << "# ID-St\t#I-Arcs\t#O-Arcs\tTime\tNote" << endl;
  fd << "# ---------------------------------------------" << endl;
  for (uint i=0; i<wg->getTotalNumberOfStates(); i++) {
    State *st = wg->getState(i);
    fd << "   " << st->getID() << "\t" << st->getNumOfInputArcs();
    fd << "\t" << st->getNumOfOutputArcs();
    fd << "\t" << st->getTimeStamp();
    if (st->getID()==wg->getInitialState()) fd << "\t" << "Initial-State" << endl;
    else if (st->getID()==wg->getFinalState()) fd << "\t" << "Final-State" << endl;
    else fd << endl;
  }
  fd << "# ---------------------------------------------" << endl << endl;
  fd << "# ---------------------" << endl;
  fd << "# ID\tVoc-Word" << endl;
  fd << "# ---------------------" << endl;
  for (uint i=1; i<=wg->getVocSize(); i++)
    fd << "   " << i << "\t" << wg->getWord((int)i) << endl;
}

void WGFile::print_EdgesList(WG *wg, ostream & fd) {
  /* Just to check consistency of Post-Prob Matrix */
  //wg->checkPostProbMatrix(); return;
  fd << "# Num\tWord\tTi\tTf\tLkh\tPos-Prb\tCnf-Msr\tS-Time\tE-Time\tT-Time" << endl;
  fd << "# ----------------------------------------------------------------------------------------------" << endl;
  for (uint i=0; i<wg->getTotalNumberOfArcs(); i++) {
    double dMaxProb;
    uint bgT, fnT;
    Arc* arc = wg->getArc(i);
    fd << i << "\t";
    fd << wg->getWord(arc->iIdWord) << "\t";
    fd << wg->getState(arc->iSource)->getTimeStamp() << "\t";
    fd << wg->getState(arc->iTarget)->getTimeStamp() << "\t";
    fd << arc->dProb << "\t";
    fd << arc->dPostProb << "\t";
    computeArcMaxProb(*wg, arc->iId, dMaxProb, bgT, fnT);
    //fd << computeConfMeasure(*wg,arc->iId) << endl;
    fd << dMaxProb << "\t" << bgT << "\t" << fnT << "\t" << wg->getNumberOfFrames() << endl;
  }
}


void WGFile::print_LineWrdProb(WG *wg, ostream & fd) {
  fd << "# Num\tWord\tLine-PPrb" << endl;
  fd << "# ------------------------------------------------------------" << endl;
  for (uint i=1; i<=wg->getVocSize(); i++) {
    fd << i << "\t";
    fd << wg->getWord((int)i) << "\t";
    fd << computeLineWordProb(*wg,(int)i) << endl;
  }
}


void WGFile::print_ListOfWrdWithMaxProb(WG *wg, ostream & fd) {
  fd << "# Num\tWord\tPos-Prob\tS-Time\tE-Time\tT-Time" << endl;
  fd << "# ----------------------------------------------------------" << endl;
  for (uint v=1; v<=wg->getVocSize(); v++) {
    double dMaxProb;
    uint bgT, fnT;
    fd << v << "\t" << wg->getWord((int)v) << "\t";
    computeWordMaxProb(*wg, (int)v, dMaxProb, bgT, fnT);
    fd << dMaxProb << "\t" << bgT << "\t" << fnT << "\t" << wg->getNumberOfFrames() << endl;
  }
}


void WGFile::print_ListOfWrdWithExactProb(WG *wg, ostream & fd) {
  fd << "# Num\tWord\tPos-Prob(Exact)" << endl;
  fd << "# ---------------------------------------" << endl;
  for (uint v=1; v<=wg->getVocSize(); v++) {
    fd << v << "\t" << wg->getWord((int)v) << "\t";
    fd << computeWordExactProb(*wg, (int)v) << endl;
  }
}


void WGFile::print_TableOfWrdProb(WG *wg, ostream & fd) {
  fd << "# Num.\tWord\tPP-Fr0\tPP-Fr1\tPP-Fr2\t ...\tPP-Fr";
  fd << wg->getNumberOfFrames()-1 << endl;
  fd << "# ------------------------------------------------------------" << endl;
  fd << "VSZ=" << wg->getVocSize(); 
  fd << " NFR=" << wg->getNumberOfFrames() << endl;
  for (uint i=1; i<=wg->getVocSize(); i++) {
    fd << i << " " << wg->getWord((int)i);
    for (uint j=0; j<wg->getNumberOfFrames(); j++)
      fd << " " << wg->getPProbMatrix(j,i);
    fd << endl;
  }
}


void WGFile::print_GlobalWrdProb(WG *wg, ostream & fd) {
  fd << "# Num\tWord\tPos-Prob(Global)" << endl;
  fd << "# ---------------------------------------" << endl;
  //for (uint v=1; v<=wg->getVocSize(); v++) {
  //  fd << v << "\t" << wg->getWord((int)v) << "\t";
  //  fd << computeWordGlobalProb(*wg, (int)v) << endl;
  //}
  double *vConf = computeWordGlobalProb(*wg);
  for (uint v=1; v<=wg->getVocSize(); v++) {
    fd << v << "\t" << wg->getWord((int)v) << "\t";
    fd << vConf[v-1] << endl;
  }
  delete [] vConf;
}


void WGFile::print_MultiWrdProb(WG *wg, ostream & fd) {
  fd << "# Num Word N-bst PsPrb_1 Stime_1 Etime_1 ... PsPrb_N Stime_N Etime_N Ttime" << endl;
  fd << "# ------------------------------------------------------------------------------------" << endl;
  //for (uint v=1; v<=wg->getVocSize(); v++) {
  //  fd << v << "\t" << wg->getWord((int)v) << "\t";
  //  fd << computeWordGlobalProb(*wg, (int)v) << endl;
  //}
  vector<multiScore> *vConf = computeMultiWordMaxProb(*wg);
  for (uint v=1; v<=wg->getVocSize(); v++) {
    fd << v << " " << wg->getWord((int)v) << " " << vConf[v-1].size() << " ";
    for (uint n=0; n<vConf[v-1].size(); n++) {
      fd << vConf[v-1][n].dPostProb << " ";
      fd << vConf[v-1][n].sTime << " ";
      fd << vConf[v-1][n].fTime << " ";
    }
    fd << wg->getNumberOfFrames() << endl;
  }
  delete [] vConf;
}
