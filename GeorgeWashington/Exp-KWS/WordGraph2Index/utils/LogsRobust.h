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

#ifndef LOGR_H
#define LOGR_H

#include <cmath>
#include <limits>

/* The following was taken from HTK library */
//#define LZERO  (-1.0E100)
#define LZERO  (-(std::numeric_limits<double>::infinity()))

//#define LSMALL (-0.5E100)
//#define minLogExp -230.25850929940456840179  /* -log(-LZERO) */
//#define ProbLogFloat2ProbFloat(x) ((x) < LSMALL ? 0.0 : (double)exp((x)))
#define minLogExp (std::numeric_limits<double>::min_exponent10)
#define ProbLogFloat2ProbFloat(x) (exp((x)))

//#define SMALL (0.5E-100)
//#define minLog -230.95165647996451371121637758989459733   /* log(SMALL) */
//#define ProbFloat2ProbLogFloat(x) ((x) < SMALL ? minLog : (double)log((x)))
#define SMALL (std::numeric_limits<double>::min())
#define minLog (-(std::numeric_limits<double>::max()))
#define ProbFloat2ProbLogFloat(x) ((x) < SMALL ? minLog : log((x)))


double sumLogs(double x, double y);

#endif

