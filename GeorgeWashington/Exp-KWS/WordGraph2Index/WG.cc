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

//#include <string.h>

#include "WG.h"
#include "LogsRobust.h"

#include <string.h>

using std::endl;
using std::ios;



/////////////////////////////////////
// VOCABULARY METHODS
/////////////////////////////////////

/* Given the string word, this function returns its id */
int Vocabulary::wordToIndex(const string &word) {
  unordered_map<string, uint>::const_iterator it = mVoc.begin();
  it=mVoc.find(word);
  if (it!=mVoc.end()) return mVoc[word];
  return -1;
}

/* Given the id, this function returns the string word */
const string& Vocabulary::indexToWord(int id) {
  if (id<=(int)uVoc && id>0) return (*vWords[id]);
  return (*vWords[0]); // return lambda
}

/* Set lambda string to symbol */
void Vocabulary::setLambda(const string &lb) {
  //mVoc.clear();
  unordered_map<string, uint>::const_iterator it = mVoc.begin();
  it=mVoc.find(lb);
  if (it!=mVoc.end()) {
    cerr << "ERROR: There is already an existing \"" << lb << "\" string." << endl;
    return;
  } 
  it=mVoc.begin(); it=mVoc.find(sLambda); mVoc.erase(it);
  sLambda=lb;
  mVoc[sLambda]=0;
  it = mVoc.begin(); it=mVoc.find(sLambda);
  vWords[0]=&(it->first);
}

/* Add new entry to the map */
void Vocabulary::addVocMap(const string &str) {
  mVoc[str]=++uVoc;
  unordered_map<string,uint>::const_iterator it = mVoc.begin();
  it = mVoc.find(str);
  vWords.push_back(&(it->first)); // Insert string* into the vocabulary vector
}

/* Return vocabulary size */
uint Vocabulary::sizeVoc() {
  return uVoc;
}

/* Print out the whole vocabulary */
void Vocabulary::print(ostream & fd) {
  for (uint i=0; i<=uVoc; i++)
    fd << i << "\t" << *vWords[i] << endl;
}

/* Write Vocabulary into a binary file */
size_t Vocabulary::writeToBinFile(FILE *pFile) {
  size_t totalAmountSpace = 0;
  fwrite (&uVoc,sizeof(uint),1,pFile);
  totalAmountSpace += sizeof(uint);
  for (uint i=1; i<=uVoc; i++) {
    size_t l = vWords[i]->size();
    l++; // This includes '\0'
    fwrite (&l,sizeof(size_t),1,pFile);
    fwrite (vWords[i]->c_str(),1,l,pFile);
    totalAmountSpace += sizeof(size_t) + l;
  }
  return totalAmountSpace;
}

/* Read Vocabulary from a binary file */
void* Vocabulary::readFromMapMem(const void *ptr) {
  uint *puAux = (uint *)ptr;
  uVoc = *(puAux++);
  //cerr << "uVoc: " << uVoc << endl;
  size_t l, *pszAux = (size_t *)puAux;
  for (uint i=1; i<=uVoc; i++) {
    l = *(pszAux++);
    char *pcAux = (char *)pszAux; pszAux = (size_t *)(pcAux+l);
    string str(pcAux);
    //cerr << str << endl;
    mVoc[str]=i;
    unordered_map<string,uint>::const_iterator it = mVoc.begin();
    it = mVoc.find(str);
    vWords.push_back(&(it->first)); // Insert string* into 
                                    // the vocabulary vector
  }
  return (void *)pszAux;
}



/////////////////////////////////////
// STATE METHODS
/////////////////////////////////////

void State::setID(int id) { iId = id; }
int State::getID() { return iId; }

void State::setIDword(int idword) { iIdWord = idword; }
int State::getIDword() { return iIdWord; }

void State::setPronunc(int ipronunc) { iPronunc = ipronunc; }
int State::getPronunc() {return iPronunc; }

uint State::getNumOfInputArcs() { return uNumInpArcs; }
uint State::getNumOfOutputArcs() { return uNumOutArcs; }

int State::addArcID(int idArc, tarc ta) {
  VitProbs avp;
  avp.idArc=idArc;
  if (ta==IN) {
    vInpArcs.push_back(avp);
    return uNumInpArcs++; // return the index of the added input arc
  } else if (ta==OUT) {
    vOutArcs.push_back(avp);
    return uNumOutArcs++; // return the index of the added output arc
  }
  return -1;
}

int State::getINArcID(uint pos) {
  if (uNumInpArcs) return vInpArcs[pos].idArc;
  else return -1;
}

int State::getMaxINArcID() {
  if (uNumInpArcs==0 || iIdxMaxInp<0) return -1;
  return vInpArcs[iIdxMaxInp].idArc;
}

