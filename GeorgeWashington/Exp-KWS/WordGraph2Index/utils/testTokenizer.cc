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


#include "StringTokenizer.h"
#include <iostream>

using std::cout;
using std::endl;
using std::string;

int main() {

  string line = "J=0 S=1 E=2 W=ho=====la== a=23.45 l=hsjhd==";
  StringTokenizer strToken(line,"\t ","="); 
  cout << line << ": " << strToken.countTokens() << endl;

  while (strToken.hasMoreTokens()) {
    cout << strToken.nextToken()+" ";
  }
  cout << endl;

  return 0;
}
