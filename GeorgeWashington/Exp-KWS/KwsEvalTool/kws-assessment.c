/*! \author Alejandro H. Toselli ahector@iti.upv.es
 *  \version 1.0
 *  \date    2014
 */

/* Copyright (C) 2013 by Pattern Recognition and Human Language
   Technology Group, Technological Institute of Computer Science,
   Valencia University of Technology, Valencia (Spain).

   Permission to use, copy, modify, and distribute this software and
   its documentation for any purpose and without fee is hereby
   granted, provided that the above copyright notice appear in all
   copies and that both that copyright notice and this permission
   notice appear in supporting documentation.  This software is
   provided "as is" without express or implied warranty.
*/

/* For compiling:
 * gcc -Wall -o kws-assessment kws-assessment.c */

#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include "uthash.h"      /* For hashing */


/* Definitions and stuffs */
/* *************************************************************** */
#define MLL 16384                   /* Maximum Line Length */
#define IDL 512                     /* Maximum ID Length */    

typedef unsigned short int usint;
typedef unsigned int uint;
typedef struct {
  uint idWrd;
  char bGrTh;
  double dPstProb;
} record;

int comp (const void *elem1, const void *elem2) {
  double f = ((const record*)elem1)->dPstProb;
  double s = ((const record*)elem2)->dPstProb;
  if (f < s) return  1;
  if (f > s) return -1;
  return 0;
}

typedef struct my_struct {
  char name[IDL];            /* key */
  uint id;
  usint visited;             /* flag of used word */
  UT_hash_handle hh;         /* makes this structure hashable */
} ht;
/* *************************************************************** */

/* Read words from file and put them into a hash-table */
ht* readWrdList(FILE *dfile) {
  ht *elem, *htabWrds = NULL;
  char line[MLL], *np, delim[] = " \t\n";
  uint n=0;
  while (fgets(line,MLL,dfile)!=NULL)
    /* This line allows to discard commented words with '#' */
    if ( (np=strtok(line,delim)) && (!(strlen(np)==1 && np[0]=='#')) ) {
      /* Checking for already existing hashed key */
      HASH_FIND_STR( htabWrds, np, elem);
      if (elem==NULL) {
	elem = (ht *)malloc(sizeof(ht));
	strcpy(elem->name,np);
	elem->id = n++; elem->visited = 0;
	HASH_ADD_STR( htabWrds, name, elem );
      }
    }
  return htabWrds;
}

/* Count visited words */
uint cntVisitWrds(ht *htWrds) {
  uint nWords = 0;
  ht *elem, *tmp;
  HASH_ITER(hh, htWrds, elem, tmp) {
    nWords += (elem->visited)?1:0;
  }
  return nWords;
}

/* free the hash table contents */
void freeHTWrds(ht *htWrds) {
  ht *elem, *tmp;
  HASH_ITER(hh, htWrds, elem, tmp) {
    HASH_DEL(htWrds, elem);
    free(elem);
  }
  HASH_CLEAR(hh, htWrds);
}
/* *************************************************************** */



record* readFile(FILE *dfile, uint *uNum, ht** htabWrds, ht** htabLns, char smth) {
  char line[MLL], *np;
  record *rc = NULL;
  uint n = 0, numW = 0;
  char delim[] = " \t\n";

  while (fgets(line,MLL,dfile)!=NULL)
    if ((np=strtok(line,delim)) && np[0]!='#') n++;
  rc = (record*)calloc(n, sizeof(record));
  
  if (fseek(dfile, 0L, SEEK_SET)) {
    fprintf(stderr,"This program does not accept stream inputs.\n");
    return NULL;
  }