void State::setINArcProbAcc(double prob, uint pos) {
  vInpArcs[pos].dProbAcc = prob;
  if (iIdxMaxInp<0 || vInpArcs[iIdxMaxInp].dProbAcc<prob) iIdxMaxInp = pos;
}

double State::getINArcProbAcc(uint pos) {
  if (uNumInpArcs && pos<uNumInpArcs)
    return vInpArcs[pos].dProbAcc;
  else return 0.0;  // That is for the initial state
}

double State::getMaxINArcProbAcc() {
  if (uNumInpArcs) {
    if (iIdxMaxInp<0) return (NDINF);
    else return vInpArcs[iIdxMaxInp].dProbAcc;
  } 
  else return 0.0;  // That is for the initial state
}

int State::getOUTArcID(uint pos) {
  if (uNumOutArcs && pos<uNumOutArcs)
    return vOutArcs[pos].idArc;
  else return -1;
}

int State::getMaxOUTArcID() {
  if (uNumOutArcs==0 || iIdxMaxOut<0) return -1;
  return vOutArcs[iIdxMaxOut].idArc;
}

void State::setOUTArcProbAcc(double prob, uint pos) {
  vOutArcs[pos].dProbAcc = prob;
  if (iIdxMaxOut<0 || vOutArcs[iIdxMaxOut].dProbAcc<prob) iIdxMaxOut = pos;
}

double State::getOUTArcProbAcc(uint pos) {
  if (uNumOutArcs && pos<uNumOutArcs)
    return vOutArcs[pos].dProbAcc;
  else return 0.0;  // That is for the final state
}

double State::getMaxOUTArcProbAcc() {
  if (uNumOutArcs) {
    if (iIdxMaxOut<0) return (NDINF);
    return vOutArcs[iIdxMaxOut].dProbAcc;
  } 
  else return 0.0; // That is for the final state
}

void State::setTimeStamp(uint ts) { uTimeStamp = ts; }
uint State::getTimeStamp() { return uTimeStamp; }

void State::setForwardProb(double dProb) { dForwardProb=dProb; }
double State::getForwardProb() {
  if (uNumInpArcs) return dForwardProb;
  else return 0.0;  // That is for the initial state
}

void State::setBackwardProb(double dProb) { dBackwardProb=dProb; }
double State::getBackwardProb() {
  if (uNumOutArcs) return dBackwardProb;
  else return 0.0; // That is for the final state
}

void State::resetProb(char c) {
  if (c!='f') {
    iIdxMaxInp = iIdxMaxOut = -1;
    for (uint i=0; i<uNumInpArcs; i++) vInpArcs[i].dProbAcc = NDINF;
    for (uint i=0; i<uNumOutArcs; i++) vOutArcs[i].dProbAcc = NDINF;
    lNumInpPaths = lNumOutPaths = 0;
  }
  dForwardProb = dBackwardProb = NDINF;
}

void State::setNumOfArrivingPaths(ulint np) {
  if ( PDINF > np ) lNumInpPaths = np;
  else lNumInpPaths = PDINF;
}
ulint State::getNumOfArrivingPaths() { return lNumInpPaths; }

void State::setNumOfDepartingPaths(ulint np) {
  if ( PDINF > np ) lNumOutPaths = np;
  else lNumOutPaths = PDINF;
}
ulint State::getNumOfDepartingPaths() { return lNumOutPaths; }

size_t State::writeToBinFile(FILE *pFile) {
  // Save iId, iIdWord, uNumInpArcs, uNumOutArcs, uTimeStamp,
  // iIdxMaxInp, iIdxMaxOut, dForwardProb, dBackwardProb,
  // lNumInpPaths, lNumOutPaths
  size_t totalAmountSpace = 0;
  fwrite (&iId,sizeof(int),1,pFile);
  fwrite (&iIdWord,sizeof(int),1,pFile);
  fwrite (&iPronunc,sizeof(int),1,pFile);
  fwrite (&uNumInpArcs,sizeof(uint),1,pFile);
  fwrite (&uNumOutArcs,sizeof(uint),1,pFile);
  fwrite (&uTimeStamp,sizeof(uint),1,pFile);
  fwrite (&iIdxMaxInp,sizeof(int),1,pFile);
  fwrite (&iIdxMaxOut,sizeof(int),1,pFile);
  fwrite (&dForwardProb,sizeof(double),1,pFile);
  fwrite (&dBackwardProb,sizeof(double),1,pFile);
  fwrite (&lNumInpPaths,sizeof(ulint),1,pFile);
  fwrite (&lNumOutPaths,sizeof(ulint),1,pFile);
  totalAmountSpace += 5*sizeof(int) + 3*sizeof(uint) + 2*sizeof(double) + 2*sizeof(ulint);
  // Save input/output arcs information linked to Viterbi process
  for (uint i=0; i<uNumInpArcs; i++) {
    //fwrite (&(vInpArcs[i].idArc),sizeof(int),1,pFile);
    //fwrite (&(vInpArcs[i].dProbAcc),sizeof(double),1,pFile);
    //totalAmountSpace += sizeof(int) + sizeof(double);
    fwrite (&vInpArcs[i],sizeof(VitProbs),1,pFile);
    totalAmountSpace += sizeof(VitProbs);
  }
  for (uint i=0; i<uNumOutArcs; i++) {
    //fwrite (&(vOutArcs[i].idArc),sizeof(int),1,pFile);
    //fwrite (&(vOutArcs[i].dProbAcc),sizeof(double),1,pFile);
    //totalAmountSpace += sizeof(int) + sizeof(double);
    fwrite (&vOutArcs[i],sizeof(VitProbs),1,pFile);
    totalAmountSpace += sizeof(VitProbs);
  }
  return totalAmountSpace;
}

