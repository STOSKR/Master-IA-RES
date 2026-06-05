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

#ifndef WG_H
#define WG_H

#include <limits>
#define NDINF (-(std::numeric_limits<double>::max())) /* Lowest double value */
#define PDINF ((std::numeric_limits<double>::max()))  /* Largest double value */

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>

typedef unsigned short int usint;
typedef unsigned int uint;
typedef long double ulint;
enum tarc { IN, OUT }; /* Type of ARC: IN for input, OUT for output */

using std::ostream;
using std::string;
using std::cerr;
using std::endl;
using std::vector;
//using std::map;
using std::unordered_map;
using std::stack;

/* Class to store the vocabulary of the WG */
class Vocabulary {
  uint uVoc;                        /* Index to the last inserted word of the 
				       vocabulary. 0 is for the empty string */
  string sLambda;                   /* Symbol for the empty string */
  //map<string,uint> mVoc;          /* map of words and their keys */
  unordered_map<string,uint> mVoc;  /* map of words and their keys */
  vector<const string*> vWords;     /* To store the vocabulary */

 public:
  /* Constructor */
  Vocabulary(): uVoc(0), sLambda("") {
    mVoc[sLambda]=uVoc;   // Map empty string to the index 0
    unordered_map<string,uint>::const_iterator it = mVoc.begin();
    it = mVoc.find(sLambda);
    vWords.push_back(&(it->first)); // Insert empty string into the vocabulary vector
  }
  
  /* Given the string word, this function returns its id */
  int wordToIndex(const string &word);

  /* Given the id, this function returns the string word */
  const string& indexToWord(int id);

  /* Set lambda string to symbol */
  void setLambda(const string &lb);

  /* Add new entry to the map */
  void addVocMap(const string &str);

  /* Return vocabulary size */
  uint sizeVoc();

  /* Print out the whole vocabulary */
  void print(ostream &);

  /* Write Vocabulary into a binary file */
  size_t writeToBinFile(FILE *pFile);
  /* Read Vocabulary from a mapped memory */
  void* readFromMapMem(const void *ptr);
};


/* Class to represent an arc in the grammar */
class Arc {
 public:
  int iId;           /* Arc identifier */
  int iSource;       /* Id of the source state */
  int iIdxOut;       /* Index to the state output arc list */
  int iTarget;       /* Id of the target state */
  int iIdxInp;       /* Index to the state input arc list */ 
  int iIdWord;       /* Id of the label of the arc */
  int iPronunc;      /* Pronunciation variant number */
  double dAcProb;    /* Morphologic probability (lg) */
  double dPrProb;    /* Pronunciation probability (lg) */
  double dLmProb;    /* Language Model probability (lg) */
  double dProb;      /* Total Probability (lg): dAcProb + dLmProb */
  double dPostProb;  /* Posterior Probability */
  usint uVisited;    /* General marking flag for the Arc */

  Arc(): iId(-1), iSource(-1), iIdxOut(-1), iTarget(-1), iIdxInp(-1), iIdWord(-1), iPronunc(-1), dAcProb(0.0), dPrProb(0.0), dLmProb(0.0), dProb(NDINF), dPostProb(NDINF), uVisited(0) {};

  //bool operator==(const Arc &a) {
  //  return (iIdWord==a.iIdWord && (iPronunc==a.iPronunc) && iSource==a.iSource && iTarget==a.iTarget);
  //};

  /* Partial copy of the Arc atributtes */
  void cpArcPart(const Arc &a) {
    iId=a.iId; iSource=a.iSource;
    iIdxOut=-1; iTarget=a.iTarget;
    iIdxInp=-1; iIdWord=-1; iPronunc=a.iPronunc;
    dAcProb=a.dAcProb; dPrProb=a.dPrProb; dLmProb=a.dLmProb;
    dProb=NDINF; dPostProb=a.dPostProb;
  };

  void print_Test() {
    cerr << "ArcId: " << iId << " iSource:" << iSource;
    cerr << " iIdxOut:" << iIdxOut << " iTarget:" << iTarget;
    cerr << " iIdxInp:" << iIdxInp << " iIdWord:" << iIdWord;
    cerr << " iPronunc:" << iPronunc;
    cerr << " dAcProb:" << dAcProb << " dPrProb:" << dPrProb << " dLmProb:" << dLmProb;
    cerr << " dProb:" << dProb << " dPostProb:"  << dPostProb << endl;
  }

