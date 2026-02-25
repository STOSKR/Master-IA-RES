/*
 *   Copyright 2017, Moisés Pastor i Gadea
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *  Created on: 20/10/2017
 *      Author: Moisés Pastor i Gadea
 */
#include "libXmlPAGE.h"

#include <unistd.h>

#include <map>
#include <sstream>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stack> 
#include <vector>
#include <algorithm>

#include "Graf.h"
#include "slopeClass.h"
#include "pugixml.hpp"
#include "libXmlPAGE.h"
#include "libPoints.h"

using namespace cv;
using namespace std;

int verbosity=0;
bool demo=false;
//int MIN_NUM_POINTS_LINE=4;  //VORE QUE FER AMB AÇO
const int MARGE=20;
//---------------------------------------------------------------------
struct  k{
  bool operator() (cv::Point pt1, cv::Point pt2) { if(pt1.x < pt2.x) return true; else if (pt1.x > pt2.x) return false; else return pt1.x < pt2.y ;}
} sort_points_func;

//---------------------------------------------------------------------
struct k1{
  bool operator() (LineStruct v1, LineStruct v2){
    double sum_y_v1=0;
    for (unsigned int i = 0; i < v1.linePoints.size(); ++i){
      sum_y_v1+=v1.linePoints[i].y;
    }

    double sum_y_v2=0;
    for (unsigned int i = 0; i < v2.linePoints.size(); ++i){
      sum_y_v2+=v2.linePoints[i].y;
    }

    double mean_y_v1=sum_y_v1/v1.linePoints.size();
    double mean_y_v2=sum_y_v2/v2.linePoints.size();
    
    if (mean_y_v1 <= mean_y_v2) return true;
    else return false;
  }
} sort_lines_func;

//-----------------------------------------------------------
class ys{
public:
  int y_min,y_max;
  ys(int y1, int y2){
    y_min=y1; 
    y_max=y2;
  }
  ys(){}
};


//----------------------------------------------------------
inline int calc_y(Point startPoint, Point endPoint, int x){

  // si hi han trams verticals
  if (startPoint.x == endPoint.x) {
    int new_y=(endPoint.y + startPoint.y)/2;
    if (new_y >= 0 ) return new_y;
    else return -new_y;
  }

  float slope= float(endPoint.y-startPoint.y)/(endPoint.x-startPoint.x);
  float desp = startPoint.y - slope * startPoint.x;

  float ytemp = slope * x + desp;
  int y = slope*x + desp;
  if(ytemp-y>0.5) // round the value of y to the closest integer
    y++;
  return y;
}

//----------------------------------------------------------
void loadPolyLines(string linesFileName,vector< vector<Point> > & lines){


  ifstream linesFile;
  linesFile.open(linesFileName.c_str());
  if(!linesFile){
    cerr << "Error: File \""<< linesFileName <<  "\" could not be open "<< endl;
    exit(-1);
  }

  float x,y;
  int clas;

  const int BUFFSIZE = 120;
  char buff[BUFFSIZE];
  stringstream line;
  int c=0;
  do{
    line.str(""); // per a buidar line
    linesFile.getline( buff,  BUFFSIZE );
    line << buff;
    if (c++ > 100) { 
      cerr << "ERROR: Point header file format wrong!!" << endl;
      exit (-1);
    }
  }while ('#' == line.peek() || line.str().size() == 0);

  int numLines;
  line >> numLines;
 
  //data=new vector<vector<Point>>(num_lines);
  int cont_line=0;
  while (cont_line < numLines){
   int c=0;
   do{
     line.str(""); // per a buidar line
     line.clear();
     linesFile.getline( buff,  BUFFSIZE );
     line << buff;
     if (c++ > 100) { 
       cerr << "ERROR: Point file format wrong!!" << endl;
       exit (-1);
       }
   }while ('#' == line.peek() || line.str().size() == 0);

   int numPoints;
   line >> numPoints;
   if (numPoints <=0){
     std::cerr<< "Warning: line "<< cont_line << " with " << numPoints << " points" << std::endl;
     continue;
   }

   int cont_points=0;
   vector<Point> points;
   while (cont_points < numPoints) { 
     int c=0;
     do{
       line.str(""); // per a buidar line
       line.clear();
       linesFile.getline( buff,  BUFFSIZE );
       line << buff;
       if (linesFile.eof()){
	 cerr << "ERROR Not enought number of points "<< cont_points << " and it is say to be " << numPoints << endl;
	 exit(-1);
       }
       if (c++ > 1000) { 
	cerr << "ERROR: Point file format wrong!!" << endl;
	exit (-1);
       }
     }while ('#' == line.peek() || line.str().size() == 0);

     line >> x >> y >> clas;
     points.push_back(Point(x,y));
     cont_points++;
   }

   cont_line++;
   // if (points.size() >= MIN_NUM_POINTS_LINE){
     lines.push_back(points);  
     //}
  }
}