void* State::readFromMapMem(const void *ptr) {
  double *pdAux;
  uint *puAux;
  int *piAux;
  ulint *plAux;
  // Read iId, iIdWord, uNumInpArcs, uNumOutArcs, uTimeStamp,
  // iIdxMaxInp, iIdxMaxOut, dForwardProb, dBackwardProb,
  // lNumInpPaths, lNumOutPaths
  //fread (&iId,sizeof(int),1,pFile);
  piAux = (int *)ptr;
  iId = *(piAux++);
  //cerr << "State id: " << iId << endl;
  //fread (&iIdWord,sizeof(int),1,pFile);
  iIdWord = *(piAux++);
  //fread (&iPronunc,sizeof(int),1,pFile);
  iPronunc = *(piAux++);
  //fread (&uNumInpArcs,sizeof(uint),1,pFile);
  puAux = (uint *)piAux;
  uNumInpArcs = *(puAux++);
  //fread (&uNumOutArcs,sizeof(uint),1,pFile);
  uNumOutArcs = *(puAux++);
  //fread (&uTimeStamp,sizeof(uint),1,pFile);
  uTimeStamp = *(puAux++);
  //fread (&iIdxMaxInp,sizeof(int),1,pFile);
  piAux = (int *)puAux;
  iIdxMaxInp = *(piAux++);
  //fread (&iIdxMaxOut,sizeof(int),1,pFile);
  iIdxMaxOut = *(piAux++);
  //fread (&dForwardProb,sizeof(double),1,pFile);
  pdAux = (double *)piAux;
  dForwardProb = *(pdAux++);
  //fread (&dBackwardProb,sizeof(double),1,pFile);
  dBackwardProb = *(pdAux++);
  //fread (&lNumInpPaths,sizeof(ulint),1,pFile);
  plAux = (ulint *)pdAux;
  lNumInpPaths = *(plAux++);
  //fread (&lNumOutPaths,sizeof(ulint),1,pFile);
  lNumOutPaths = *(plAux++);
  // Read input/output arcs information linked to Viterbi process
  //piAux = (int *)plAux;
  VitProbs *pvpAux = (VitProbs *)plAux;
  vInpArcs.assign(pvpAux,pvpAux+uNumInpArcs);
  pvpAux += uNumInpArcs;
  /*
  for (uint i=0; i<uNumInpArcs; i++) {
    VitProbs avp; vInpArcs.push_back(avp);
    //fread (&vInpArcs[i].idArc,sizeof(int),1,pFile);
    //vInpArcs[i].idArc = *(piAux++); pdAux = (double *)piAux;
    //fread (&vInpArcs[i].dProbAcc,sizeof(double),1,pFile);
    //vInpArcs[i].dProbAcc = *(pdAux++); piAux = (int *)pdAux;
    vInpArcs[i] = *(pvpAux++);
  }*/
  vOutArcs.assign(pvpAux,pvpAux+uNumOutArcs);
  pvpAux += uNumOutArcs;
  /*
  for (uint i=0; i<uNumOutArcs; i++) {
    VitProbs avp; vOutArcs.push_back(avp);
    //fread (&vOutArcs[i].idArc,sizeof(int),1,pFile);
    //vOutArcs[i].idArc = *(piAux++); pdAux = (double *)piAux;    
    //fread (&vOutArcs[i].dProbAcc,sizeof(double),1,pFile);
    //vOutArcs[i].dProbAcc = *(pdAux++); piAux = (int *)pdAux;
    vOutArcs[i] = *(pvpAux++);
  }*/

  //return (void *)piAux;
  return (void *)pvpAux;
}




/////////////////////////////////////
// WG METHODS
/////////////////////////////////////

void WG::setName(string & str) { name = str; }
const string & WG::getName() { return name; }