  size_t writeToBinFile(FILE *pFile) {
    fwrite (&iId,sizeof(int),1,pFile);
    fwrite (&iSource,sizeof(int),1,pFile);
    fwrite (&iIdxOut,sizeof(int),1,pFile);
    fwrite (&iTarget,sizeof(int),1,pFile);
    fwrite (&iIdxInp,sizeof(int),1,pFile);
    fwrite (&iIdWord,sizeof(int),1,pFile);
    fwrite (&iPronunc,sizeof(int),1,pFile);
    fwrite (&dAcProb,sizeof(double),1,pFile);
    fwrite (&dPrProb,sizeof(double),1,pFile);
    fwrite (&dLmProb,sizeof(double),1,pFile);
    fwrite (&dProb,sizeof(double),1,pFile);
    fwrite (&dPostProb,sizeof(double),1,pFile);
    return (7*sizeof(int) + 5*sizeof(double));
  }
  void* readFromMapMem(const void *ptr) {
    double *pdAux;
    int *piAux;
    //fread (&iId,sizeof(int),1,pFile);
    piAux = (int *)ptr;
    iId = *(piAux++);
    //cerr << "Arc id: " << iId << endl;
    //fread (&iSource,sizeof(int),1,pFile);
    iSource = *(piAux++);
    //fread (&iIdxOut,sizeof(int),1,pFile);
    iIdxOut = *(piAux++);
    //fread (&iTarget,sizeof(int),1,pFile);
    iTarget = *(piAux++);
    //fread (&iIdxInp,sizeof(int),1,pFile);
    iIdxInp = *(piAux++);
    //fread (&iIdWord,sizeof(int),1,pFile);
    iIdWord = *(piAux++);
    //fread (&iPronunc,sizeof(int),1,pFile);
    iPronunc = *(piAux++);
    //fread (&dAcProb,sizeof(double),1,pFile);
    pdAux = (double *)piAux;
    dAcProb = *(pdAux++);
    //fread (&dPrProb,sizeof(double),1,pFile);
    dPrProb = *(pdAux++);
    //fread (&dLmProb,sizeof(double),1,pFile);
    dLmProb = *(pdAux++);
    //fread (&dProb,sizeof(double),1,pFile);
    dProb = *(pdAux++);
    //fread (&dPostProb,sizeof(double),1,pFile);
    dPostProb = *(pdAux++);
    
    return (void *)pdAux;
  }
};


/* Class to represent a state in the grammar */
class State {
 private:

  typedef struct vp {
    int idArc;
    double dProbAcc;
    vp() {idArc=-1; dProbAcc=NDINF;}
  } VitProbs;

  int iId;                    /* State identifier */
  int iIdWord;                /* Id of the word stored in this state */
  int iPronunc;               /* Pronunciation variant number */
  uint uNumInpArcs;           /* Number of arcs that arrive to this state */
  vector<VitProbs> vInpArcs;  /* Arc Ids that arrive to this state */
  uint uNumOutArcs;           /* Number of arcs that leave from this state */
  vector<VitProbs> vOutArcs;  /* Arcs Ids that leave from this state */
  uint uTimeStamp;            /* Time stamp of the node */ 

  int iIdxMaxInp;             /* Index to the input with max dProbAcc 
                                 vInpArcs vector */ 
  int iIdxMaxOut;             /* Index to the output with max dProbAcc
                                 vOutArcs vector */ 
  
  double dForwardProb;        /* Store the total forward probability */
  double dBackwardProb;       /* Store the total backward probability */

  ulint lNumInpPaths;         /* Store the total number of input paths */
  ulint lNumOutPaths;         /* Store the total number of output paths */


 public:
  State(): iId(-1), iIdWord(-1), iPronunc(-1), uNumInpArcs(0), uNumOutArcs(0), uTimeStamp(0), iIdxMaxInp(-1), iIdxMaxOut(-1), dForwardProb(NDINF), dBackwardProb(NDINF), lNumInpPaths(0), lNumOutPaths(0) {};

  /* Partial copy of the State atributtes */
  void cpStatePart(const State &s) {
    iId=s.iId; iIdWord=-1; iPronunc=s.iPronunc;
    uNumInpArcs=0; uNumOutArcs=0;
    uTimeStamp=s.uTimeStamp; iIdxMaxInp=-1;
    iIdxMaxOut=-1; dForwardProb=NDINF;
    dBackwardProb=NDINF;
    lNumInpPaths=0; lNumOutPaths=0;
  };

