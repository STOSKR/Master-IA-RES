/*****************************************************************************/
/*! \author  Alejandro H. Toselli <ahector@prhlt.upv.es>
 *  \version 10.0
 *  \date    2014
 */

/* Copyright (C) 2014 by Pattern Recognition and Human Language
   Technology Group, Technological Institute of Computer Science,
   Valencia University of Technology, Valencia (Spain).

   Permission to use, copy, modify, and distribute this software and
   its documentation for any purpose and without fee is hereby
   granted, provided that the above copyright notice appear in all
   copies and that both that copyright notice and this permission
   notice appear in supporting documentation.  This software is
   provided "as is" without express or implied warranty.
*/

/* Compile order */
/* g++ -Wall -O3 -o smoothTabPosProbs smoothTabPosProbs.cc WG.cc \
   -pedantic -ansi -std=gnu++0x -Iutils -Lutils -lUtils -fopenmp */

#include "WG.h"
#include "LevDistance.h"
#include "LogsRobust.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using std::showpoint;
using std::setprecision;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;


Vocabulary vocab;
double* dPstProb=NULL;
int *iLvDst=NULL;


void usage(char * nomProg, char cMt, unsigned short sufNum, double se, double ee, uint ne) {
  cerr << endl;
  cerr << "Usage: " << nomProg << " [options] <Table-file> <Voc-file>" << endl << endl;
  cerr << "  Mandatory Args: <Table-file> <Voc-file> are Post-Prob table and "<< endl;
  cerr << "                  Vocabulary input files" << endl << endl;
  cerr << "  Options:\n" << endl;
  cerr << "     -m <n|g|e|t>                  Smooth type: n (Normal)\n";
  cerr << "                                   g (Good Turing) e (Err-Corr)\n";
  cerr << "                                   t (Entropy)  (def.: " << cMt << ")" << endl;
  cerr << "     -n <int>                      ID to be added in the suffix\n";
  cerr << "                                   of output file names (def.: " << sufNum << ")" << endl;
  cerr << "     -s \"<float> <float> <int>\"    Starting, Ending and Num. of\n";
  cerr << "                                   Epsilon values (def.: " << showpoint << setprecision(2) << se << " " << ee << " " << ne << ")" << endl;
  cerr << "     -h                            This help" << endl;
  cerr << endl << endl;
}


bool readTabOfPosProbs(istream & ifd, uint &numFram, uint &vocSize) {
  const uint sz=1024;
  char cad[sz];
  uint uNF=0, uVS=0;
  while (ifd.getline(cad,sz)) {
    uint i=0;
    while (i<sz && cad[i]==' ') i++;
    if (cad[i]=='#') continue;
    strtok(cad," \t=");
    uVS=(uint)atoi(strtok(NULL," \t="));
    strtok(NULL," \t=");
    uNF=(uint)atoi(strtok(NULL," \t="));
    break;
  }

  if (ifd.fail() || ifd.bad() || ifd.eof()) return true;

  if (!(dPstProb = new double[uNF*uVS])) return true;

  uint cnt=0;
  int itk; string wtk; double dtk;
  for (uint v=0; v<uVS; v++) {
    ifd >> itk >> wtk;
    vocab.addVocMap(wtk);
    for (uint t=0; t<uNF; t++) {
      ifd >> dtk;
      dPstProb[cnt++]=dtk;
    }
  }
  numFram=uNF; vocSize=uVS;
  return false;
}


uint readVocabulary(istream & ifd) {
  uint uVS=0;
  const uint sz=1024;
  char cad[sz];
  while (ifd.getline(cad,sz)) {
    char* p;
    for (p=cad; *p!='\0' && *p==' '; p++);
    if (*p=='#') continue;
    string wtk = strtok(p," \t");
    int iAux=vocab.wordToIndex(wtk);
    if (iAux==-1) vocab.addVocMap(wtk);
  }

  uVS=vocab.sizeVoc();
  return uVS;
}


bool compLevDistance(uint uVS) {

  iLvDst = new int[(uVS*(uVS-1))/2];
  if (!iLvDst) return true;

  for (uint i=0; i<uVS-1; i++) {
    const char *s = vocab.indexToWord((int)i+1).c_str();
    for (uint j=i+1; j<uVS; j++) {
      const char *t = vocab.indexToWord((int)j+1).c_str();
      iLvDst[j+i*(uVS-1)-(((i+1)*i)>>1)-1]=levenshtein_distance(s,t);
    }
  }
  return false;
}


inline int getLevDistance(uint i, uint j, uint uVS) {
  if (i==j) return 0;
  if (i>j) { uint k=j; j=i; i=k; }
  return iLvDst[j+i*(uVS-1)-(((i+1)*i)>>1)-1];
}