//----------------------------------------------------------
vector<Point>  millorCami(Mat & rotada, vector< LineStruct>  & lines_rotades){
  //marges rotats. Valors respecte al eix de coord de la rotada
  map<int,ys> marges;

  int p_central=0, p_down=0;
  for (int x = 0; x < rotada.cols; x++) {
    if (p_central < lines_rotades[0].linePoints.size()-2 && x >= lines_rotades[0].linePoints[p_central+1].x) p_central++;
    if (p_down < lines_rotades[1].linePoints.size()-2 &&x >= lines_rotades[1].linePoints[p_down+1].x) p_down++;
   
    int y_central=calc_y(lines_rotades[0].linePoints[p_central],lines_rotades[0].linePoints[p_central+1],x);
    int y_down   =calc_y(lines_rotades[1].linePoints[p_down],   lines_rotades[1].linePoints[p_down+1],x);

    if (y_central >= rotada.rows) y_central=rotada.rows-1;
    if (y_down >= rotada.rows) y_down=rotada.rows-1;

    //if crossed
    if (y_central < y_down)
      marges[x]=ys(y_central,y_down);
    else 
      marges[x]=ys(y_down,y_central);
      
    if (marges[x].y_min + 15 > marges[x].y_max){
      if (marges[x].y_max + 15 < rotada.rows)
	marges[x].y_max = marges[x].y_max + 15;
      else {
	marges[x].y_min -= 15 - (rotada.rows - marges[x].y_max);
	marges[x].y_max = rotada.rows -1;
      }
    }

     // if (y_central > y_down) cerr << "Warning y_central = " << y_central << " y_down = " <<y_down<< endl;

    if (demo){
      //circle(rotada, Point(x,y_central), 2, Scalar(100), 2);
      //circle(rotada, Point(x,y_down), 2, Scalar(100), 2);
      circle(rotada, Point(x,marges[x].y_min),1, Scalar(100),2);
      circle(rotada, Point(x,marges[x].y_max), 1, Scalar(100), 2);
    }
  }


  if (marges.size()!= rotada.cols)
    cerr << "WARNING: size of marges structure must be "  << rotada.cols << " and is " << marges.size() << endl;

  Graf * graf=new Graf(rotada.cols*rotada.rows);
  for (int x = 0; x < rotada.cols-1; x++) {

    int y_up= marges[x].y_min;
    int y_down= marges[x].y_max;
    int marge=(int)(y_down-y_up)*0.25 + 0.5;
    //marge=0;
    for (int y = y_up+1; y< y_down-marge; y++) {
      int punt=y*rotada.cols + x;
      
      int punt_sud=(y+1)*rotada.cols + x; 
      int punt_nord=(y-1)*rotada.cols + x; 
      int punt_est=y*rotada.cols + x + 1; 
      int punt_sud_est=(y+1)*rotada.cols + x + 1;
      int punt_nord_est=(y-1)*rotada.cols + x + 1; 
      
      int pes_nord=255-rotada.at<uchar>(y,x)+ 255 - rotada.at<uchar>(y-1,x);
      int pes_sud=255-rotada.at<uchar>(y,x)+ 255 - rotada.at<uchar>(y+1,x);
      int pes_est=255-rotada.at<uchar>(y,x)+ 255 -rotada.at<uchar>(y,x+1);
      int pes_sud_est=255-rotada.at<uchar>(y,x) +  255-rotada.at<uchar>(y+1,x+1);
      int pes_nord_est=255-rotada.at<uchar>(y,x) + 255-rotada.at<uchar>(y-1,x+1);


      if (! graf->addEdge(punt, punt_nord, pes_nord) && verbosity > 1){
	cout << "NORD "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "NORD "<< punt_nord << " ("<< punt_nord %rotada.cols << "," << punt_nord/rotada.cols << ")"<< endl << endl;
      }
    
      if (! graf->addEdge(punt, punt_nord_est, pes_nord_est) && verbosity > 1 ){
	cout << "NORD-EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "NORD-EST "<< punt_nord_est << "("<< punt_nord_est %rotada.cols << "," << punt_nord_est/rotada.cols << ")"<< endl << endl;
      }

      if (! graf->addEdge(punt, punt_est,pes_est+5) && verbosity > 1 ){
	cout << "EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "EST "<< punt_est << " ("<< punt_est %rotada.cols << "," << punt_est/rotada.cols << ")"<< endl << endl;
      }
      
      if (! graf->addEdge(punt,punt_sud_est, pes_sud_est+15) && verbosity > 1 ){
	cout << "SUD-EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "SUD-EST "<< punt_sud_est << " ("<< punt_sud_est %rotada.cols << "," << punt_sud_est/rotada.cols << ")" << endl << endl;
      }

      if (! graf->addEdge(punt,punt_sud, pes_sud+20) && verbosity > 1 ){
	cout << "SUD "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "SUD "<< punt_sud << " ("<< punt_sud %rotada.cols << "," << punt_sud/rotada.cols << ")" << endl << endl;
      }   
    
    }
  }

  //dijkstra per trobar el camí més curt
  int y_ini=( marges[0].y_max - marges[0].y_min )/4 + marges[0].y_min ;
  //si al rotar fora la vora negra
  int x_ini=0;
  uchar pixel= rotada.at<uchar>(x_ini,y_ini) ;
  while (x_ini && pixel == 0) 
    x_ini++;

  int orig=y_ini* rotada.cols; //+ x_ini;
  graf->dijkstra(orig);

  int * path=graf->getPath();
  double * dist=graf->getDist();
  
  if (path== NULL || dist==NULL) return vector<Point>();

  //per a tots els finals busquem el més barat:

  //cerr<< "marges finals " << marges[rotada.cols-1].y_min << " " << marges[rotada.cols-1].y_max << endl;

  double mesBarat=DBL_MAX;
  int pos=0;
  for (int y =  marges[rotada.cols-1].y_min; y < marges[rotada.cols-1].y_max; y++){
    if (dist[rotada.cols*(y+1) -1] <= mesBarat){
      mesBarat=dist[rotada.cols*(y+1) -1];
      pos=rotada.cols*(y+1) -1;
    }
  }
  if (verbosity > 1)
    cout << "Cami mes barat, cost -> " << mesBarat << endl;
  
  //recuperar el cami    
  //vector<Point> cami(rotada.cols);
  stack<Point> cami;
  vector<Point> cami_tornar(0);  //comença el vector amb 0 posicions. Per a que el size tinga algun sentit
  int x_ant=rotada.cols;
  if (mesBarat < DBL_MAX){
    while(path[pos] != orig && path[pos]!= -1){
      int x=pos%rotada.cols;
      int y=pos/rotada.cols;

      if (x != x_ant && x < rotada.cols){
	cami.push( Point(x,y));
	x_ant=x;
      }
      pos=path[pos];
    }

    x_ant--;
    while (x_ant >=0){
      cami.push( Point(x_ant,orig/rotada.cols));

      x_ant--;
    }
  }


  delete graf;

  while (!cami.empty()) {
    cami_tornar.push_back(cami.top());
    cami.pop();
  }


  return cami_tornar;
}