  void print_Test() {
    cerr << "iId=" << iId << " iIdWord=" << iIdWord << " iPronunc=" << iPronunc << endl;
    cerr << "uNumInpArcs=" << uNumInpArcs << " uNumOutArcs=" << uNumOutArcs << endl;
    cerr << "uTimeStamp=" << uTimeStamp << endl; 
    cerr << "iIdxMaxInp=" << iIdxMaxInp << " iIdxMaxOut=" << iIdxMaxOut << endl;
    cerr << "dForwardProb=" << dForwardProb << " dBackwardProb=" << dBackwardProb << endl;
    cerr << "lNumInpPaths=" << lNumInpPaths << endl; 
    cerr << "lNumOutPaths=" << lNumOutPaths << endl; 
  };

  void setID(int id);
  int getID();

  void setIDword(int idword);
  int getIDword();

  void setPronunc(int ipronunc);
  int getPronunc();

  uint getNumOfInputArcs();
  uint getNumOfOutputArcs();

  int addArcID(int idArc, tarc ta);

  int getINArcID(uint pos);
  int getMaxINArcID();

  void setINArcProbAcc(double prob, uint pos);
  double getINArcProbAcc(uint pos);
  double getMaxINArcProbAcc();

  int getOUTArcID(uint pos);
  int getMaxOUTArcID();

  void setOUTArcProbAcc(double prob, uint pos);
  double getOUTArcProbAcc(uint pos);
  double getMaxOUTArcProbAcc();

  void setTimeStamp(uint ts);
  uint getTimeStamp();

  void setForwardProb(double dProb);
  double getForwardProb();
  void setBackwardProb(double dProb);  
  double getBackwardProb();

  void resetProb(char c='\0'); /* Reset dForwardProb, dBackwardProb, iIdxMaxInp, 
  				  vInpArcs.dProbAcc, lNumInpPaths, iIdxMaxOut, 
				  vOutArcs.dProbAcc and lNumOutPaths. 
                                  if c='f' only dForwardProb and dBackwardProb
				  probabilities are reset. */

  void setNumOfArrivingPaths(ulint np);
  ulint getNumOfArrivingPaths();

  void setNumOfDepartingPaths(ulint np);
  ulint getNumOfDepartingPaths();

  size_t writeToBinFile(FILE *pFile);    /* Write the State into a binary file */
  void* readFromMapMem(const void *ptr); /* Read the State from a mapped memory */
};



/* WG Class */ 
class WG {  
 private:
  string name;                   /* WG Name */
  uint uNumStates;               /* Number of states (total) */
  uint uNumArcs;                 /* Number of arcs (total) */
  vector<State> vStates;         /* states[] */
  vector<Arc> vArcs;             /* arcs[] */

  Vocabulary wgVoc;              /* Vocabulary of the WG */

  int iInitialState;             /* Initial state iId */
  int iFinalState;               /* Final state iId */
  uint uNumFrames;		 /* Number of (frames) Feat. Vectors */
  uint* vTO;                     /* State sequence in topological order */

  double dEdgesPerFrame;         /* Store the average number of WG Edges per 
                                    Frame and per Word */

  float wgDisp;                  /* WG Dispersion factor */
  float lmScl;                   /* Language model scale factor */
  float wIPen;                   /* Word Insertion penalty */
  float prnScl;                  /* Pronunciation scale factor */
  float accScl;                  /* Morphologic scale factor */
  float tmScl;                   /* Time scale factor */

  /* This is used by setTopologicalOrder() function */  
  void deepFirstSearch(uint v, uint &n, uint *R, stack<uint> &P); 

  /* To map State and Arc IDs to their true locations into vStates and vArcs */
  unordered_map<int, uint> mapStatesStorage, mapArcsStorage;

  /* Build the post-probability matrix storing for each frame and each 
     WG's vocabulary word the corresponding max post prob */
  void buildPostProbMatrix();

 protected:
  /* Compute the forward and backward probabilities through the whole WG
     NOTE: This let's invoque these methods in the inherited classes */
  void forward();
  void backward();
       