double compAlphOpt(uint uWrdID, uint frame, uint uWGVS, uint uVS, uint uNF, double eps) {
  double dMax=0.0, dAux;
  for (uint i=0; i<uWGVS; i++)
    if (dPstProb[frame+i*uNF]>=eps) {
      dAux = dPstProb[frame+i*uNF]/(getLevDistance(uWrdID,i,uVS)+1);
      if (dAux>dMax) dMax=dAux;
    }
  return dMax;
}


double compPstPrbOpt(uint uWrdID, uint frame, uint uWGVS, uint uVS, uint uNF, double gamm) {
  double dMax=minLog, dAux;
  for (uint i=0; i<uWGVS; i++) {
    //dAux = pow(dPstProb[frame+i*uNF],1-gamm)/pow(getLevDistance(uWrdID,i,uVS)+1,gamm);
    dAux = ProbFloat2ProbLogFloat(dPstProb[frame+i*uNF])*(1-gamm) - getLevDistance(uWrdID,i,uVS)*gamm;
    if (dAux>dMax) dMax=dAux;
  }
  return ProbLogFloat2ProbFloat(dMax);
}


double compEntropy(uint frame, uint uWGVS, uint uNF) {
  double dEntr=0.0;
  uint uN=0;
  for (uint i=0; i<uWGVS; i++)
    if (dPstProb[frame+i*uNF]>0.0) {
      uN++;
      dEntr = dEntr - dPstProb[frame+i*uNF]*ProbFloat2ProbLogFloat(dPstProb[frame+i*uNF])/ProbFloat2ProbLogFloat(2);
    }
  if (!uN) return 0.0;
  //return ProbFloat2ProbLogFloat(2)/ProbFloat2ProbLogFloat(uWGVS)*dEntr;
  return ProbFloat2ProbLogFloat(2)/ProbFloat2ProbLogFloat(uN)*dEntr;
}




bool compPstProbSmthNormal(double* dPPS, uint uWGVS, uint uVS, uint uNF, double eps) {

  double *dA = new double[uVS];
  if (!dA || !dPPS) return false;
  
  // Computation  
  for (uint j=0; j<uNF; j++) {
    double dEPS=0.0, totALPH=0.0;
    for (uint i=0; i<uWGVS; i++) {
      if (dPstProb[j+i*uNF]>=eps) {
	dPPS[j+i*uNF]=dPstProb[j+i*uNF]-eps;
	dEPS+=eps;
      } else {
	dPPS[j+i*uNF]=-1.0;
	dEPS+=dPstProb[j+i*uNF];
	double dAux=compAlphOpt(i,j,uWGVS,uVS,uNF,eps);
	dA[i]=dAux;
	totALPH+=dAux;
      }
    }     
    for (uint i=uWGVS; i<uVS; i++) {
      dPPS[j+i*uNF]=-1.0;
      double dAux=compAlphOpt(i,j,uWGVS,uVS,uNF,eps);
      dA[i]=dAux;
      totALPH+=dAux;
    }

    // In case dPstProb[j+i*uNF]<eps for all i (totALPH=0.0)
    if (totALPH==0.0) {
      for (uint i=0; i<uVS; i++)
        if (dPPS[j+i*uNF]<0.0) dPPS[j+i*uNF]=dEPS/uVS;
    } else
      for (uint i=0; i<uVS; i++)
        if (dPPS[j+i*uNF]<0.0) dPPS[j+i*uNF]=dA[i]/totALPH*dEPS;
  }

  delete [] dA;
  return true;
}


bool compPstProbSmthGTuring(double* dPPS, uint uWGVS, uint uVS, uint uNF, double eps) {

  double *dA = new double[uVS];
  if (!dA || !dPPS) return false;
  
  // Computation  
  for (uint j=0; j<uNF; j++) {
    double dEPS=0.0, totALPH=0.0;
    for (uint i=0; i<uWGVS; i++) {
      if (dPstProb[j+i*uNF]>=eps) dPPS[j+i*uNF]=dPstProb[j+i*uNF];
      else {
	dPPS[j+i*uNF]=-1.0;
	dEPS+=dPstProb[j+i*uNF];
	double dAux=compAlphOpt(i,j,uWGVS,uVS,uNF,eps);
	dA[i]=dAux;
	totALPH+=dAux;
      }
    }     
    for (uint i=uWGVS; i<uVS; i++) {
      dPPS[j+i*uNF]=-1.0;
      double dAux=compAlphOpt(i,j,uWGVS,uVS,uNF,eps);
      dA[i]=dAux;
      totALPH+=dAux;
    }

    // In case dPstProb[j+i*uNF]<eps for all i (totALPH=0.0)
    if (totALPH==0.0) {
      for (uint i=0; i<uVS; i++)
        if (dPPS[j+i*uNF]<0.0) dPPS[j+i*uNF]=dEPS/uVS;
    } else
      for (uint i=0; i<uVS; i++)
        if (dPPS[j+i*uNF]<0.0) dPPS[j+i*uNF]=dA[i]/totALPH*dEPS;
  }

  delete [] dA;
  return true;
}


