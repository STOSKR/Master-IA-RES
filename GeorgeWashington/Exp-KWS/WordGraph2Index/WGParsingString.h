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

#ifndef WGPARSING_H
#define WGPARSING_H

#include "WG.h"

/* Check whether a given input string word is in the WG */
bool parsingString(WG & wg, const string & word, const string & tk = "");

/* Generate a new WG based on parsing a given input string word */
WG* parsingStringToGraph(WG & wg, const string & word, const string & tk = "");

#endif