  ht *elem, *auxHtabWrds = NULL;
  uint flagErr;
  rewind(dfile); n = 0;
  while (fgets(line,MLL,dfile)!=NULL) {
    flagErr = 0;
    if ((np=strtok(line,delim)) && np[0]!='#') {
      HASH_FIND_STR( *htabLns, np, elem);
      if (elem==NULL) {
        elem = (ht *)malloc(sizeof(ht));
	strcpy(elem->name,np);
	HASH_ADD_STR( *htabLns, name, elem );
      }
      if (!(np=strtok(NULL,delim))) {flagErr = 1; break; }
      if (*htabWrds) {
	HASH_FIND_STR( *htabWrds, np, elem);
	if (elem) {
	  rc[n].idWrd = elem->id;
	  elem->visited = 1;
	} else continue;
      } else {
	HASH_FIND_STR( auxHtabWrds, np, elem);
	if (elem==NULL) {
	  elem = (ht *)malloc(sizeof(ht));
	  strcpy(elem->name,np); rc[n].idWrd = numW;
	  elem->id = numW++; elem->visited = 1;
	  HASH_ADD_STR( auxHtabWrds, name, elem );
	} else rc[n].idWrd = elem->id;
      }
      if (!(np=strtok(NULL,delim))) { flagErr = 1; break; } 
      rc[n].bGrTh = np[0];
      if (!(np=strtok(NULL,delim))) { flagErr = 1; break; } 
      rc[n].dPstProb = atof(np);
      if ( rc[n].dPstProb < 0) {
        /* If smoothing is set, all rc[n].dPstProb<0 are set up to 0.0 */
        if (smth=='y') rc[n].dPstProb = 0.0;
        /* In case rc[n].dPstProb<0, it's set up to -1.0 */
        else if ( rc[n].dPstProb != -1.0 ) rc[n].dPstProb = -1.0;
      }
      n++;
    }
  }
  if (flagErr || n==0) {
    free(rc); rc = NULL;
    fprintf(stderr,"Syntax error in file or there is not any record matching the list of words.\n");
    return rc;
  }
  // Sort in descendent order
  qsort(rc, n, sizeof(record), comp);
  //uint i; for (i=0; i<n; i++) fprintf(stderr,"%d %c %g\n",rc[i].idWrd,rc[i].bGrTh,rc[i].dPstProb);
  if (*htabWrds == NULL) *htabWrds = auxHtabWrds;
  *uNum = n;
  return rc;
}


void globalAveragePrecision(record *rc, uint n, uint offset, char bIp, char trp) {
  /*
    TAU: Threshold Points
      D: counter of Detected events
      R: counter of relevant events
    NoR: counter of Non-Relevant events
      P: Precision on each threshold point
     Rc: Recall on each threshold point
     IP: Interpolated Precision on each threshold point
    CNT: CouNTer of events whose CM is not equal to -1 
     TR: Total number of Relevant events
  */
 
  /* We alloc n+1 units of memory in case we need to store counts for TAU=0.0 */
  double *TAU = (double*)calloc(n+1, sizeof(double));
  uint *D = (uint*)calloc(n+1, sizeof(uint));
  uint *R = (uint*)calloc(n+1, sizeof(uint));
  uint *NoR = (uint*)calloc(n+1, sizeof(uint));

  /* Flag to know whether there are negative dPstProb */
  int negThr = 0;

  uint CNT=0, TR=0, i;
  double dPP = -1.0;
  for (i=0; i<n; i++) {
    if (rc[i].dPstProb<0) {
      negThr = 1;
      if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') TR++;
      continue;
    }
    if (rc[i].dPstProb != dPP) {
      CNT++;
      dPP = rc[i].dPstProb;
      TAU[CNT-1] = dPP; D[CNT-1] = R[CNT-1] = NoR[CNT-1] = 0;
    }
    D[CNT-1]++;
    if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') { R[CNT-1]++; TR++; } else NoR[CNT-1]++;
  }
  if (offset && !negThr) {
    if (TAU[CNT-1]==0.0) { D[CNT-1] += offset; NoR[CNT-1] += offset; }
    else {
      CNT++;
      TAU[CNT-1] = 0.0; D[CNT-1] = offset; R[CNT-1] = 0; NoR[CNT-1] = offset;
    }
  }
  n+=offset; /* Used in the CER computation */

  if (TR==0) {
    fprintf(stderr,"No relevant events have been found ...\n");
    free(TAU); free(D); free(R); free(NoR);
    return;   
  }
  if (CNT==0) {
    fprintf(stderr,"No events have been found with positive threshold score ...\n");
    free(TAU); free(D); free(R); free(NoR);
    return;   
  }
  fprintf(stderr,"INFO: Total number of relevant events registered: %d\n",TR);

  double *P = (double*)calloc(CNT, sizeof(double));
  double *Rc = (double*)calloc(CNT, sizeof(double));
  uint ERR=TR, d=0, r=0, ind=0; 
  double min=DBL_MAX, aux, mxF=-1.0, auxF, tauF=-1; 
  double minCER=1.0, auxCER, tauCER=-1.0;
  for (i=0; i<CNT; i++) {
    ERR=ERR-R[i]+NoR[i]; auxCER=(double)ERR/n;
    if (minCER>auxCER) {minCER=auxCER; tauCER=TAU[i];}
    d+=D[i]; r+=R[i];
    P[i]=(double)r/d; Rc[i]=(double)r/TR;
    //fprintf(stderr,"%2.6f %d : %d %d\n",P[i],d,R[i],TR);
    aux=(P[i]-Rc[i]); aux=(aux<0)?-1*aux:aux;
    if (min>aux) {min=aux; ind=i;}
    auxF=((P[i]+Rc[i])!=0)?2*(P[i]*Rc[i])/(P[i]+Rc[i]):0;
    if (mxF<auxF) {mxF=auxF; tauF=TAU[i];}
  }
  double *IP = (double*)calloc(CNT, sizeof(double));
  double RP=Rc[ind], sum=0.0;
  if (bIp=='y') {
    double iP=0.0; 
    ind=0; min=DBL_MAX; mxF=-1.0;
    for (i=CNT; i>0; i--) {
      if (iP<P[i-1]) iP=P[i-1];
      IP[i-1]=iP;
      //fprintf(stderr,"%2.6f : %d %d\n",IP[i-1],R[i-1],TR);
      aux=(IP[i-1]-Rc[i-1]); aux=(aux<0)?-1*aux:aux;
      if (min>aux) {min=aux; ind=i-1;}
      auxF=((IP[i-1]+Rc[i-1])!=0)?2*(IP[i-1]*Rc[i-1])/(IP[i-1]+Rc[i-1]):0;
      if (mxF<auxF) {mxF=auxF; tauF=TAU[i-1];}
    }
    RP=Rc[ind];
    sum=(R[0]>0)?IP[0]*R[0]:0.0;
    if (trp=='y') {
      for (i=1; i<CNT; i++) sum+=(IP[i-1]+IP[i])*R[i]/2;
    } else for (i=1; i<CNT; i++) sum+=IP[i]*R[i];
  } else {
    if (trp=='y') for (i=1; i<CNT; i++) sum+=(P[i-1]+P[i])*R[i]/2;
    else for (i=0; i<CNT; i++) sum+=P[i]*R[i];
  }
  fprintf(stdout,"          AP = %.9f\n\
          RP = %.9f ( min|Rec-Prc| = %f )\n\
       F1max = %.9f ( thrs = %f )\n\
       RCmax = %.9f\n\
       PRres = %.9f\n\
      CERmin = %.9f ( thrs = %f )\n\
   CER(>1.0) = %.9f\n"\
  ,sum/TR,RP,min,mxF,tauF\
  ,Rc[CNT-1],(bIp=='y')?IP[CNT-1]:P[CNT-1]\
  ,minCER,tauCER,(double)TR/n);

  free(TAU); free(D); free(R); free(NoR);
  free(P); free(Rc); free(IP);
}