//----------------------------------------------------------
bool getLineImage(vector< LineStruct > & lines,Mat & img,vector< vector<cv::Point> > & segmentacio , unsigned int lin_num, Rect regio){

  //obtenim les vores de la imatge a tallar
  int y_min=INT_MAX;
  int y_max=0;
  for (int i = 0; i < lines[lin_num].linePoints.size(); i++) 
    if (y_min > lines[lin_num].linePoints[i].y)
      y_min = lines[lin_num].linePoints[i].y;
  
  for (int i = 0; i < lines[lin_num+1].linePoints.size(); i++) 
    if ( y_max < lines[lin_num+1].linePoints[i].y)
       y_max = lines[lin_num+1].linePoints[i].y;

  if (lin_num == lines.size()-1) y_max=img.rows-1;

  if (y_min + 10 > y_max) return false;

  
  // using clone
  Mat linia=img(Rect(regio.tl().x, y_min, regio.br().x-regio.tl().x, y_max-y_min+1)).clone();

  slopeClass rotator(linia);
  linia.release();
  rotator.deslope();
   
  double angle=rotator.getSlopeAngle();
  rotator.rotar(-angle);
  Mat rotada=rotator.getRotada();
  
 //rotem els punts
  vector< LineStruct >lines_rotades(2);
  for (int l = 0; l <2; l++) 
    for (int p = 0; p < lines[lin_num + l].linePoints.size(); p++) {
      lines_rotades[l].linePoints.push_back(rotatePoint(lines[lin_num + l].linePoints[p], -angle, img));
      lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y -= y_min; //relatiu a la imatge retallada
      if (lines_rotades[l].linePoints[p].y < 0) lines_rotades[l].linePoints[p].y = 0;
      if (lines_rotades[l].linePoints[p].y >= rotada.rows) lines_rotades[l].linePoints[p].y = rotada.rows-1;
      if (lin_num+l == lines.size()-1) lines_rotades[l].linePoints[p].y = rotada.rows-1; //piso per a l'ultima linia
    }

  for (int l = 0; l < 2; l++) { 
    //afegim punt inicial a la col 0 i final a numCols-1
     lines_rotades[l].linePoints.insert(lines_rotades[l].linePoints.begin(), Point(0,lines_rotades[l].linePoints[0].y));
     if (lines_rotades[l].linePoints[0].y < 0) lines_rotades[l].linePoints[0].y = 0;
     if (lines_rotades[l].linePoints[0].y >= rotada.rows) lines_rotades[l].linePoints[0].y = rotada.rows-1;

     lines_rotades[l].linePoints.push_back(Point(rotada.cols-1, lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y));
     lines_rotades[l].name = lines[l].name;
     
     if (lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y < 0) lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y = 0;
     if (lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y >= rotada.rows) lines_rotades[l].linePoints[lines_rotades[l].linePoints.size()-1].y = rotada.rows-1;
  }

  static vector<Point> cami_dalt;
  static vector<Point> cami_baix;

  if (cami_baix.size() > 0)
    cami_dalt=cami_baix;
  
  if (lin_num < lines.size()-1)
    cami_baix=millorCami(rotada, lines_rotades);

  if  (cami_baix.size() <= 0){
    if (verbosity > 1)
      cerr << "Cami de baix, not found in line "<< lin_num << endl;
    return false;
  } else 
    for (int i = 0; i < cami_baix.size(); i++) {
      cami_baix[i]=Point(rotatePoint(cami_baix[i],angle,rotada));
      cami_baix[i].y+=y_min; //relatiu a la imatge general
      cami_baix[i].x+=regio.tl().x;
      if (cami_baix[i].y < 0) cami_baix[i].y=0;
    }
  
  segmentacio.push_back(cami_baix);

  return true;
}
//----------------------------------------------------------
void fussionLines(vector< vector<Point> > & lines, Mat & img){

//espacio interlinea promedio
  double media=0,media_total=0,ant=0;;
  for (int lin_num = 0; lin_num < lines.size()-1; lin_num++) {
    ant=media;
    for (int p = 0; p < lines[lin_num].size(); p++) {
      media+=lines[lin_num][p].y;
    }
    media=media/lines[lin_num].size();
    if (lin_num > 0)
      media_total+=(media - ant);
  }

  media_total=media_total/lines.size();


  bool crossed=false;

  for (int lin_num = 0; lin_num < lines.size()-1; lin_num++) {
    
    double media_central=0,media_down=0;

    for (int p = 0; p < lines[lin_num].size(); p++) {
      media_central+=lines[lin_num][p].y;
    }
    media_central/=lines[lin_num].size();

    for (int p = 0; p < lines[lin_num+1].size(); p++) {
      media_down+=lines[lin_num+1][p].y;
    }
    media_down/=lines[lin_num+1].size();

    //if (media_central + 10 > media_down){
    if (media_down - media_central < media_total/3){
      // if (verbosity){
      // 	cerr << "WARNING: lines "<< lin_num << " and " <<lin_num+1 << " crossed" << endl;
      // 	cerr << "media_central = " << media_central << " media_down = " << media_down<< endl;
      // 	cerr << "      size 1 " << lines[lin_num].size() << endl;
      // 	cerr << "      size 2 " << lines[lin_num+1].size() << endl;
      // }

      //afegix els punts de la segona linia a la primera
      lines[lin_num].insert(lines[lin_num].end(), lines[lin_num+1].begin(), lines[lin_num+1].end());
      std::sort(lines[lin_num].begin(), lines[lin_num].end(), sort_points_func);
      
      //esborra la linia sobrant
      lines.erase(lines.begin() + lin_num+1);
            
      // if (verbosity){
      // 	cerr <<"      Num de linies " << lines.size() << endl << endl;
      // 	cerr <<"      total  "<< lines[lin_num].size() << endl<< endl;
      // }
      lin_num--; // per a repetir la linia
      crossed=true;
      break; //tornem a procesarla
    }
  }

  if (crossed)
    fussionLines(lines,img);
}
//----------------------------------------------------------
void purgarSegmentacio(  vector< vector<Point> >segmentacio, Mat img){
  for (int c = 0; c < segmentacio.size();c++) {
      for (int i = 0; i < segmentacio[c].size()-1; i++) {
	if (segmentacio[c][i].x == segmentacio[c][i+1].x){
	  if (segmentacio[c][i].y < segmentacio[c][i+1].y)
	    segmentacio[c].erase(segmentacio[c].begin() + i);
	  else
	    segmentacio[c].erase(segmentacio[c].begin() + i+1);
	
	  i--;
	}
      }
 
  }
}

