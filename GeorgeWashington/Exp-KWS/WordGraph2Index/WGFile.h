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

#ifndef WGFile_H
#define WGFile_H

#include "WG.h"

#include <iostream>
#include <string>

#define UNSET_PAR ((std::numeric_limits<float>::max()))

using std::istream;
using std::string;
using std::endl;


/* WG File Class */ 
class WGFile {
 private:
  
  istream* ifd;        /* Grammar File/STD Input Stream */
  uint uLine;          /* Number of line read from the grammar
			  input file */

  float wgDisp;        /* WG Dispersion factor */
  float lmScl;         /* Language model scale factor */
  float wIPen;         /* Word Insertion penalty */
  float prnScl;        /* Pronunciation scale factor */
  float accScl;        /* Morphologic scale factor */
  float tmScl;         /* Time scale factor */
  uint verbosity;      /* Verbosity level */

  /* Method to read one "valid" line for the WG file, ruling
     out blank lines, remarks, etc. */  
  string readLine();

  /* Method to read all the states of the grammar file in HTK format */
  bool readElements_HTK(WG *wg, const string & dummyTk);


 public:

  /* Constructor */ 
  WGFile(): ifd(NULL), uLine(0), wgDisp(1.0), lmScl(1.0), wIPen(0.0), \
            prnScl(1.0), accScl(1.0), tmScl(100.0), verbosity(0) {};
  
  /* Set grammar scale factor, word insertion penalty, 
     accustic scale factor and pronunciation scale factor */
  void setWGParameters(float lmsf=1.0, float wip=0.0, float asf=1.0, float psf=1.0, float wgd=1.0, float tscl=100.0);

  /* Read WG File in HTK format */
  bool readFile_HTK(istream & fd, WG *wg, const string & dummyTk = "");

  /* Print the WG to outstream in HTK format */
  void print_HTK(WG *wg, ostream & fd);

  /* Print the WG to outstream in DOT format */
  void print_DOT(WG *wg, ostream & fd);

  /* Print the WG */
  void print_TEST(WG *wg, ostream & fd);

  /* Print the WG statistics */
  void print_WGStat(WG *wg, ostream & fd);

  /* Print List of Word-Edges along with the corresponding CM */
  void print_EdgesList(WG *wg, ostream & fd);

  /* Print List of Voc Words along with the corresponding Line Post-Prob */
  void print_LineWrdProb(WG *wg, ostream & fd);

  /* Print List of Words along with their respective Max Post-Prob and Interval */
  void print_ListOfWrdWithMaxProb(WG *wg, ostream & fd);

  /* Print List of Words along with their respective Exact Post-Prob Occupancy */
  void print_ListOfWrdWithExactProb(WG *wg, ostream & fd);

  /* Print Words along with their respective sequence of Post-Prob/frame */
  void print_TableOfWrdProb(WG *wg, ostream & fd);

  /* Print List of Words along with their respective Global Occupancy Post-Prob in the Line */
  void print_GlobalWrdProb(WG *wg, ostream & fd); 

  /* Print List of Words along with their N-Best Occupancy Post-Probs in the Line */
  void print_MultiWrdProb(WG *wg, ostream & fd);

  /* Set verbosity level */
  void setVerbosity(int vb) { verbosity = vb; };

};

#endif