void meanAveragePrecision(record *rc, uint nR, uint nW, uint nL, char bIp, char trp, char vrb) {
  double s_ap = 0.0;
  /*
     D: counter of Detected events
     R: counter of Relevant events
     P: Precision on each threshold point
    IP: Interpolated Precision on each threshold point
   CNT: CouNTer of events whose CM is not equal to -1 
    TR: Total number of Relevant events per query
  */
  
  /* We alloc nR+1 units of memory in case we need to store counts for TAU=0.0 */
  uint *D = (uint*)calloc(nR+1, sizeof(uint));
  uint *R = (uint*)calloc(nR+1, sizeof(uint));
  double *P = (double*)calloc(nR+1, sizeof(double));
  double *IP = (double*)calloc(nR+1, sizeof(double));

  /* nL is set to zero if there are elements with negative dPstProb */
  if (rc[nR-1].dPstProb<0) nL=0;

  uint j, i, numRealWrds = 0;
  for (j=0; j<nW; j++) {

    uint CNT=0, TR=0, nO=0; /* nO: counts number of occurrences of word-id:j */
    double dPP = -1.0;
    for (i=0; i<nR; i++) {
      if (rc[i].idWrd != j) continue; else nO++;
      
      if (rc[i].dPstProb<0) {
	if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') TR++;
	continue;
      }
      if (rc[i].dPstProb != dPP) {
	CNT++;
	dPP = rc[i].dPstProb; D[CNT-1] = R[CNT-1] = 0;
      }
      D[CNT-1]++;
      if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') { R[CNT-1]++; TR++; }
    }
    if (nL) {
      uint offset = (nL>nO) ? nL-nO : 0;
      if (dPP==0.0) D[CNT-1] += offset;
      else {
        CNT++;
        D[CNT-1] = offset; R[CNT-1] = 0;
      }
    }
    if (TR!=0) numRealWrds++; // Relevant word counter increments if TR!=0
    if (TR==0 || CNT==0) {
      //fprintf(stderr,"INFO: No relevant events have been found for word: %d\n",j);
      if (vrb != 'n') fprintf(stderr,"INFO: wrdID: %d    AP: %.6f  TR: %d\n",j,0.00,TR);
      continue;
    }

    uint d = 0, r = 0;
    for (i=0; i<CNT; i++) {
      d+=D[i]; r+=R[i];
      P[i]=(double)r/d;
    }
    //fprintf(stderr,"wid:%d TR:%d CNT:%d d:%d r:%d\n",j,TR,CNT,d,r);

    double sum=0.0;
    if (bIp=='y') {
      double iP=0.0; 
      for (i=CNT; i>0; i--) {
	if (iP<P[i-1]) iP=P[i-1];
	IP[i-1]=iP;
      }
      sum=(R[0]>0)?IP[0]*R[0]:0.0;
      if (trp=='y') for (i=1; i<CNT; i++) sum+=(IP[i-1]+IP[i])*R[i]/2;
      else for (i=1; i<CNT; i++) sum+=IP[i]*R[i];
    } else {
      if (trp=='y') for (i=1; i<CNT; i++) sum+=(P[i-1]+P[i])*R[i]/2;
      else for (i=0; i<CNT; i++) sum+=P[i]*R[i];
    }
    
    if (vrb != 'n') fprintf(stderr,"INFO: wrdID: %d    AP: %.6f  TR: %d\n",j,sum/TR,TR);
    s_ap += sum/TR; 
    //fprintf(stderr,"%f %d\n",sum,numRealWrds);
  }

  fprintf(stdout,"         MAP = %.9f ( #Rel-Wrds = %d )\n",s_ap/numRealWrds,numRealWrds);
  free(D); free(R); free(P); free(IP);
}