//----------------------------------------------------------
void pintaLinies(Mat & img, bool deBatABat,  vector <LineStruct>  lines,vector< map<int,int> > seg_lines){
  
  //pinta el poligon al voltant de les frases
  int x_ini=0;
  int x_fi=img.cols-1;
  deBatABat=false;
  
    //per a la primera linia
     if (!deBatABat){
       x_ini=lines[0].linePoints[0].x - MARGE;
       if (x_ini < 0) x_ini=0;	

	x_fi=lines[0].linePoints[lines[0].linePoints.size()-1].x+MARGE;
      	if (x_fi > img.cols) x_fi=img.cols-1;
     }
     for (int x = x_ini; x <x_fi; ++x)
	  circle(img, Point(x,seg_lines[0][x]),1, Scalar(100),1);

     //per a la resta de la pagina
     x_ini=0;
     x_fi=img.cols-1;
     
    for (int lin = 1; lin < seg_lines.size(); lin++) {
      if (!deBatABat){
	x_ini=lines[lin].linePoints[0].x < lines[lin-1].linePoints[0].x ? lines[lin].linePoints[0].x : lines[lin-1].linePoints[0].x ;
	x_ini-=MARGE;
      	if (x_ini < 0) x_ini=0;

      	x_fi=lines[lin].linePoints[lines[lin].linePoints.size()-1].x > lines[lin-1].linePoints[lines[lin-1].linePoints.size()-1].x ? lines[lin].linePoints[lines[lin].linePoints.size()-1].x : lines[lin-1].linePoints[lines[lin-1].linePoints.size()-1].x;
	x_fi+=MARGE;
      	if (x_fi > img.cols) x_fi=img.cols-1;
      }
      
      for (int x = x_ini; x <x_fi; ++x)
	circle(img, Point(x,seg_lines[lin][x]),1, Scalar(100),1);
    
    }

}

