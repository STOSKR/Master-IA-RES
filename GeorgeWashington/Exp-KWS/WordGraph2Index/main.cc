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


#include "WGFile.h"
#include "WGN-Best.h"
#include "WGParsingString.h"
#include "WGErrCorr.h"
#include "WGPrunning.h"
#include "gzstream.h"

#include <unistd.h>
#include <string.h>

#if (defined TAKETIME)
#include <sys/times.h>
#endif

#include <fstream>
#include <iomanip>

using std::showpoint;
using std::setprecision;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::cerr;
using std::cin;
using std::cout;
using std::ios;

void usage(char *nomProg, const char *in, const char *inpImg, const char *out, const char *prWGthr, int nb, float bps, float wgd, float timScl, char frm, const char *w, const char *e, const char *l, int verb, const char *outStat) {
  cerr.setf(ios::fixed); cerr.precision(2); // set number of decimals to 2
  cerr << endl;
  cerr << "Usage: " << nomProg << " [options]" << endl << endl;
  cerr << "  Options:\n" << endl;
  cerr << "     -i <file>         Word graph input file            (def.: " << in << ")" << endl;
  cerr << "     -m                Load/Save input data image       (def.: " << inpImg << ")" << endl;
  cerr << "     -o <file>         Output file                      (def.: " << out << ")" << endl;
  cerr << "     -u <float>        WG prunning threshold            (def.: " << prWGthr << ")" << endl;
  cerr << "     -n <[-1:*]>       N-best hypotheses (-1 for ALL)   (def.: " << nb << ")" << endl;
  cerr << "     -y <-float>       Beam Prunning Score respect to\n\
                       the 1-Best Score (0.0 for N/A)   (def.: " << bps << ")" << endl;
  cerr << "     -d <float>        Log Base Scale Factor            (def.: " << wgd << ")" << endl;
  cerr << "     -s <float>        Language Model Scale Factor      (def.: read from file)" << endl;
  cerr << "     -p <float>        Word Insertion Penalty           (def.: read from file)" << endl;
  cerr << "     -r <float>        Pronunciation Scale Factor       (def.: read from file)" << endl;
  cerr << "     -a <float>        Morphologic Scale Factor         (def.: read from file)" << endl;
  cerr << "     -t <float>        Time Scale Factor                (def.: " << timScl << ")" << endl;
  cerr << "     -f <h|d|t>        Output Fomat: h (htk), d (dot) \n\
                       t (test)                         (def.: " << frm << ")" << endl;
  cerr << "     -w <string>       Generate a resulting WG from \n\
                       the parsed string                (def.: " << w << ")" << endl;
  cerr << "     -e <string>       Perform Error Correcting on the \n\
                       given string                     (def.: " << e << ")" << endl;
  cerr << "     -l <string>       Unparseable string tokens        (def.: " << l << ")" << endl;
  cerr << "     -v <0|1|2|3>      Verbosity                        (def.: " << verb << ")" << endl;
  cerr << "     -z <e|w|m|x|l|t>  Output list of:                  (def.: none)\n\
                          e (word edges with their C-Measures)\n\
			  w (voc. words with their Max P-Probs)\n\
                          m (voc. words with their N-Best Max P-Probs)\n\
			  x (voc. words with their Exact P-Probs)\n\
			  l (voc. words with their P-Probs/line)\n\
			  g (voc. words with their Global P-Probs)\n\
			  t (voc. words with their P-Probs/frame)" << endl;
  cerr << "     -g                Output WG information            (def.: " << outStat << ")" << endl;
  cerr << "     -h                This help" << endl;
  cerr << endl << endl;
}