void recallPrecisionCurve(record *rc, uint n, uint offset, char bIp, char trp) {
  /*
    TAU: threshold points
      D: counter of detected events
      R: counter of relevant events
    NoR: counter of non-relevant events
      P: precision on each threshold point
    CNT: CouNTer of events whose CM is not equal to -1 
    CER: Clasif. Error Rate on each threshold point
      H: counter hits accumulator on each threshold point
  */

  /* We alloc n+1 units of memory in case we need to store counts for TAU=0.0 */
  double *TAU = (double*)calloc(n+1, sizeof(double));
  uint *D = (uint*)calloc(n+1, sizeof(uint));
  uint *R = (uint*)calloc(n+1, sizeof(uint));
  uint *NoR = (uint*)calloc(n+1, sizeof(uint));

  /* Flag to know whether there are negative dPstProb */
  int negThr = 0;

  uint CNT=0, TR=0, i;
  double dPP = -1;
  for (i=0; i<n; i++) {
    if (rc[i].dPstProb<0) {
      negThr = 1;
      if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') TR++;
      continue;
    }
    if (rc[i].dPstProb != dPP) {
      CNT++;
      dPP = rc[i].dPstProb;
      TAU[CNT-1] = dPP; D[CNT-1] = R[CNT-1] = NoR[CNT-1] = 0;
    }
    D[CNT-1]++;
    if (rc[i].bGrTh=='1' || toupper(rc[i].bGrTh)=='T' || toupper(rc[i].bGrTh)=='Y') { R[CNT-1]++; TR++; } else NoR[CNT-1]++;
  }
  if (offset && !negThr) {
    if (TAU[CNT-1]==0.0) { D[CNT-1] += offset; NoR[CNT-1] += offset; }
    else {
      CNT++;
      TAU[CNT-1] = 0.0; D[CNT-1] = offset; R[CNT-1] = 0; NoR[CNT-1] = offset;
    }
  }
  n+=offset; /* Used in the CER computation */

  if (TR==0) {
    fprintf(stderr,"No relevant events have been found ...\n");
    free(TAU); free(D); free(R); free(NoR);
    return;   
  }
  if (CNT==0) {
    fprintf(stderr,"No events have been found with positive threshold score ...\n");
    free(TAU); free(D); free(R); free(NoR);
    return;   
  }

  if (trp=='y') {
    fprintf(stdout,"# ---------------------------------------------------------------------------------------------------------\n");
    fprintf(stdout,"# Threshold\tPrecision\tRecall   \tF1-measure\tClassif-ER\tFls-Alarm-Prob\tMiss-Prob   \n");
    fprintf(stdout,"# ---------------------------------------------------------------------------------------------------------\n");
  } else {
    fprintf(stdout,"# -----------------------------\n");
    fprintf(stdout,"# Precision\tRecall   \n");
    fprintf(stdout,"# -----------------------------\n");
  }
  double *P = (double*)calloc(CNT, sizeof(double));
  uint *H = (uint*)calloc(CNT, sizeof(uint));
  double *CER = (double*)calloc(CNT, sizeof(double));
  uint ERR=TR, d = 0, h = 0;
  for (i=0; i<CNT; i++) {
    ERR=ERR-R[i]+NoR[i]; CER[i]=(double)ERR/n;
    d+=D[i]; h+=R[i];
    P[i]=(double)h/d; H[i]=h;
  }
  double iPaux = 0.0, Rc, Fa, Ms, InP, F, pRc = 0.0, pInP = 0.0;
  for (i=CNT; i>0; i--) {
    // Interpolated precision
    if (iPaux<P[i-1]) iPaux=P[i-1];
    InP = (bIp=='y')?iPaux:P[i-1];

    Rc=(double)H[i-1]/TR;
    if (i==CNT) {pInP = InP; pRc = Rc;}

    F=((InP+Rc)!=0)?2*(InP*Rc)/(InP+Rc):0;
    Ms=1-Rc; Fa=Rc*TR*(1-InP)/((n-TR)*InP); 
    //fprintf(stdout,"%.7f\t%.7f\t%.7f\t%.7f\t%d\t%d\t%d\t%u\n",Fa,Ms,Rc,InP,TR,n,H[i-1],d); exit(1);
    if (trp=='y')
      fprintf(stdout,"%.7e\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",TAU[i-1],InP,Rc,F,CER[i-1],Fa,Ms);
    else {
      if (Rc!=pRc)
        fprintf(stdout,"%.7f\t%.7f\n%.7f\t%.7f\n",pInP,Rc,InP,Rc);
      else
        fprintf(stdout,"%.7f\t%.7f\n",InP,Rc);
      pRc = Rc; pInP = InP;
    }
  }
  if (bIp=='y') {
    if (trp=='y') {
      if (Rc>0) fprintf(stdout,"%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\t%.7f\n",TAU[i],InP,0.0,0.0,CER[i],0.0,1.0);
    } else if (Rc>0) fprintf(stdout,"%.7f\t%.7f\n",InP,0.0);
  }

  free(TAU); free(D); free(R); free(NoR);
  free(P); free(H); free(CER);
}