bool compPstProbWithErrCorr(double* dPPS, uint uWGVS, uint uVS, uint uNF, double gamm) {

  if (!dPPS) return false;

  // Computation of EC for all samples, including those that are in the WG 
#pragma omp parallel for
  for (uint j=0; j<uNF; j++) 
    for (uint i=0; i<uVS; i++)
      dPPS[j+i*uNF]=compPstPrbOpt(i,j,uWGVS,uVS,uNF,gamm);    

/* Just for Samples which are not in the WG
#pragma omp parallel for
  for (uint j=0; j<uNF; j++) 
#pragma omp critical
    for (uint i=0; i<uWGVS; i++) dPPS[j+i*uNF]=dPstProb[j+i*uNF];
#pragma omp critical
    for (uint i=uWGVS; i<uVS; i++)
      dPPS[j+i*uNF]=compPstPrbOpt(i,j,uWGVS,uVS,uNF,gamm);    
  }
*/
  return true;
}


bool compPstProbWithEntropy(double* dPPS, uint uWGVS, uint uVS, uint uNF, double gamm) {

  for (uint j=0; j<uNF; j++) {
    double dEntrp = compEntropy(j,uWGVS,uNF);
    //cerr << j << " " << dEntrp << endl;
    for (uint i=0; i<uWGVS; i++) dPPS[j+i*uNF]=dPstProb[j+i*uNF]*(1-dEntrp);
  }     
  return true;
}




void computeWordMaxProb(double* dPPS, uint idW, uint uNF, double &dMaxProb, uint &sTime, uint &fTime) {
  uint bgTm, fnTm;
  double scrMax = 0.0;
  uint t = bgTm = fnTm = 0;
  bool bEndTime = false;
  while (t < uNF) {
    double scr = dPPS[t+idW*uNF];
    if (scrMax<scr) { scrMax = scr; bgTm = fnTm = t; bEndTime = false; }
    else if (scrMax==scr && !bEndTime) fnTm = t;
    else bEndTime = true;
    t++;
  }
  dMaxProb=scrMax; sTime=bgTm; fTime=fnTm;
}


void printWrdListMaxProb(ostream &fd, double *dPPS, uint uVS, uint uNF, double eps) {
  fd << "# Smoothed with Epsilon = " << eps << endl;
  for (uint i=0; i<uVS; i++) {
    double dMaxProb;
    uint bgT, fnT;
    computeWordMaxProb(dPPS,i,uNF,dMaxProb,bgT,fnT);
    fd << vocab.indexToWord((int)i+1) << " ";
    fd << dMaxProb << "\t" << bgT << "\t" << fnT << "\t" << uNF << endl;
  }
}


