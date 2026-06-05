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

#ifndef WGCONFMEASURE_H
#define WGCONFMEASURE_H

#include "WG.h"

/* Using by the "computeMultiWordMaxProb()" function */
typedef struct {
  double dPostProb;
  uint sTime, fTime;
} multiScore;

/* Maximum score percentage over which is computed the word borders */
const float THRESHMAXSRC = 0.9;

/* Compute different values based on the post-probabilities of WG's arcs */
double computeConfMeasure(WG & wg, int iArcId);

void computeArcMaxProb(WG & wg, int iArcId, double &dMaxProb, uint &sTime, uint &fTime);

void computeWordMaxProb(WG & wg, int idxW, double &dMaxProb, uint &sTime, uint &fTime);

/* Using the disjoint probability formula */
double computeWordTotProb(WG & wg, int idxW);

double computeWordExactProb(WG & wg, int idxW);

double computeLineWordProb(WG & wg, int idxW);

double computeWordGlobalProb(WG & wg, int idxW);

/* This is an implementation proposed by Enrique Vidal. It improves
   the version of the plain sum of word edge post-probabilities. Its
   is assumed that the states are sorted according to their
   time-stamp. */
double* computeWordGlobalProb(WG & wg);

/* This is an implementation proposed by Enrique Vidal. It improves
   the version of the plain sum of word edge post-probabilities. Its
   is assumed that the states are sorted according to their
   time-stamp. This is actually "computeWordGlobalProb" extended to
   produce the N-best word localizations in the line.*/
vector<multiScore>* computeMultiWordMaxProb(WG & wg);

#endif