void ROC_Curve(record *rc, uint n, uint offset, char trp) {
  /*
    TAU: threshold points
    CNT: CouNTer of events whose CM is not equal to -1 
     dF: account the increment of false recog, on each threshold point
     dT: account the increment of true recog, on each threshold point
  */

  /* We alloc n+1 units of memory in case we need to store counts for TAU=0.0 */
  double *TAU = (double*)calloc(n+1, sizeof(double));
  uint *dT = (uint*)calloc(n+1, sizeof(uint));
  uint *dF = (uint*)calloc(n+1, sizeof(uint));

  uint CNT=0, TN=0, TP=0, FN=0, FP=0, i;
  double dPP = -1;
  if (offset && !(rc[n-1].dPstProb<0)) { /* This is applicable for elements with no negative dPstProb */
    CNT++;
    TAU[CNT-1] = dPP = rc[n-1].dPstProb;
    dT[CNT-1] = 0; dF[CNT-1] = offset; FP += offset;
  }
  for (i=n; i>0; i--) { /* read the rc in descending order */
    if (rc[i-1].dPstProb<0) {
      /*if (rc[i-1].bGrTh=='1' || toupper(rc[i-1].bGrTh)=='T' || toupper(rc[i-1].bGrTh)=='Y') FN++; else TN++;*/
      continue;
    }
    if (rc[i-1].dPstProb != dPP) {
      CNT++;
      dPP = rc[i-1].dPstProb;
      TAU[CNT-1] = dPP; dT[CNT-1] = dF[CNT-1] = 0;
    }
    if (rc[i-1].bGrTh=='1' || toupper(rc[i-1].bGrTh)=='T' || toupper(rc[i-1].bGrTh)=='Y') { dT[CNT-1]++; TP++; }
    else { dF[CNT-1]++; FP++; }
  }
  if (offset) {
    if (TAU[CNT-1]==0.0) dF[CNT-1] += offset;
    else {
      CNT++;
      TAU[CNT-1] = 0.0; dT[CNT-1] = 0; dF[CNT-1] = offset;
    }
    FP += offset;
  }

  if (CNT==0) {
    fprintf(stderr,"No events have been found with positive threshold score ...\n");
    free(TAU); free(dT); free(dF); 
    return;   
  }

  if (trp=='y') {
    fprintf(stdout,"# ------------------------------------------------------\n");
    fprintf(stdout,"# Num.\tThreshold\tFalse Ps.Rt\tTrue Ps.Rt\n");
    fprintf(stdout,"# ------------------------------------------------------\n");
  } else {
    fprintf(stdout,"# --------------------------------\n");
    fprintf(stdout,"# False Ps.Rt\tTrue Ps.Rt\n");
    fprintf(stdout,"# --------------------------------\n");
  }
  double TPR, FPR, pFPR = 0.0;
  uint C=TP+FN, I=TN+FP; /* C: Correct, I: Incorrect */
  for (i=0; i<CNT; i++) {
    TPR=(C)?((double)TP/C):((double)-1.0);
    FPR=(I)?((double)FP/I):((double)-1.0);
    if (i==0) pFPR = FPR;
    if (trp=='y')
      fprintf(stdout,"%4d\t%.7f\t%.7f\t%.7f\n",i+1,TAU[i],FPR,TPR);
    else
      fprintf(stdout,"%.7f\t%.7f\n%.7f\t%.7f\n",pFPR,TPR,FPR,TPR);
    TP-=dT[i]; /*FN+=dT[i];*/
    /*TN+=dF[i];*/ FP-=dF[i];
    pFPR=FPR;
  }
  TPR=(C)?((double)TP/C):((double)-1.0);
  FPR=(I)?((double)FP/I):((double)-1.0);
  if (trp=='y')
    fprintf(stdout,"%4d\t%5s\t\t%.7f\t%.7f\n",i+1,">1",FPR,TPR);
  else
    fprintf(stdout,"%.7f\t%.7f\n%.7f\t%.7f\n",pFPR,TPR,FPR,TPR);
  free(TAU); free(dT); free(dF); 
}