int main(int argc, char ** argv) {
  
  char *prog;
  int option;
  if ((prog=strrchr(argv[0],'/'))) prog+=1;
  else prog=argv[0];
  
  char *ifname = NULL, *ofname = NULL;
  float prWGthr = -1.0;
  int nbest = 0;
  float fBeamPrScr = 0.0;
  //ifstream *ifd=NULL;
  igzstream *ifd = NULL;
  ofstream *ofd = NULL;
  FILE *fimg = NULL;
  float wgDisp = 1.0, lmScl=UNSET_PAR, wip=UNSET_PAR, prnScl=UNSET_PAR, accScl=UNSET_PAR, timScl = 100.0;
  char frm = 't', probOutFrm = '\0';
  string pword, rword, lambda=""; //"<s> </s> !NULL";
  uint verbosity = 0;
  string wgName = "stdin";
  bool bWGStat = false, bFileImage = false, bImgFlg = false;

#if (defined TAKETIME)
  // Declaration for taking time measures
  static clock_t t1 ,t2, t3, t4;
  static struct tms st1, st2, st3, st4;

  // Time measure point
  t1 = times(&st1);
#endif

  while ((option=getopt(argc,argv,"i:mo:u:n:y:d:s:p:r:a:t:f:w:e:v:z:l:gh"))!=-1)
    switch (option) {
    case 'i':
      ifname=optarg;
      //ifd=new ifstream(ifname);
      ifd=new igzstream(ifname);
      //if (!ifd->is_open()) {
      if (!ifd->rdbuf()->is_open()) {
        cerr << "ERROR: File \"" << ifname << "\" could not be opened!" << endl;
        return 1;
      }
      wgName=ifname;
      break;
    case 'm':
      bImgFlg=true;
      break;
    case 'o':
      ofname=optarg;
      ofd=new ofstream(ofname);
      if (!ofd->is_open()) {
	cerr << "ERROR: File \"" << ofname << "\" could not be opened!" << endl;
        return 1;
      }
      break;
    case 'u':
      prWGthr=atof(optarg);
      break;
    case 'n':
      nbest=atoi(optarg);
      break;
    case 'y':
      fBeamPrScr=atof(optarg);
      break;
    case 'd':
      wgDisp=atof(optarg);
      break;
    case 's':
      lmScl=atof(optarg);
      break;
    case 'p':
      wip=atof(optarg);
      break;
    case 'r':
      prnScl=atof(optarg);
      break;
    case 'a':
      accScl=atof(optarg);
      break;
    case 't':
      timScl=atof(optarg);
      break;
    case 'f':
      frm=*optarg;
      break;
    case 'w':
      pword=string(optarg);
      break;
    case 'e':
      rword=string(optarg);
      break;
    case 'v':
      verbosity=(uint)atoi(optarg);
      break;
    case 'z':
      probOutFrm=*optarg;
      break;
    case 'l':
      lambda=string(optarg);
      break;
    case 'g':
      bWGStat=true;
      break;
    case 'h':
    default:
      usage(prog,ifd?ifname:"stdin",\
      bImgFlg?"ON":"OFF",\
      ofd?ofname:"stdout",\
      (prWGthr>0)?"ON":"<=0.0 -> OFF",\
      nbest,fBeamPrScr,wgDisp,timScl,frm,\
      pword.empty()?"empty":pword.c_str(),\
      rword.empty()?"empty":rword.c_str(),\
      lambda.empty()?"empty":lambda.c_str(),\
      verbosity,bWGStat?"ON":"OFF");
      return 1;
    }


  WG *wg=new WG(), *pwg=NULL;
  WGFile wgLoad;

  string auxStr;
  if (ifd) auxStr=string(ifname)+".img";
  if (bImgFlg && (fimg=fopen(auxStr.c_str(),"rb"))) {
    cerr << "Reading from File Image: \"" << auxStr << "\"" << endl;
    wg->readFromBinFile(fimg);
    fclose(fimg);
    bFileImage=true;
  } else {
    wgLoad.setWGParameters(lmScl,wip,prnScl,accScl,wgDisp,timScl);
    wgLoad.setVerbosity(verbosity);

    // HTK Format
    if (ifd) {
      if (!wgLoad.readFile_HTK(*ifd,wg,lambda)) return 1;
    } else {
      cerr << "Reading from stdin ..." << endl;
      if (!wgLoad.readFile_HTK(cin,wg,lambda)) return 1;
    }
#if (defined TAKETIME)
    // Time measure point
    t2 = times(&st2);
#endif
    if (rword.empty()) {
      if (!wgName.empty()) wg->setName(wgName);
      if (prWGthr<=0) {
        wg->viterbiForward();
        wg->viterbiBackward();
        wg->compPostProb();
      }
#if (defined TAKETIME)
      // Time measure point
      t3 = times(&st3);
#endif
    }
  }

  ////////////////////////
  // Perform WG prunning 
  ////////////////////////
  if (prWGthr>0) {
    if ((pwg=reduceSize(*wg,prWGthr))) {
      delete wg; wg=pwg;
      size_t p=wgName.find_last_of(".");
      if (p==string::npos) wgName.append(".red");
      else wgName.insert(p,".red");
      wg->setName(wgName);
      //wg->resetProb();
      wg->viterbiForward();
      wg->viterbiBackward();
      wg->compPostProb();
    } else {
      cerr << "ERROR: WG prunning process has failed !" << endl;
      return 1;
    }
  }

  /////////////////////
  // Error Correcting 
  /////////////////////
  if (!rword.empty()) {
    cout << "----------------------------------------------------------------" << endl;
    string sResuls;
    errorCorrecting(*wg,rword,sResuls,"");
    cout << " Edition Cost: " << sResuls << endl;
    cout << "----------------------------------------------------------------" << endl;
  }
  /////////////////////
 
  /////////////////////
  if (!pword.empty()) {
    cerr << "PARSING: " << pword << endl;
    if (parsingString(*wg,pword,lambda)) cerr << "ACCEPTED" << endl;
    else cerr << "UNACCEPTED" << endl;
  }
  if (!pword.empty()) {
    cerr << "PARSING: " << pword << endl;
    pwg=parsingStringToGraph(*wg,pword,lambda);
    delete wg; wg=pwg;
    if (wg) wg->setName(pword);
  }
  /////////////////////

  //const uint *T = pwg->getTopologicalOrder();
  //for (uint p=0; p<pwg->getTotalNumberOfStates(); p++)
  //  cout << p << " " << (pwg->getState(p))->getID() << " " << T[p] << endl;

  if (rword.empty() && wg) {

    if (ofd) {
      string str(ofname);
      wg->setName(str);
    }
    ostream *auxfd = (ofd==NULL)?&cout:ofd;

    if (nbest!=0) {
      branchAndBound(*wg,nbest,fBeamPrScr,*auxfd,verbosity);
    } else if (probOutFrm) {
      switch (probOutFrm) {
        case 'e': wgLoad.print_EdgesList(wg,*auxfd); break;
        case 'l': wgLoad.print_LineWrdProb(wg,*auxfd); break;
        case 'g': wgLoad.print_GlobalWrdProb(wg,*auxfd); break;
        case 't': wgLoad.print_TableOfWrdProb(wg,*auxfd); break;
        case 'x': wgLoad.print_ListOfWrdWithExactProb(wg,*auxfd); break;
        case 'm': wgLoad.print_MultiWrdProb(wg,*auxfd); break;
        case 'w': 
        default: wgLoad.print_ListOfWrdWithMaxProb(wg,*auxfd);
      }
    } else if (bWGStat) {
      wgLoad.print_WGStat(wg,*auxfd);
    } else
      switch (frm) {
      case 'h':
	wgLoad.print_HTK(wg,*auxfd); break;
      case 'd':
	//wg->viterbiBackward();
	wgLoad.print_DOT(wg,*auxfd); break;
      default:
	//wg->viterbiForward();
	//wg->viterbiBackward();
	wgLoad.print_TEST(wg,*auxfd);
      }
  }

  if (bImgFlg && !bFileImage) {
    cerr << "Saving to File Image: \"" << auxStr << "\" ";
    cerr << "with a total storage of ";
    fimg=fopen(auxStr.c_str(),"wb");
    cerr << wg->writeToBinFile(fimg) << " bytes" << endl;
    fclose(fimg);
  }

  if (wg) delete wg;
  if (ifd) delete ifd;
  if (ofd) delete ofd;

#if (defined TAKETIME)
  // Time measure point
  t4 = times(&st4);
  float tps = (float)sysconf (_SC_CLK_TCK);

  fprintf(stderr,"\nWord-Graph Loading (seg.):\nReal Time: %2.4f, User Time: %2.4f, System Time: %2.4f\n\n", \
	  (float)((t2 - t1)/(float)tps), \
	  (float)((st2.tms_utime - st1.tms_utime)/tps), \
	  (float)((st2.tms_stime - st1.tms_stime)/tps) \
	  );
  fprintf(stderr,"Forwad-Backward Process (seg.):\nReal Time: %2.4f, User Time: %2.4f, System Time: %2.4f\n\n", \
	  (float)((t3 - t2)/(float)tps), \
	  (float)((st3.tms_utime - st2.tms_utime)/tps), \
	  (float)((st3.tms_stime - st2.tms_stime)/tps) \
	  );
  fprintf(stderr,"Posteriorgram Generation (seg.):\nReal Time: %2.4f, User Time: %2.4f, System Time: %2.4f\n", \
	  (float)((t4 - t3)/(float)tps), \
	  (float)((st4.tms_utime - st3.tms_utime)/tps), \
	  (float)((st4.tms_stime - st3.tms_stime)/tps) \
	  );
#endif

  return 0;
}