// These were defined as inline in the header
//uint WG::getTotalNumberOfStates() const { return uNumStates; }
//uint WG::getTotalNumberOfArcs() const { return uNumArcs; }

void WG::setInitialState(int iIs) { 
  iInitialState = iIs;
  if (iFinalState!=-1 && uNumFrames==0) {
    State *sti = getState(iInitialState);
    uint uTi = sti->getTimeStamp();
    State *stf = getState(iFinalState);
    uint uTf = stf->getTimeStamp();
    if ((uTf-uTi)<=0) {
      cerr << "WARNING: Initial time \"" << uTi << "\"";
      cerr << " is greater/equal than final time: \"" << uTf << "\"." << endl;
    }
    uNumFrames = uTf - uTi;
  }
}
int WG::getInitialState() const { return iInitialState; }


void WG::setFinalState(int iFs) {
  iFinalState = iFs;
  if (iInitialState!=-1 && uNumFrames==0) {
    State *sti = getState(iInitialState);
    uint uTi = sti->getTimeStamp();
    State *stf = getState(iFinalState);
    uint uTf = stf->getTimeStamp();
    if ((uTf-uTi)<=0) {
      cerr << "WARNING: Initial time \"" << uTi << "\"";
      cerr << " is greater/equal than final time: \"" << uTf << "\"." << endl;
    }
    uNumFrames = uTf - uTi;
  }
}
int WG::getFinalState() const { return iFinalState; }


// This was defined as inline in the header
//uint WG::getNumberOfFrames() const { return uNumFrames; }

void WG::addState(State & st, const string & word) {
  if (getState(st.getID())) { // In case the state already exists
    //State *nst = getState(st.getID());
    //nst->print_Test();
    //cerr << "WARNING: State: " << st.getID() << " already exists" << endl; 
    return; 
  }
  if (!word.empty()) {
    int iAux=wgVoc.wordToIndex(word);
    if (iAux==-1) {
      wgVoc.addVocMap(word);
      st.setIDword(wgVoc.wordToIndex(word));
    } else st.setIDword(iAux);
  }
  vStates.push_back(st);
  uNumStates=vStates.size();
  mapStatesStorage[st.getID()]=uNumStates-1;
}


State * WG::getState(uint uSt) {
  if (uSt<uNumStates) return &vStates[uSt];
  return NULL;
}


State * WG::getState(int iSt) {
  if (iSt<0) return NULL;
  unordered_map<int,uint>::const_iterator it = mapStatesStorage.begin();
  it=mapStatesStorage.find(iSt);
  if (it!=mapStatesStorage.end()) return getState(mapStatesStorage[iSt]);
  return NULL;
}


bool WG::addArc(Arc & arc, const string & word, bool isDummy) {
  if (getArc(arc.iId)) { // In case the arc already exists
    //Arc *narc = getArc(arc.iId);
    //narc->print_Test();
    //cerr << "WARNING: Arc: " << arc.iId << " already exists" << endl; 
    return true;
  }
  State *sti, *sto;
  if ((sti=getState(arc.iTarget)) && \
      (sto=getState(arc.iSource))) {
    arc.iIdxInp = sti->addArcID(arc.iId,IN);
    arc.iIdxOut = sto->addArcID(arc.iId,OUT);
    arc.dProb = (isDummy)?0.0:(( arc.dAcProb * accScl + arc.dPrProb * prnScl + \
                                 arc.dLmProb * lmScl  + wIPen ) * wgDisp);
    if (!word.empty()) {
      int iAux=wgVoc.wordToIndex(word);
      if (iAux==-1) {
	wgVoc.addVocMap(word);
	arc.iIdWord=wgVoc.wordToIndex(word);
      } else arc.iIdWord=iAux;
    } else if (sti->getIDword()!=-1) {
      // In case the word label was located in the target node
      arc.iIdWord = sti->getIDword();
      arc.iPronunc = sti->getPronunc();
    } else {
      cerr << "ERROR: Word label is not defined in state: " << sti->getID() << endl;
      return false;
    }
    //arc.print_Test(); exit(-1);
    vArcs.push_back(arc);
    uNumArcs=vArcs.size();
    mapArcsStorage[arc.iId]=uNumArcs-1;
    return true;
  }
  cerr << "ERROR: There is a lacking state ..." << endl;
  return false;
}


Arc * WG::getArc(uint uArc) {
  if (uArc<getTotalNumberOfArcs())
    return &vArcs[uArc];
  return NULL;
}


Arc * WG::getArc(int iArc) {
  if (iArc<0) return NULL;
  unordered_map<int,uint>::const_iterator it = mapArcsStorage.begin();
  it=mapArcsStorage.find(iArc);
  if (it!=mapArcsStorage.end()) return getArc(mapArcsStorage[iArc]);
  return NULL;
}