void compute_AUROC(record *rc, uint n, uint offset, char trp) {
  /*
    TAU: threshold points
    CNT: CouNTer of events whose CM is not equal to -1 
     dF: account the increment of false recog, on each threshold point
     dT: account the increment of true recog, on each threshold point
  */

  /* We alloc n+1 units of memory in case we need to store counts for TAU=0.0 */
  double *TAU = (double*)calloc(n+1, sizeof(double));
  uint *dT = (uint*)calloc(n+1, sizeof(uint));
  uint *dF = (uint*)calloc(n+1, sizeof(uint));
 
  uint CNT=0, TN=0, TP=0, FN=0, FP=0, i;
  double dPP = -1.0;
  if (offset && !(rc[n-1].dPstProb<0)) { /* This is applicable for elements with no negative dPstProb */
    CNT++;
    TAU[CNT-1] = dPP = rc[n-1].dPstProb;
    dT[CNT-1] = 0; dF[CNT-1] = offset; FP += offset;
  }
  for (i=n; i>0; i--) {  /* read the rc in descending order */
    if (rc[i-1].dPstProb<0) {
      /*if (rc[i-1].bGrTh=='1' || toupper(rc[i-1].bGrTh)=='T' || toupper(rc[i-1].bGrTh)=='Y') FN++; else TN++;*/
      continue;
    }
    if (rc[i-1].dPstProb != dPP) {
      CNT++;
      dPP = rc[i-1].dPstProb;
      TAU[CNT-1] = dPP; dT[CNT-1] = dF[CNT-1] = 0;
    }
    if (rc[i-1].bGrTh=='1' || toupper(rc[i-1].bGrTh)=='T' || toupper(rc[i-1].bGrTh)=='Y') { dT[CNT-1]++; TP++; }
    else { dF[CNT-1]++; FP++; }
  }

  if (CNT==0) {
    fprintf(stderr,"No events have been found with positive threshold score ...\n");
    free(TAU); free(dT); free(dF); 
    return;   
  }

  double dAUROC=0.0;
  uint C=TP+FN, I=TN+FP; /* C: Correct, I: Incorrect */
  //fprintf(stderr,"dAUROC:%.0f C:%d I:%d TP:%d FP:%d TN:%d FN:%d\n",dAUROC,C,I,TP,FP,TN,FN);
  for (i=0; i<CNT; i++) {
    //dAUROC += (trp=='y') ? (2*TP-dT[i]) * dF[i] / 2 : TP * dF[i];
    TP-=dT[i]; /*FN+=dT[i];*/
    /*TN+=dF[i]; FP-=dF[i];*/
    dAUROC += (trp=='y') ? (double)(2*TP+dT[i]) * (double)dF[i] / 2.0 : (double)TP * dF[i];
  }
  //double dFcNrm = (double)C*I/2.0;
  //fprintf(stderr,"dAUROC:%.0f dFcNrm:%.0f C:%d I:%d TP:%d FP:%d TN:%d FN:%d\n",dAUROC,dFcNrm,C,I,TP,FP,TN,FN);
  if (C&&I) fprintf(stdout,"        AROC = %.9f\n", dAUROC/(C*I));
  /*if (C&&I) fprintf(stdout,"        AROC = %.9f\n", dAUROC/dFcNrm;*/
  /*if (C&&I) fprintf(stdout,"        AROC = %.9f\n", dAUROC/dFcNrm - 1.0);*/
  else fprintf(stdout,"        AROC = undef\n");

  free(TAU); free(dT); free(dF); 
}


