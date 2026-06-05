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

#ifndef WGERRCORR_H
#define WGERRCORR_H

#include "WG.h"

#define PUIINF ((std::numeric_limits<uint>::max())) /* Largest uint value */

/* Perform Error-Correcting parsing on a given input string */
void errorCorrecting(WG &wg, const string &word, string &strResult, const string &tk = "");

#endif