uint WG::getVocSize() { return wgVoc.sizeVoc(); }
int WG::addWord(const string &word) {
  int iAux = -1;
  if (!word.empty()) {
    iAux=wgVoc.wordToIndex(word);
    if (iAux==-1) { wgVoc.addVocMap(word); iAux=wgVoc.wordToIndex(word); } 
  }
  return iAux;
}
const string & WG::getWord(int idword) { return wgVoc.indexToWord(idword); }
int WG::getWordID(const string &word) { return wgVoc.wordToIndex(word); }


uint WG::mapStateIDToTrueLoc(int iStID) {
  // WARNING: If iStID does not match the key of any element in the 
  // container, the function throws an out_of_range exception.
  return mapStatesStorage.at(iStID);
}


uint WG::mapArcIDToTrueLoc(int iArcID) {
  // WARNING: If iArcID does not match the key of any element in the 
  // container, the function throws an out_of_range exception.
  return mapArcsStorage.at(iArcID);
}


void WG::deepFirstSearch(uint v, uint &n, uint *R, stack<uint> &P) {
  n++; R[v]=n;
  State *st = getState(v);
  for (uint i=0; i<st->getNumOfOutputArcs(); i++) {
    const Arc *arc = getArc(st->getOUTArcID(i));
    uint w = mapStatesStorage.at(arc->iTarget);
    if (R[w]==0) deepFirstSearch(w,n,R,P);
  }
  P.push(v);
}


const uint* WG::getTopologicalOrder() {
  if (vTO) return vTO;
  uint n=0, *R = new uint[getTotalNumberOfStates()];
  for (uint v=0; v<getTotalNumberOfStates(); v++) R[v]=0;
  stack<uint> P;
  
  for (uint v=0; v<getTotalNumberOfStates(); v++) 
    if (R[v]==0) deepFirstSearch(v,n,R,P);

  uint *Rinv = new uint[getTotalNumberOfStates()];
  n=0;
  while(!P.empty()) {
    //State *st = getState(P.top());
    //cerr << P.top() << " " << st->getTimeStamp() << endl;
    R[n++] = P.top(); P.pop();
    Rinv[R[n-1]] = n-1;
  }
  cerr << endl;

  // This is for dtecting cycles in the WG
  for (uint v=0; v<getTotalNumberOfStates(); v++) {
    State *st = getState(R[v]);
    for (uint i=0; i<st->getNumOfOutputArcs(); i++) {
      const Arc *arc = getArc(st->getOUTArcID(i));
      uint w = mapStatesStorage.at(arc->iTarget);
      if (Rinv[w]<=v) {
      	cerr << 
	//Rinv[w] << " " << v << " " <<
	"WARNING: A Cycle has been detected in the imput WG: J=" << arc->iId << endl;
      }
    }
  }
  delete [] Rinv;

  return (vTO=R);
}


/* By default search for the path with maximum likelihood.
 * However with srchMax sets to false search for the path
 * with minimum likelihood, but with its sign reversed. */
void WG::viterbiForward(bool srchMax) {

  const uint* vTopOrder = getTopologicalOrder();

  for (uint i=0; i<getTotalNumberOfStates(); i++) {
    
    State *st = getState(vTopOrder[i]);
      
    // The getArcInpIDProbAcc() takes into account whether this
    // state is initial or not
    double maxProb=st->getMaxINArcProbAcc();

    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc *arc = getArc(st->getOUTArcID(j));
      State *nextSt = getState(arc->iTarget);
      nextSt->setINArcProbAcc(maxProb+(srchMax?1:-1)*arc->dProb,arc->iIdxInp);
    }
  }
}


void WG::forward() {

  const uint* vTopOrder = getTopologicalOrder();

  for (uint i=0; i<getTotalNumberOfStates(); i++) {
    
    State *st = getState(vTopOrder[i]);
      
    // The getForwardProb() takes into account whether this
    // state is initial or not
    double fProb=st->getForwardProb();

    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc *arc = getArc(st->getOUTArcID(j));
      State *nextSt = getState(arc->iTarget);
      nextSt->setForwardProb(sumLogs(nextSt->getForwardProb(),fProb+arc->dProb));
    }
  }
}


void WG::viterbiBackward() {

  const uint* vTopOrder = getTopologicalOrder();

  for (uint j=getTotalNumberOfStates(); j>0; j--) {
    
    State *st = getState(vTopOrder[j-1]);
      
    // The getMaxOUTArcProbAcc() takes into account whether this
    // state is finalal or not
    double maxProb=st->getMaxOUTArcProbAcc();

    for (uint j=0; j<st->getNumOfInputArcs(); j++) {
      const Arc *arc = getArc(st->getINArcID(j));
      State *prevSt = getState(arc->iSource);
      prevSt->setOUTArcProbAcc(maxProb+arc->dProb,arc->iIdxOut);
    }
  }
}