void usage(char *nomProg, char *roc, char *aroc, char *wl, char *ap, char *map, char *ip, char *trp, char *smth, char *verb) {
  fprintf(stderr,"\nUsage: %s [options] <table-file>\n",nomProg);
  fprintf(stderr,"\n   Options:\n");
  fprintf(stderr,"\t-r         Compute ROC Curve (def: %s)\n",roc);
  fprintf(stderr,"\t-u         Compute Area Under ROC Curve (def: %s)\n",aroc);
  fprintf(stderr,"\t-w <file>  Use a word list (def: %s)\n",wl);
  fprintf(stderr,"\t-l <+int>  Number of lines (used with -w option) (def: AUTO)\n");
  fprintf(stderr,"\t-a [-m]    Compute Global AP (def: %s)\n",ap);
  fprintf(stderr,"\t    -m     Compute ALSO Mean AP (def: %s)\n\n",map);
  fprintf(stderr,"\t-d         Disable Interpolated AP (def: %s)\n",ip);
  fprintf(stderr,"\t-t         Enable trapezoidal integration (def: %s)\n",trp);
  fprintf(stderr,"\t-s         Negative-scores aren't set up to 0 (def: %s)\n",smth);
  fprintf(stderr,"\t-v         Print query APs when -m is set (def: %s)\n\n",verb);

  fprintf(stderr,"Expected table-file format:\n\n");
  fprintf(stderr,"\tlinId1 \tword1\tGrTh(0|1)\tPst-Prob1\n");
  fprintf(stderr,"\tlinId2 \tword2\tGrTh(0|1)\tPst-Prob2\n");
  fprintf(stderr,"\t ... \t ...\t  ...\t\t   ...\n");
  fprintf(stderr,"\tlinIdN \twordN\tGrTh(0|1)\tPst-ProbN\n\n");

  fprintf(stderr,"\n    Without specifying the options: -[a,m,r], it returns\n  directly the recall-precision values (among others) in\n  function of the confidence measure.\n\n");

  fprintf(stderr,"\n    Tipical examples of use:\n\n");
  fprintf(stderr,"\t# Obtaining data for plotting the R-P curve\n");
  fprintf(stderr,"\t%s -t -s <table-file> > plotdata.dat\n\n",nomProg);
  fprintf(stderr,"\t# Obtaining AP, F1mx, ... including also MAP\n");
  fprintf(stderr,"\t%s -a -m <table-file>\n\n",nomProg);
  fprintf(stderr,"\t# Obtaining AP for 1st-best ...\n");
  fprintf(stderr,"\t%s -a -s <table-file>\n\n",nomProg);
  fprintf(stderr,"\t# Obtaining Area Under ROC curve ...\n");
  fprintf(stderr,"\t%s -u -t <table-file>\n\n",nomProg);
}