//----------------------------------------------------------
void use (char * programName){

  cerr << "Usage: "<<programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -x #String; inputPageXmlFile" << endl;
  cerr << "             -i #String;  inputImageFile" << endl;
  cerr << "             -r #int; final number of rows (by default 64). -1 original size]" << endl;
  //cerr << "             [-l #int line to be extracted (by default all)]" << endl;
  cerr << "             [-o #String ] outputImageFile " << endl;
  cerr << "             [-d ] demo (by default false)" << endl;
  //  cerr << "             [-m #int ] (by default 4)] min num points per line" << endl;
  // cerr << "             [-t ] segment just the baseline (by default no)"<< endl;
  cerr << "             [-v #int ] level of verbosity (by default 0)]" << endl;

}

Mat cropVertical(Mat & image){

  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  
  Mat img(image.size(), image.type());
  
  int pad=3;
  copyMakeBorder( image, img, pad, pad, pad,  pad , BORDER_CONSTANT, Scalar(255) );
  
  cv::threshold( img,img, 0, 255, THRESH_BINARY  | THRESH_OTSU);
  
   
  Mat img2(image.size(), image.type());
  img.copyTo(img2);
  
  cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT , cv::Size(7, 3));
  cv::erode(img, img, element);
  
  findContours(img, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0) );
  
   /// Find the convex hull object for each contour
  vector<vector<Point> >hull( contours.size() );
  vector<Point> contour;
  float superficieMax=0;
  int contourMaxSuperficie=1;

  for( uint i = 0; i < contours.size(); i++ )  {     
    if ( hierarchy[i][3] != -1 && contours[i].size() > 8){
      convexHull( Mat(contours[i]), hull[i] );
      approxPolyDP(Mat(hull[i]), contour, 0.001, true);
      float sup =  contourArea(hull[i]);
      int y_max=0, y_min=img.rows-1;
      int x_max=0, x_min=img.cols-1;
      for (uint h = 0; h < hull[i].size(); h++){
	int y = hull[i][h].y;
	int x = hull[i][h].x;
	if (y > y_max) y_max = y;
	if (y < y_min) y_min = y;
	if (x > x_max) x_max = x;
	if (x < x_min) x_min = x;
      }

      float asp_ratio =  ((y_max - y_min) / (float)(x_max -x_min));
      if (sup > superficieMax && asp_ratio > 0.2){
	superficieMax=sup;
	contourMaxSuperficie = i;
      }      
    }
  }
 
  
  int altura = -1;
  int y_orig = 0;
  int max_y=-1, min_y=100000;
  
  for (uint h = 0; h < hull[contourMaxSuperficie].size(); h++) {
    int y = hull[contourMaxSuperficie][h].y;
    if (y > max_y) max_y = y;
    if (y < min_y) min_y = y;
  }
     
  if (max_y-min_y > altura){
    altura = max_y-min_y;
    y_orig = min_y;
  }
  
  altura = altura > -1 ? altura: img.rows;
       
  y_orig = y_orig - pad >= 0 ? y_orig-pad: 0;
  altura = altura + y_orig < image.rows ? altura: image.rows - y_orig;
  Rect rectCrop(0, y_orig, image.cols-1, altura);       
  image = Mat(image, rectCrop);
  

  return image;
}

