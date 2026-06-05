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

#include "StringTokenizer.h"
#include <stdlib.h>


inline bool StringTokenizer::isDelim(char c, bool bSpc) const {
   if (bSpc) { /* Search for special delimiters */
     if (!delimSpc.empty() && delimSpc.find(c) != std::string::npos) return true;
   } else if (delim.find(c) != std::string::npos) return true; /* Search for normal delimiters */
   return false;
}

StringTokenizer::StringTokenizer(const std::string& _str, const char* _delim, const char* _delimSpc) {
   token_str = _str;
   delim     = _delim;
   delimSpc  = _delimSpc;
   if (_str.length()) initializeDelim();
}

void StringTokenizer::addNewLine(const std::string& _str) {
   token_str.clear();
   token_str = _str;
   if (_str.length()) initializeDelim();
}

void StringTokenizer::initializeDelim() {
   unsigned int curr_pos = 0;

   /* Remove sequential delimiter */
   while(true) {
     for ( ; curr_pos < token_str.length() && !isDelim(token_str.at(curr_pos)); curr_pos++);
     if (curr_pos == token_str.length()) break;
     curr_pos++;
     while (curr_pos < token_str.length() && isDelim(token_str.at(curr_pos))) token_str.erase(curr_pos,1);
   }

   /* Trim leading delimiter */
   if (isDelim(token_str.at(0))) token_str.erase(0,1);

   /* Trim ending delimiter */
   if (isDelim(token_str.at(token_str.length()-1))) token_str.erase(token_str.length() - 1,1);

   if (delimSpc.empty()) return;
   /* Replace the first special delimiter appearing after each normal delimiter by this */
   curr_pos = 0;
   while(true) {
     for ( ; curr_pos < token_str.length() && !isDelim(token_str.at(curr_pos),true); curr_pos++);
     if (curr_pos == token_str.length()) break;
     token_str.replace(curr_pos,1,1,delim.at(0)); curr_pos++;
     for ( ; curr_pos < token_str.length() && !isDelim(token_str.at(curr_pos)); curr_pos++);
   }
}

unsigned int StringTokenizer::countTokens() {
   unsigned int num_tokens        = 0;

   if (token_str.length() > 0) {
      num_tokens = 0;
      for (unsigned int curr_pos = 0; curr_pos < token_str.length(); curr_pos++)
         if (isDelim(token_str.at(curr_pos))) num_tokens++;
      return ++num_tokens;
   } else return 0;
}


bool StringTokenizer::hasMoreTokens() {
   return (token_str.length() > 0);
}


std::string StringTokenizer::nextToken() {
   if (token_str.length() == 0) return "";

   std::string tmp_str = "";
   unsigned int pos;
   for (pos=0; pos < token_str.length() && !isDelim(token_str.at(pos)); pos++);

   if (pos < token_str.length()) {
      tmp_str   = token_str.substr(0,pos);
      token_str = token_str.substr(pos+1);
      //token_str = token_str.substr(pos + 1,token_str.length() - pos);
   } else {
      tmp_str   = token_str.substr(0,token_str.length());
      token_str = "";
   }

   return tmp_str;
}


int StringTokenizer::nextIntToken() {
   return atoi(nextToken().c_str());
}


double StringTokenizer::nextFloatToken() {
   return atof(nextToken().c_str());
}


double StringTokenizer::nextFloatToken(const char* delim) {
   return atof(nextToken(delim).c_str());
}


std::string StringTokenizer::nextToken(const char* delimiter) {
   if (token_str.length() == 0) return "";

   delim     = delimiter;
   delimSpc.clear();
   initializeDelim();

   std::string tmp_str = "";
   unsigned int pos;
   for (pos=0; pos < token_str.length() && !isDelim(token_str.at(pos)); pos++);

   if (pos < token_str.length()) {
      tmp_str = token_str.substr(0,pos);
      token_str = token_str.substr(pos + 1);
      //token_str = token_str.substr(pos + 1,token_str.length() - pos);
   } else {
      tmp_str = token_str.substr(0,token_str.length());
      token_str = "";
   }

   return tmp_str;
}

std::string StringTokenizer::remainingString() {
   return token_str;
}


std::string StringTokenizer::filterNextToken(const std::string& filterStr) {
   std::string  tmp_str    = nextToken();
   unsigned int currentPos = 0;

   while((currentPos = tmp_str.find(filterStr,currentPos)) != std::string::npos)
      tmp_str.erase(currentPos,filterStr.length());

   return tmp_str;
}






// Make an uppercase copy of s:
std::string upperCase(std::string& s) {
  char* buf = new char[s.length()];
  s.copy(buf, s.length());
  for(int i=0; i<(int)s.length(); i++)
    buf[i] = toupper(buf[i]);
  std::string r(buf, s.length());
  delete [] buf;
  return r;
}

// Make a lowercase copy of s:
std::string lowerCase(std::string& s) {
  char* buf = new char[s.length()];
  s.copy(buf, s.length());
  for(int i=0; i<(int)s.length(); i++)
    buf[i] = tolower(buf[i]);
  std::string r(buf, s.length());
  delete [] buf;
  return r;
}