  double* dPostProbMatrix;	 /* Store the WG's Post-Probability by frame and idWord */

 public:
  /* Constructor */ 
  WG(): name(""), uNumStates(0), uNumArcs(0), iInitialState(-1), \
  iFinalState(-1), uNumFrames(0), vTO(NULL), dEdgesPerFrame(0), \
  wgDisp(1.0), lmScl(1.0), wIPen(0.0), prnScl(1.0), accScl(1.0), tmScl(100.0), \
  dPostProbMatrix(NULL) {}
  
  /* Destructor: Close the WG file */
  ~WG() { if (vTO) delete [] vTO; 
          if (dPostProbMatrix) free(dPostProbMatrix);
	}

  void setName(string & str);
  const string & getName();

  /* Set grammar scale factor, word insertion penalty, 
   * accustic scale factor, WG dispersion factor and
   * time sclae factor */
  void setWGParameters(float lmsf=1.0, float wip=0.0, float psf=1.0, \
                       float asf=1.0, float wgd=1.0, float tscl=100.0) {
    lmScl = lmsf; wIPen = wip; prnScl = psf; accScl = asf; wgDisp = wgd; tmScl = tscl;
  }
  /* The following get WG dispersion factor, grammar scale factor, 
   * word insertion penalty, pronunciation scale factor, morphologic
   * scale factor and time scale factor */
  inline float getWGParam_WgDisp() { return wgDisp; }
  inline float getWGParam_lmScl() { return lmScl; }
  inline float getWGParam_wIPen() { return wIPen; }
  inline float getWGParam_prnScl() { return prnScl; }
  inline float getWGParam_accScl() { return accScl; }
  inline float getWGParam_tmScl() { return tmScl; }

  inline uint getTotalNumberOfStates() const { return uNumStates; }
  inline uint getTotalNumberOfArcs() const { return uNumArcs; }

  void setInitialState(int iIs);
  int getInitialState() const;
  void setFinalState(int iFs);
  int getFinalState() const;
  inline uint getNumberOfFrames() const { return uNumFrames; }

  void addState(State & st, const string & word = "");
  State * getState(uint uSt); /* Return State pointer using the State storage index */
  State * getState(int iSt);  /* Return State pointer using the original ID of the State */

  bool addArc(Arc & arc, const string & word = "",  bool isDummy = false);
  Arc * getArc(uint uArc);  /* Return Arc pointer using the Arc storage index */
  Arc * getArc(int iArc);   /* Return Arc pointer using the original ID of the Arc */

  uint getVocSize();	    /* Return the WG vocabulary size */
  int addWord(const string &word);    /* Add new word to the WG lexicon */
  const string & getWord(int idword); /* Get the STRING of the given word ID */ 
  int getWordID(const string &word);  /* Get the word ID of the given STRING */

  uint mapStateIDToTrueLoc(int iStID);  /* To map State IDs to their true 
     					   locations into vStates */

  uint mapArcIDToTrueLoc(int iArcID);   /* To map Arc IDs to their true 
     					   locations into vArcs */

  const uint* getTopologicalOrder();    /* Obtain the WG topological order */

  /* By default search for the path with maximum likelihood.
   * However with srchMax sets to false search for the path
   * with minimum likelihood, but with its sign reversed. */
  void viterbiForward(bool srchMax = true);
  void viterbiBackward();

  /* To compute the posterior probability of each WG arc */
  void compPostProb();

  /* To compute the normalized probability of dProb respect to the total
     Forward (or Backward) WG probability */
  double normProbWhtRespTotalProb(double dProb);

  /* Get the Ppst-Prob of word (iwith idWowrd) at (frame) time */
  double getPProbMatrix(uint time, int idWord);

  /* Check consistency of Post-Prob Matrix */
  void checkPostProbMatrix();

  /* Compute the total number of different paths arriving to each WG node */
  void compNumOfArrivingPaths();

  /* Compute the total number of different paths departing from each WG node */
  void compNumOfDepartingPaths();

  /* Return the average number of WGedges per Frame */
  double getEdgesPerFrame();

  /* Reset WG probabilities: Posterior, Backward, Forward, Viterbi-back-forward */
  void resetProb();
  
  /* Write the WG into a binary file through serialization */
  size_t writeToBinFile(FILE *pFile);

  /* Read the WG from a binary file */
  void readFromBinFile(FILE *pFile);
};

#endif