//----------------------------------------------------------
int main(int argc,char** argv ) {
  string inFileName="",xmlFileName="",outputFileName="";
  int option, finalNumberRows=64;;
  bool deBatABat=true;

  while ((option=getopt(argc,argv,"i:x:dm:o:v:r:"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 'x':
      xmlFileName = optarg;
      break;
    case 'o':
      outputFileName = optarg;
      break;
    case 'r':
        finalNumberRows=atoi(optarg);
        break;

    case 'd':
      demo=true;
      break;
    case 't':
      deBatABat=false;
      break;
    // case 'm':
    //   MIN_NUM_POINTS_LINE = atoi(optarg);
    //   break;
    case 'v':
      verbosity=atoi(optarg);
      break;
    default:
      use(argv[0]);
      return(-1);
    }

  if (xmlFileName.size()==0){
    cerr << argv[0] << " Error: points file name must be provided" << endl;
    use(argv[0]);
    return (-1);
  }
 
  Mat img;
  if (inFileName.size()!=0){
    img = imread(inFileName,0);
  }
  if (!img.data) {
    cerr << argv[0] << ": ERROR: image file " << inFileName.c_str() << " do not contains data " << endl;
    return (-1);
  }

 //li llevem la extensio al nom de fitxer d'entrada
  size_t found = inFileName.find_last_of(".");
  inFileName.erase(found,-1);

  if(outputFileName.size() > 0){
    found = outputFileName.find_last_of(".");
    if (found > -1)
      outputFileName.erase(found,-1);
  }

  pugi::xml_document page;
  pugi::xml_parse_result result = page.load_file(xmlFileName.c_str());
  if (!result){
    cerr << "ERROR reading \"" << xmlFileName << "\""<< endl;
    cerr << "\t" << result.description()<< endl;
    exit(-1);
  }
 
 //llevem el skew de la pagina
 slopeClass desquewer(img);
 desquewer.deslope();
 
 double angleSlope=desquewer.getSlopeAngle();
 
 Mat rot_mat = getRotationMatrix2D(Point2f(img.cols/2.0, img.rows/2.0), -angleSlope, 1.0);
 Mat img_rotated;
 img_rotated.create(img.rows,img.cols, CV_8UC(255));
 
 warpAffine(img, img_rotated, rot_mat,img.size(), cv::INTER_CUBIC, cv::BORDER_CONSTANT, Scalar(255,255,255));
 

 vector< vector<Point> >segmentacio(0);
 vector <cv::Point>  region;
 vector <LineStruct>  lines;
 int cont_lin=0, cont_reg=0;
 for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){

   region.clear();
   getRegionCoords(text_region, region);
   
   lines.clear();
   getBaselinesFromRegion_id(text_region, lines);
   
   if (verbosity > 0)
     cout << "Region " << cont_reg << " Number of lines " << lines.size() << endl;
   
   if (lines.size()==0) continue; // si una regió no té linies...

   //ordenem els punts d'esquerre a dreta
   for (uint l = 0; l < lines.size(); l++) {       
      std::sort(lines[l].linePoints.begin(), lines[l].linePoints.end(), sort_points_func);
    }  
   
   //rotem els punts
   for (int l = 0; l < lines.size(); l++) 
     for (int p = 0; p < lines[l].linePoints.size(); p++) 
       lines[l].linePoints[p]=rotatePoint(lines[l].linePoints[p], -angleSlope, img);
   
 
   //ordenem les linies
   std::sort(lines.begin(), lines.end(), sort_lines_func);

   //fussionem  linies base
   //fussionLines(lines, img_rotated);
   
   int minX=img_rotated.cols-1;
   int maxX=0;
   int minY=img_rotated.rows-1;
   int maxY=0;
   
   for (uint p = 0; p < region.size(); p++) {
     region[p]=rotatePoint(region[p], -angleSlope, img_rotated);
     
     if (minX > region[p].x)
       minX = region[p].x;
     if (maxX <  region[p].x)
       maxX = region[p].x;
     if (minY > region[p].y)
       minY = region[p].y;
     if (maxY <  region[p].y )
       maxY = region[p].y;
   }
   if (minX < 0 ) minX = 0;
   if (minY < 0 ) minY = 0;
   if (maxX > img.cols ) maxX = img.cols-1;
   if (maxY > img.rows ) maxY = img.rows-1;
   
  
 //afegim una linia artificial baix de tot per a poder tindre vora en la segmentacio
  vector<Point> ultimaLinia;
  ultimaLinia.push_back(Point(minX, maxY));
  ultimaLinia.push_back(Point(maxX, maxY));
  lines.push_back(LineStruct("ultima",ultimaLinia));
 
  
  // obtenim la segmentacio
  cv::Rect rect(Point(minX,minY),Point(maxX,maxY));
  segmentacio.clear();
  for (int l = 0; l < lines.size()-1; l++) {      
    if (verbosity > 1)
      cout << endl<< "processing line "<< l+1<< endl;
    
    getLineImage(lines, img_rotated, segmentacio, l, rect);
  }

  //afegim una linia inicial damunt de la primera i igual a ella
  vector <cv::Point> linia_zero;
  linia_zero.push_back(Point(minX,minY));
  linia_zero.push_back(Point(maxX,minY));
  segmentacio.insert(segmentacio.begin(), linia_zero);
  
  vector< map<int,int> > seg_lines;

  //LES X'S ES DESPLACEN AL ROTAR
  for (int l = 0; l < segmentacio.size(); l++){ 
    map<int,int> seg;
    for (int p = 0; p < segmentacio[l].size(); p++) 
      seg[segmentacio[l][p].x] = segmentacio[l][p].y;  
    
    
    Point primer(segmentacio[l][0]);
    if (primer.x > 0){
      int x_ini=primer.x - 1;
      //for (int x = x_ini; x >=0 ; x--)
      for (int x = x_ini; x >=minX ; x--)
        seg[x]=primer.y;
    }

    Point ultim(segmentacio[l][segmentacio[l].size()-1]);
    if (ultim.x < img.cols - 1){
      for (int x = ultim.x+1; x < maxX; x++)
	seg[x]=ultim.y;
    }
    seg_lines.push_back(seg);   
  }
  
  //al rotar es creen discontinuitats en les x
  for (int l = 0; l < seg_lines.size(); l++)
    for (int x = minX+1; x < maxX; x++) {
      if(seg_lines[l].find(x) == seg_lines[l].end())
	seg_lines[l][x]=seg_lines[l][x-1];
    }
  

  if (demo){
    pintaLinies(img_rotated,deBatABat, lines, seg_lines);
    string demoFileName;
    if (outputFileName.size() >0)
      demoFileName=outputFileName;
    else
      demoFileName=inFileName+"_segmented.jpg";

    imwrite(demoFileName,img_rotated);
    
  } else {

    for (uint seg = 0; seg < segmentacio.size()-1; seg++) {

      int xini=rect.tl().x ;
      int xfi=rect.br().x ;

      if (!deBatABat){
	xini=lines[seg].linePoints[0].x ;
	if (xini < 0) xini=0;
	xfi=lines[seg].linePoints[lines[seg].linePoints.size()-1].x ;
	if (xfi > img_rotated.cols) xfi=img_rotated.cols-1;
      }

      int y_min= img_rotated.rows;
      int y_max = 0;

     		  
      for (int x = xini; x < xfi; x++){
	 if (y_min > seg_lines[seg][x]) y_min = seg_lines[seg][x];
	 if (y_max < seg_lines[seg+1][x]) y_max = seg_lines[seg+1][x];
      } 

      Mat img_lin(y_max-y_min+1, xfi-xini,  CV_8UC1, Scalar(255));

      for (int x = xini; x < xfi; x++)
	if ( seg_lines[seg].find(x) != seg_lines[seg].end())
	  for (int y = seg_lines[seg][x]; y < seg_lines[seg+1][x]; y++) 
	    if ( y < img_rotated.rows)
	      img_lin.at<uchar>(y-y_min, x-xini) = img_rotated.at<uchar>(y,x); 

      if (!deBatABat){
	int x = lines[seg].linePoints[0].x;
	int x2 =lines[seg].linePoints[lines[seg].linePoints.size()-1].x;
	//cout << " x " << x << " x2 " << x2 << " " << img_lin.size() << endl;
	img_lin = img_lin(Rect(Point(x,0),Point(x2, img_lin.size().height-1)));
      }
      std::stringstream num_linea_str;
      std::stringstream outFileName_line;
      num_linea_str << setw(2) << setfill('0') <<cont_lin;
      if (outputFileName.size() > 0){
	outFileName_line << outputFileName;
      }else{
	outFileName_line << inFileName; 
      }


      //      text_region.child("Coords").attribute("points").value();
      //      outFileName_line  << text_region.attribute("id").value();
      //outFileName_line <<"_"<< num_linea_str.str()<< "_"<< lines[cont_lin].name << ".jpg";
      outFileName_line << "_" << lines[cont_lin].name << ".jpg";
      cont_lin++;


      img_lin = cropVertical(img_lin);
      if (finalNumberRows > 0){
	float factor=float(finalNumberRows)/img_lin.rows;
	cv::resize(img_lin, img_lin, cv::Size(int(img_lin.cols*factor), finalNumberRows), 0, 0, INTER_LANCZOS4);// CV_INTER_LINEAR);
      }

      bool write_result=false;
      try{
	write_result = imwrite(outFileName_line.str(),img_lin);
      } catch (const cv::Exception& ex)	{
        cerr <<  "Exception writing imaget: %s\n"<< ex.what()<< endl;
      }
      if (!write_result){
	cerr << "ERROR: output file "<< outFileName_line.str() << " can not be created " << endl;
      }
      img_lin.release();
    }
  }
  cont_reg++;
 }
 
}