void WG::backward() {

  const uint* vTopOrder = getTopologicalOrder();

  for (uint j=getTotalNumberOfStates(); j>0; j--) {
    
    State *st = getState(vTopOrder[j-1]);
      
    // The getBackwardProb() takes into account whether this
    // state is final or not
    double bProb=st->getBackwardProb();

    for (uint j=0; j<st->getNumOfInputArcs(); j++) {
      const Arc *arc = getArc(st->getINArcID(j));
      State *prevSt = getState(arc->iSource);
      prevSt->setBackwardProb(sumLogs(prevSt->getBackwardProb(),bProb+arc->dProb));
    }
  }
}


void WG::compPostProb() {

  forward();
  backward();
  
  // State *initSt = getState(getInitialState());
  // double totalProb = initSt->getBackwardProb();
  State *finalSt = getState(getFinalState());
  double totalProb = finalSt->getForwardProb();

  for (uint i=0; i<getTotalNumberOfArcs(); i++) {
    Arc *arc = getArc(i);
    State *prevSt = getState(arc->iSource);
    State *nextSt = getState(arc->iTarget);
    arc->dPostProb = ProbLogFloat2ProbFloat(prevSt->getForwardProb() + arc->dProb + nextSt->getBackwardProb() - totalProb);
    // Second way to normalize arc probabilities: \sum_i st.arc-out[i] = 1
    //arc->dPostProb = ProbLogFloat2ProbFloat(arc->dProb + nextSt->getBackwardProb() - prevSt->getBackwardProb());
  }

}


double WG::normProbWhtRespTotalProb(double dProb) {

  State *initSt = getState(getInitialState());
  double totalProb = initSt->getBackwardProb();

  return ProbLogFloat2ProbFloat(dProb - totalProb);
}


void WG::buildPostProbMatrix() {

  uint uNF = getNumberOfFrames();
  uint uVS = getVocSize();
  double *dM = (double*)calloc(uNF*uVS, sizeof(double));
  uint uEdgesFr=0;

//#pragma omp parallel for
  for (uint i=0; i<getTotalNumberOfArcs(); i++) {
    Arc* arc = getArc(i);
    State *st1 = getState(arc->iSource);
                                 // Discount the time stamp of the initial state
    uint t1 = st1->getTimeStamp() - getState(getInitialState())->getTimeStamp();
    State *st2 = getState(arc->iTarget);
                                 // Discount the time stamp of the initial state
    uint t2 = st2->getTimeStamp() - getState(getInitialState())->getTimeStamp();

    if (t1 == t2)
      cerr << "WARNING: Initial and end time stamps of the Arc are the same: " << arc->iId << endl;

    uint v = arc->iIdWord - 1; // As arc->iIdWord=0 is for lambda symbol
//#pragma omp critical
    {
      for (uint t=t1; t<t2; t++) dM[t+v*uNF] += arc->dPostProb; 
      uEdgesFr += (t2-t1)>0?t2-t1:0;
    }
  }

  dEdgesPerFrame = (double)uEdgesFr/uNF; // Set Edges per Frame
  dPostProbMatrix = dM;
}


double WG::getPProbMatrix(uint time, int idWord) {

  if (!dPostProbMatrix) buildPostProbMatrix();
                // Discount the time stamp of the initial state
  time = time - getState(getInitialState())->getTimeStamp();
  uint uNF = getNumberOfFrames();
  uint uVS = getVocSize();
  uint idW = idWord - 1; // Because idWord=1 is in pos=0 now
                         // v=0 corresponds to the Lambda ("") 
                         // symbol in the vocabulary

  //cerr << "\nNF: " << uNF << "  Word :" << getWord(idWord) << " idWord: " << idWord << " idW: " << idW << endl;
  //exit(-1);

  if ( (time+idW*uNF) < (uNF*uVS) ) return dPostProbMatrix[time+idW*uNF];

  cerr << "ERROR: Matrix addressing is out of range !";
  exit(-1);
}


void WG::checkPostProbMatrix() {

  if (!dPostProbMatrix) buildPostProbMatrix();
  uint uNF = getNumberOfFrames();
  uint uVS = getVocSize();
  uint tInit = getState(getInitialState())->getTimeStamp();

  for (uint t=0; t<uNF; t++) {
    double sumScr=0.0;
    for (uint v=0; v<uVS; v++)
      sumScr += dPostProbMatrix[t+v*uNF];
    cerr << "Time: " << t + tInit << " CS: " << sumScr << endl;
  }
}


