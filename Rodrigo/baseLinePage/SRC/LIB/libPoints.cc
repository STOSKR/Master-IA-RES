#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;
//----------------------------------------------------------
int calc_y(Point startPoint, Point endPoint, int x){

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

//---------------------------------------------------------------------
Point rotatePoint(Point p,double angle,Mat & img){
  angle=angle*M_PI/180.0;

  double x = p.x - img.cols/2;
  double y = -p.y + img.rows/2;
 
  double x1 = x*cos(angle) - y*sin(angle);
  double y1 = x*sin(angle) + y*cos(angle);

  x =  (int)(x1+0.5) + img.cols/2;
  y = (int)(-y1-0.5) + img.rows/2;

  return Point(x,y);
}

//---------------------------------------------------------------------
Point rotatePoint(Point p,double angle, int rows, int cols){
  if (angle == 0) return p;
  
  angle=angle*M_PI/180.0;

  int x = p.x - cols/2;
  int y = -p.y + rows/2;

  int x1 = x*cos(angle) - y*sin(angle);
  int y1 = x*sin(angle) + y*cos(angle);

  x =  x1 + cols/2;
  y = -y1 + rows/2;

  return Point(x,y);
}


//---------------------------------------------------------------------
int getPointsFromFile(string pointsFileName,vector<Point> & points){

  ifstream pointsFile;
  pointsFile.open(pointsFileName.c_str());
  if(!pointsFile){
    cerr << "Error: File \""<< pointsFileName <<  "\" could not be open "<< endl;
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
    pointsFile.getline( buff,  BUFFSIZE );
    line << buff;
    if (c++ > 100) { 
      cerr << "ERROR: Point header file format wrong!!" << endl;
      exit (-1);
    }
  }while ('#' == line.peek() || line.str().size() == 0);

  int numPoints;
  line >> numPoints;
  int cont_points=0;
  int x_min=INT_MAX;
  int x_max=0;
  while (cont_points < numPoints) {
    c=0;
    do{
      line.str(""); // per a buidar line
      line.clear();
      pointsFile.getline( buff,  BUFFSIZE );
      line << buff;
      if (pointsFile.eof()){
	cerr << "ERROR Not enought number of points "<< cont_points << " and it is say to be " << numPoints << endl;
	exit(-1);
      }
      if (c++ > 100) { 
	cerr << "ERROR: Point file format wrong!!" << endl;
	exit (-1);
      }
    }while ('#' == line.peek() || line.str().size() == 0);

    line >> x >> y >> clas;
    if (clas == 2){ //baseline 
      points.push_back(Point2f(x,y));

      if (x > x_max) x_max=x;
      if (x < x_min) x_min=x;
    }
    cont_points++;
  }
  
  return (x_max - x_min);
}
//---------------------------------------------------------------------
int getPointsFromFile(string pointsFileName,vector<Point3d> & points){

  ifstream pointsFile;
  pointsFile.open(pointsFileName.c_str());
  if(!pointsFile){
    cerr << "Error: File \""<< pointsFileName <<  "\" could not be open "<< endl;
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
    pointsFile.getline( buff,  BUFFSIZE );
    line << buff;
    if (c++ > 100) { 
      cerr << "ERROR: Point header file format wrong!!" << endl;
      exit (-1);
    }
  }while ('#' == line.peek() || line.str().size() == 0);

  int numPoints;
  line >> numPoints;
  int cont_points=0;
  int x_min=INT_MAX;
  int x_max=0;
  while (cont_points < numPoints) {
    c=0;
    do{
      line.str(""); // per a buidar line
      line.clear();
      pointsFile.getline( buff,  BUFFSIZE );
      line << buff;
      if (pointsFile.eof()){
	cerr << "ERROR Not enought number of points "<< cont_points << " and it is say to be " << numPoints << endl;
	exit(-1);
      }
      if (c++ > 100) { 
	cerr << "ERROR: Point file format wrong!!" << endl;
	exit (-1);
      }
    }while ('#' == line.peek() || line.str().size() == 0);

    line >> x >> y >> clas;
   
    points.push_back(Point3d(x,y,clas));

    if (x > x_max) x_max=x;
    if (x < x_min) x_min=x;
      
    cont_points++;
  }
  
  return (x_max - x_min);
}

//per al sort
bool compare_pointsX(Point a, Point b){
  return a.x < b.x;
}
bool compare_pointsX3d(Point3d a, Point3d b){
  return a.x < b.x;
}
//--
//-------------------------------------------------------------
void writePointsToFile(string outFileName,vector<Point>  selected_min){
  ofstream minFile;

  std::sort(selected_min.begin(), selected_min.end(), compare_pointsX);
  minFile.open(outFileName.c_str());

  if (!minFile){
    cerr << "Error creating "<< outFileName << endl;
    exit (-1); 
  }

  // writing head min points file
  minFile << "# Number and class of extrema points" << endl;
  minFile << selected_min.size() << " ";
  minFile << "Min" << endl;
  minFile << "# Points" << endl;

  // writing min points
  sort(selected_min.begin(), selected_min.end(), compare_pointsX);
  for(unsigned int i=0;i<selected_min.size();i++)
    minFile << selected_min[i].x << " " << selected_min[i].y << endl;

  // clossing files
  minFile.close();
}

//-------------------------------------------------------------
void writePointsToFile(string outFileName,vector<Point3d> & selected_min){
  ofstream minFile;

  minFile.open(outFileName.c_str());

  if (!minFile){
    cerr << "Error creating "<< outFileName << endl;
    exit (-1); 
  }

  // writing head min points file
  minFile << "# Number and class of extrema points" << endl;
  minFile << selected_min.size() << " ";
  minFile << "Min" << endl;
  minFile << "# Points" << endl;

  // writing min points
  sort(selected_min.begin(), selected_min.end(), compare_pointsX3d);
  for(unsigned int i=0;i<selected_min.size();i++)
    minFile << selected_min[i].x << " " << selected_min[i].y << " " << selected_min[i].z <<endl;

  // clossing files
  minFile.close();
}

//--------------------------------------------------------------------
void normaliza_traza(vector<cv::Point> & baseline, int NumPuntsNuevo=10){
  vector<Point> puntos(baseline);
 
 if (baseline.size()<=0) { 
      cerr << "WARNING: baseline with "<< baseline.size()<< " points" << endl;
      return;
  }
 
  baseline.clear();

  // calculamos el vector de distancias desde el punto inicial a cualquier otro
  int DistEntrePuntos;
  double * D=new double[puntos.size()];
  D[0]=0;
  for (uint j = 1; j < puntos.size(); j++) {
    int DX=(puntos[j].x - puntos[j-1].x);
    int DY=(puntos[j].y - puntos[j-1].y);
    D[j]=D[j-1]+sqrt((double)DX*DX+DY*DY);
  }
  
  // si no se recibe, se distribuyen los puntos que ya hay equidistantemente
  if (NumPuntsNuevo == 0){ 
    NumPuntsNuevo=puntos.size();
  }

  DistEntrePuntos=D[puntos.size()-1]/(NumPuntsNuevo-1);
  
  // ponemos el punto inicial donde estaba el original
   baseline.push_back(puntos[0]);
  int n=1;
    
  for (int j = 1; j < NumPuntsNuevo-1; j++) {
    while (!((D[n-1] <= j*DistEntrePuntos) && (j*DistEntrePuntos <= D[n]))) n++;
    float C;
    if (D[n-1]==D[n]) C=1;
    else C=(j*DistEntrePuntos-D[n-1])/(D[n]-D[n-1]);
    
    float TX=puntos[n-1].x + (puntos[n].x - puntos[n-1].x)*C;
    float TY=puntos[n-1].y + (puntos[n].y - puntos[n-1].y)*C;
    
    Point p_nuevo(int(TX+0.5), int(TY+0.5));
    baseline.push_back(p_nuevo);
  }

  // ponemos el punto final donde estaba el original
  baseline.push_back(puntos[puntos.size()-1]);
}

