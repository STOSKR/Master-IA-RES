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

#ifndef STRINGTOKENIZER_H
#define STRINGTOKENIZER_H

#include <string>

class StringTokenizer {
   public:
    StringTokenizer(const std::string& _str, const char* _delim=" \t", const char* _delimSpc="");
    ~StringTokenizer(){};

    void         addNewLine(const std::string& _str);
    unsigned int countTokens();
    bool         hasMoreTokens();
    std::string  nextToken();
    int          nextIntToken();
    double       nextFloatToken();
    double       nextFloatToken(const char* delim);
    std::string  nextToken(const char* delim);
    std::string  remainingString();
    std::string  filterNextToken(const std::string& filterStr);

   private:
    std::string  token_str;
    std::string  delim;
    std::string  delimSpc;
    void         initializeDelim();
    inline bool  isDelim(char c, bool bSpc=false) const;
};


////////////////////////////////////////////////////////////////////////////
// Other useful functions
////////////////////////////////////////////////////////////////////////////
// Make an uppercase copy of s:
std::string upperCase(std::string &);

// Make a lowercase copy of s:
std::string lowerCase(std::string &);


#endif