int main (int argc, char ** argv) {
  char *prog;
  int option;
  if ((prog=rindex(argv[0],'/'))) prog+=1;
  else prog=argv[0];
  FILE *idfile = NULL, *wdfile = NULL;
  char *ifname = NULL, *wfname = NULL;
  uint numR = 0, numL = 0, numW = 0, offset = 0;

  char roc = 'n', aroc = 'n', av_pr = 'n', mav_pr = 'n', ip = 'y', trap = 'n', smth = 'y', verb = 'n';
  while ((option=getopt(argc,argv,"ruw:l:amdtsvh"))!=-1)
    switch (option) {
    case 'r': 
      roc = 'y';
      break;
    case 'u': 
      aroc = 'y';
      break;
    case 'w':
      wfname = optarg;
      if ((wdfile = fopen(wfname, "r")) == NULL) {
        fprintf(stderr,"ERROR: File \"%s\" cannot be opened!\n",wfname);
        return 1;
      }
      break;
    case 'l': 
      numL = atoi(optarg);
      break;
    case 'a': 
      av_pr = 'y';
      break;
    case 'm': 
      mav_pr = 'y';
      break;
    case 'd': 
      ip = 'n';
      break;
    case 't': 
      trap = 'y';
      break;
    case 's': 
      smth = 'n';
      break;
    case 'v': 
      verb = 'y';
      break;
    case 'h':
    default:
      usage(prog,(roc=='y')?"ON":"OFF",		\
	    (aroc=='y')?"ON":"OFF",		\
	    (wfname)?wfname:"NULL",		\
	    (av_pr=='y')?"ON":"OFF",		\
	    (mav_pr=='y')?"ON":"OFF",		\
	    (ip=='n')?"ON":"OFF",		\
	    (trap=='y')?"ON":"OFF",		\
	    (smth=='y')?"OFF":"ON",		\
	    (verb=='n')?"OFF":"ON"		\
	    );
      return 1;
    }

  if (argc - optind == 1) {
    ifname = argv[optind];
    if ((idfile = fopen(ifname, "r")) == NULL) {
      fprintf(stderr,"ERROR: File \"%s\" cannot be opened!\n",argv[optind]);
      return 1;
    }
  } else {
    usage(prog,(roc=='y')?"ON":"OFF",		\
	  (aroc=='y')?"ON":"OFF",		\
	  (wfname)?wfname:"NULL",		\
	  (av_pr=='y')?"ON":"OFF",		\
	  (mav_pr=='y')?"ON":"OFF",		\
	  (ip=='n')?"ON":"OFF",			\
	  (trap=='y')?"ON":"OFF",		\
	  (smth=='y')?"OFF":"ON",		\
	  (verb=='n')?"OFF":"ON"		\
	  );
    return 1;
  }

  ht *htWrds = NULL, *htLns = NULL;
  if (wdfile) htWrds = readWrdList(wdfile);
  else if (numL) {
    fprintf(stderr,"WARNING: Number of lines (-l) has been set up without option -w. Ignoring it !\n");
    numL=0;
  }

  record *aRecords = readFile(idfile, &numR, &htWrds, &htLns, smth);
  if (!aRecords) return 1; 
 
  numW = HASH_COUNT(htWrds);
  if (!numL) numL = HASH_COUNT(htLns);
  fprintf(stderr,"INFO: Total number of events registered: %d\n",numR);
  fprintf(stderr,"INFO: Total number of different read line IDs: %d\n",numL);
  fprintf(stderr,"INFO: Total number of different read words: %d\n",numW);
  if (wdfile) {
    uint numV = cntVisitWrds(htWrds);
    if (numW-numV) fprintf(stderr,"INFO: Total number of OOV words: %d\n",numW - numV);
  }

  offset = (wdfile && numW*numL>numR)?numW*numL-numR:0; 
  if (offset) fprintf(stderr,"INFO: Estimated number of missing no-relevant events (OffSet) = %d\n",offset);

  if (av_pr == 'n') {
    if (roc == 'y') ROC_Curve(aRecords,numR,offset,trap);
    else if (aroc == 'y') compute_AUROC(aRecords,numR,offset,trap);
    else recallPrecisionCurve(aRecords,numR,offset,ip,trap);
  } else {
    if (mav_pr == 'y') meanAveragePrecision(aRecords,numR,numW,(wdfile)?numL:0,ip,trap,verb);
    globalAveragePrecision(aRecords,numR,offset,ip,trap);
  }

  if (htWrds) freeHTWrds(htWrds);
  if (htLns) freeHTWrds(htLns);
  if (aRecords) free(aRecords);
  if (wfname) fclose(wdfile);
  fclose(idfile);
  return 0;
}