void WG::compNumOfArrivingPaths() {

  const uint* vTopOrder = getTopologicalOrder();

  // Initialize number of arriving paths to the initial state to 1
  State *stI = getState(getInitialState());
  stI->setNumOfArrivingPaths(1);

  for (uint i=0; i<getTotalNumberOfStates(); i++) {
    
    State *st = getState(vTopOrder[i]);
      
    // The getArcInpIDProbAcc() takes into account whether this
    // state is initial or not
    ulint ulnp=st->getNumOfArrivingPaths();

    for (uint j=0; j<st->getNumOfOutputArcs(); j++) {
      Arc *arc = getArc(st->getOUTArcID(j));
      State *nextSt = getState(arc->iTarget);
      nextSt->setNumOfArrivingPaths(nextSt->getNumOfArrivingPaths()+ulnp);
    }
  }

}


void WG::compNumOfDepartingPaths() {

  const uint* vTopOrder = getTopologicalOrder();

  // Initialize number of departing paths from the final state to 1
  State *stF = getState(getFinalState());
  stF->setNumOfDepartingPaths(1);

  for (uint j=getTotalNumberOfStates(); j>0; j--) {
    
    State *st = getState(vTopOrder[j-1]);
      
    // The getArcOutIDProbAcc() takes into account whether this
    // state is initial or not
    ulint ulnp=st->getNumOfDepartingPaths();

    for (uint j=0; j<st->getNumOfInputArcs(); j++) {
      Arc *arc = getArc(st->getINArcID(j));
      State *prevSt = getState(arc->iSource);
      prevSt->setNumOfDepartingPaths(prevSt->getNumOfDepartingPaths()+ulnp);
    }
  }

}


double WG::getEdgesPerFrame() {
  if (!dPostProbMatrix) buildPostProbMatrix();
  return dEdgesPerFrame;
}


void WG::resetProb() {
  for (uint i=0; i<getTotalNumberOfStates(); i++) {
    State *st = getState(i);
    st->resetProb();
  }
  for (uint i=0; i<getTotalNumberOfArcs(); i++) {
    Arc *arc = getArc(i);
    arc->dPostProb=NDINF;
  }
  if (dPostProbMatrix) free(dPostProbMatrix); dPostProbMatrix=NULL;
}


size_t WG::writeToBinFile(FILE *pFile) {

  size_t totalAmountSpace = 0;
  // Reserve space for the total storage space
  fwrite (&totalAmountSpace,sizeof(size_t),1,pFile);

  // Save lmScl wip prnScl accScl wgDisp
  fwrite (&lmScl,sizeof(float),1,pFile);
  fwrite (&wIPen,sizeof(float),1,pFile);
  fwrite (&prnScl,sizeof(float),1,pFile);
  fwrite (&accScl,sizeof(float),1,pFile);
  fwrite (&wgDisp,sizeof(float),1,pFile);
  fwrite (&tmScl,sizeof(float),1,pFile);
  totalAmountSpace += 6*sizeof(float);

  // Save Vocabulary  
  totalAmountSpace += wgVoc.writeToBinFile(pFile);

  // Save WG name, uNumStates, uNumArcs, iInitialState, iFinalState
  // and uNumFrames
  size_t l = name.size(); l++; // This includes '\0'
  fwrite (&l,sizeof(size_t),1,pFile);
  fwrite (name.c_str(),1,l,pFile);
  fwrite (&uNumStates,sizeof(uint),1,pFile);
  fwrite (&uNumArcs,sizeof(uint),1,pFile);
  fwrite (&iInitialState,sizeof(int),1,pFile);
  fwrite (&iFinalState,sizeof(int),1,pFile);
  fwrite (&uNumFrames,sizeof(uint),1,pFile);
  totalAmountSpace += sizeof(size_t) + l + 3*sizeof(uint) + 2*sizeof(int); 

  // Save States
  for (uint i=0; i<uNumStates; i++) totalAmountSpace += vStates[i].writeToBinFile(pFile);

  // Save Arcs
  for (uint i=0; i<uNumArcs; i++) totalAmountSpace += vArcs[i].writeToBinFile(pFile);

  // Save topological order Vector vTO
  fwrite (vTO,sizeof(uint),uNumStates,pFile);
  totalAmountSpace += uNumStates*sizeof(uint);

  // Save MAP's mapStatesStorage (uNumStates) and mapArcsStorage (uNumArcs)
  for (unordered_map<int,uint>::const_iterator it=mapStatesStorage.begin(); \
       it!=mapStatesStorage.end(); it++) {
    fwrite (&it->first,sizeof(int),1,pFile);
    fwrite (&it->second,sizeof(uint),1,pFile);
    totalAmountSpace += sizeof(int) + sizeof(uint);
  }
  for (unordered_map<int,uint>::const_iterator it=mapArcsStorage.begin(); \
       it!=mapArcsStorage.end(); it++) {
    fwrite (&it->first,sizeof(int),1,pFile);
    fwrite (&it->second,sizeof(uint),1,pFile);
    totalAmountSpace += sizeof(int) + sizeof(uint);
  }

  // Go back to the beginning of the file and write the total storage space
  rewind (pFile);
  fwrite (&totalAmountSpace,sizeof(size_t),1,pFile);

  return totalAmountSpace;
}