int main(int argc, char ** argv) {
  
  char *prog;
  int option;
  if ((prog=rindex(argv[0],'/'))) prog+=1;
  else prog=argv[0];
  
  char *itfname=NULL, *ivfname=NULL;
  ifstream *itfd=NULL, *ivfd=NULL;
  ofstream *ofd=NULL;

  char *sInterv=NULL, cMethod='n';
  double dBgnEps=0.0;
  double dEndEps=0.5;
  unsigned short int uStpEps=1, sufNum=0;

  while ((option=getopt(argc,argv,"m:n:s:h"))!=-1)
    switch (option) {
    case 'm':
      cMethod=*optarg;
      break;
    case 'n':
      sufNum=atoi(optarg);
      break;
    case 's':
      sInterv=optarg;
      break;
    case 'h':
    default:
      usage(prog,cMethod,sufNum,dBgnEps,dEndEps,uStpEps);
      return 1;
    }

  if (argc - optind == 2) {
    itfname=argv[optind++];
    itfd=new ifstream(itfname);
    if (!itfd->is_open()) {
      cerr << "ERROR: File \"" << itfname << "\" could not be open." << endl;
      return 1;
    }
    ivfname=argv[optind++];
    ivfd=new ifstream(ivfname);
    if (!ivfd->is_open()) {
      cerr << "ERROR: File \"" << ivfname << "\" could not be open." << endl;
      return 1;
    }
  } else {
    usage(prog,cMethod,sufNum,dBgnEps,dEndEps,uStpEps);
    return 1;
  }

  if (sInterv) {
    char *tk = strtok(sInterv," ");
    dBgnEps = atof(tk);
    tk = strtok(NULL," ");
    dEndEps = atof(tk);
    tk = strtok(NULL," ");
    uStpEps = atoi(tk);
    tk = strtok(NULL," ");
    if (tk) {
      cerr << "SYNTAX ERROR: Deteted an extra argument !";
      usage(prog,cMethod,sufNum,dBgnEps,dEndEps,uStpEps);
      return 1;
    }
  }

  // cout << dBgnEps << " " << dEndEps << " " << uStpEps << endl;
  // return 0;

  uint uWGVS, uNF;
  if (readTabOfPosProbs(*itfd,uNF,uWGVS)) return 1;
  
  uint uVS;
  if (!(uVS=readVocabulary(*ivfd))) return 1;

  // cout << uWGVS << " " << uNF << endl;
  // for (uint i=0; i<uWGVS; i++) {
  //   cout << i+1 << " " << vocab.indexToWord((int)i+1);
  //   for (uint j=0; j<uNF; j++) cout << " " << dPstProb[j+i*uNF];
  //   cout << endl;
  // }

  // cout << uVS-uWGVS << endl;
  // for (uint i=uWGVS; i<uVS; i++)
  //   cout << i+1 << " " << vocab.indexToWord((int)i+1) << endl;

  if (compLevDistance(uVS)) return 1;

  // cout << uVS << " " << uVS << endl;
  // for (uint i=0; i<uVS; i++) {
  //   for (uint j=0; j<uVS; j++) {
  //     cout << vocab.indexToWord((int)i+1) << " ";
  //     cout << vocab.indexToWord((int)j+1) << " ";
  //     cout << getLevDistance(i,j,uVS) << endl;
  //   }
  // }

  // Output useful information
  cerr << "\nProcessing file: " << itfname << endl;
  if (cMethod=='t') cerr << "Post-Prob weighted with WG Entropy per frame" << endl;
  else if (cMethod=='e') cerr << "Smoothing type: " << "Error Correcting" << endl;
  else cerr << "Smoothing type: " << ((cMethod=='n')?"Normal":"Good-Turing") << endl;
  cerr << "#Words in/out WGS: " << uWGVS << "/" << uVS-uWGVS << " (" << uVS << ")" << endl;
  cerr << "#Frames: " << uNF << endl;

  char ofname[512], auxname[512];
  strcpy(auxname,itfname);
  char* p=rindex(auxname,'.'); if (p) *p='\0';
  double* dPPS = new double[uVS*uNF];
  //ostream *auxfd = (ofd==NULL)?&cout:ofd;

  double eps=dBgnEps, dIncr=(dEndEps-dBgnEps)/uStpEps;
  bool (* compMethod)(double*, uint, uint, uint, double) = NULL;
  if (cMethod=='n') compMethod = compPstProbSmthNormal;
  else if (cMethod=='g') compMethod = compPstProbSmthGTuring;
  else if (cMethod=='e') compMethod = compPstProbWithErrCorr;
  else if (cMethod=='t') compMethod = compPstProbWithEntropy;
  else {
    usage(prog,'n',sufNum,dBgnEps,dEndEps,uStpEps);
    return 1;
  }

  if (cMethod=='t') {
    if (!compMethod(dPPS,uWGVS,uVS,uNF,0.0)) return 1;

    sprintf(ofname,"%s_ENTRP.lst",auxname);
    cerr << "\tFile: " << ofname << endl;
    ofd=new ofstream(ofname);
    if (!ofd->is_open()) {
      cerr << "ERROR: File \"" << ofname << "\" could not be open." << endl;
      return 1;
    }

    printWrdListMaxProb(*ofd,dPPS,uWGVS,uNF,0.0);
    if (ofd) delete ofd;
  } else
    while (eps<dEndEps) {
      if (cMethod!='e') cerr << "Interval Epsilon applied: " << eps;
      else cerr << "Interval Gamma applied: " << eps;
      if (!compMethod(dPPS,uWGVS,uVS,uNF,eps)) return 1;
      
      // cout << uWGVS << " " << uVS << " " << uNF << endl;
      // for (uint i=0; i<uVS; i++) {
      //   cout << i+1 << " " << vocab.indexToWord((int)i+1);
      //   for (uint j=0; j<uNF; j++) cout << " " << dPPS[j+i*uNF];
      //   cout << endl;
      // }
      
      sprintf(ofname,"%s_SMTH-%02d.lst",auxname,sufNum);
      cerr << "\t\tFile: " << ofname << endl;
      ofd=new ofstream(ofname);
      if (!ofd->is_open()) {
	cerr << "ERROR: File \"" << ofname << "\" could not be open." << endl;
	return 1;
      }
      
      printWrdListMaxProb(*ofd,dPPS,uVS,uNF,eps);
      if (ofd) delete ofd;
      eps+=dIncr; sufNum++;
    }

  if (dPstProb) delete [] dPstProb;
  if (iLvDst) delete [] iLvDst;
  if (dPPS) delete [] dPPS;
  if (itfd) delete itfd;
  if (ivfd) delete ivfd;
  return 0;
}
