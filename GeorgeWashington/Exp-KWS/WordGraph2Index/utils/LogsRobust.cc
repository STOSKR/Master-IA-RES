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

#include "LogsRobust.h"

double sumLogs(double x, double y) {
  double temp, diff;
  if (x<y) { temp = x; x = y; y = temp; }
  diff = y - x;
  //if (diff <= minLogExp) return (x<LSMALL)?LZERO:x;
  if (diff <= minLogExp) return x;
  else return x + log( 1.0 + exp(diff) );
}