void WG::readFromBinFile(FILE *pFile) {

  void *pvBase, *pvAux;
  float *pfAux;
  char *pcAux;
  uint *puAux;
  int *piAux;
  size_t *pszAux;

  cerr.setf(ios::fixed); cerr.precision(2); // set number of decimals to 2

  size_t totalAmountSpace = 0;
  fread (&totalAmountSpace,sizeof(size_t),1,pFile);
  cerr << "INFO: Total storage space of WG: " << totalAmountSpace << endl;

  pvBase = malloc(totalAmountSpace);
  fread (pvBase,1,totalAmountSpace,pFile);
  
  // Read lmScl wip prnScl accScl wgDisp tmScl
  pfAux = (float *)pvBase;
  
  lmScl  = *(pfAux); cerr << "INFO: Grammar Scale Factor: " << *(pfAux++) << endl;
  wIPen  = *(pfAux); cerr << "INFO: Word Insertion Penalty: " <<  *(pfAux++) << endl;
  prnScl = *(pfAux); cerr << "INFO: Pronunciation Scale Factor: " << *(pfAux++) << endl;
  accScl = *(pfAux); cerr << "INFO: Morphologic Scale Factor: " << *(pfAux++) << endl;
  wgDisp = *(pfAux); cerr << "INFO: Log Base Scale Factor: " << *(pfAux++) << endl;
  tmScl  = *(pfAux); cerr << "INFO: Time Scale Factor: " << *(pfAux++) << endl;

  // Read Vocabulary  
  pvAux = wgVoc.readFromMapMem(pfAux);
  //cerr << "pvAux: " << pvAux << endl;

  // Read WG name, uNumStates, uNumArcs, iInitialState, iFinalState
  // and uNumFrames
  size_t l;
  pszAux = (size_t *)pvAux;
  l = *(pszAux++);
  //cerr << "Len of WG's name: " << l << endl;
  pcAux = (char *)pszAux; name = pcAux;
  //cerr << name << endl;
  puAux = (uint *)(pcAux + l);
  uNumStates = *(puAux++);
  uNumArcs = *(puAux++);
  //cerr << "nS:" << uNumStates << " nA:" << uNumArcs << endl;
  piAux = (int *)puAux;
  iInitialState = *(piAux++);
  iFinalState = *(piAux++);
  //cerr << "IS:" << iInitialState << " FS:" << iFinalState << endl;
  puAux = (uint *)piAux;
  uNumFrames = *(puAux++);
  //cerr << "NF:" << uNumFrames << endl;
  
  // Read States
  pvAux = (void *)puAux;
  for (uint i=0; i<uNumStates; i++) {
    State st; vStates.push_back(st);
    pvAux = vStates[i].readFromMapMem(pvAux);
  }

  // Read Arcs
  for (uint i=0; i<uNumArcs; i++) {
    Arc arc; vArcs.push_back(arc);
    //pvAux = vArcs[i].readFromMapMem(pvAux);
    pvAux = vArcs.back().readFromMapMem(pvAux);
  }

  // Read topological order Vector vTO
  vTO = new uint[uNumStates];
  memcpy (vTO, pvAux, sizeof(uint)*uNumStates);
  //for (uint i=0; i<uNumStates; i++) cerr << "vTO[" << i << "]=" << vTO[i] << endl;
  puAux = (uint *)pvAux; puAux+=uNumStates;

  // Read MAP's mapStatesStorage (uNumStates) and mapArcsStorage (uNumArcs)
  piAux = (int *)puAux;
  for (uint i=0; i<uNumStates; i++) {
    //int iId; uint uId;
    //iId = *(piAux++); puAux = (uint *)piAux;
    //uId = *(puAux++); piAux = (int *)puAux;
    //mapStatesStorage[iId]=uId;
    mapStatesStorage.insert(std::make_pair(*piAux,*((uint *)(piAux+1))));
    puAux = (uint *)++piAux; piAux = ((int *)++puAux);
  }
  for (uint i=0; i<uNumArcs; i++) {
    //int iId; uint uId;
    //iId = *(piAux++); puAux = (uint *)piAux;
    //uId = *(puAux++); piAux = (int *)puAux;
    //mapArcsStorage[iId]=uId;
    mapArcsStorage.insert(std::make_pair(*piAux,*((uint *)(piAux+1))));
    puAux = (uint *)++piAux; piAux = ((int *)++puAux);
  }

  //pvAux = (void *)piAux;
  //cerr << pvAux << " " << pvBase + totalAmountSpace << endl;
  free(pvBase);
}
