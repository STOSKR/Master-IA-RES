// compile: g++ -o adjustParam adjustParam.cc   -lopencv_imgproc -lopencv_core -lopencv_highguiq

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <opencv2/opencv.hpp>
#include "/usr/include/opencv4/opencv2/opencv.hpp"

using namespace cv;
using namespace std;


class LocalMinima{

public:
  Mat img;
  int window;
  int kernel_size;
  int pTallMin;
  bool verbosity;
  //static const int max_thresh = 400;
  LocalMinima(String inFilename,int window,int pTallMin, int kernel_size, bool verbosity);
  void preprocessing(Mat & img);
  void getMinimaPoints();
  void getMinimaPoints(Mat & aux);
  void pintaPunts(Mat &);
  Mat pintaPuntsEnContorns(Mat & img_gray);
  void writeMinimaToFile(String inFileName);
private: 
  vector<Point> selected_min;
  vector<Point> * purge(vector<Point> &points);  
  bool exterior(Point p, bool isUp);
  void findMinInContours (vector<Point> & points);
};

//-----------------------------------------------------------
//-----------------------------------------------------------
void callback(int v, void *ptr){
  LocalMinima *that = (LocalMinima*)ptr;
  Mat img_gray= that->img.clone();

  that->preprocessing(img_gray);
  that->getMinimaPoints(img_gray);
 
  that->pintaPunts(img_gray);
  imshow( "Minima control",img_gray);
 
  img_gray.release();  
}

//-----------------------------------------------------------
void usage(char * nomProg){
  cerr << "Usage: "<<nomProg << " options" << endl;
  cerr << "      options:" << endl;
  cerr << "             -i inputfile" << endl;
  cerr << "            [-t minum number of points per contourn (by default 5)]" << endl;
  cerr << "            [-w width of half the window (by default 30)]" << endl;
  cerr << "            [-v verbosity(by default none)]" << endl;
}
//-----------------------------------------------------------
int main( int argc, char** argv ) {
  string inFileName,outFileName;

  int option, window=30, kernel_size=4 ;
  unsigned int pTallMin=20; 
  bool verbosity=false;
  bool graphicalMode=false;

 if(argc == 1){
    usage(argv[0]);
    return -1;
  }
  while ((option=getopt(argc,argv,"h:i:o:t:w:k:gv"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 'o':
      outFileName = optarg;
      break;
    case 't':
      pTallMin=atoi(optarg);
      break; 
    case 'w':
      window=atoi(optarg);
      break;
    case 'k':
      kernel_size=atoi(optarg);
      if (kernel_size < 2) kernel_size=2;
      break;
    case 'g':
      graphicalMode=true;
      break;
    case 'v':
      verbosity=true;
      break;
    case 'h':
    default:
      usage(argv[0]);
      exit(1);
    }
  
  LocalMinima loc(inFileName, window, pTallMin, kernel_size, verbosity);
  
  if (graphicalMode){
           
    const char* main_window = "Minima control";
    namedWindow(main_window, WINDOW_GUI_EXPANDED);    
    resizeWindow(main_window, 1600,1200);    
    
    createTrackbar("win context:", main_window, &(loc.window), 100, callback, &loc);
    createTrackbar( "Boud size context:", main_window, &(loc.pTallMin), 300, callback, &loc);
    createTrackbar( "Erosion kernel size:", main_window, &(loc.kernel_size), 100, callback,  &loc);
    setTrackbarPos( "Erosion kernel size:",main_window ,kernel_size );
 
  
    imshow(main_window, loc.img);
  
    const char ESC = (char)27;
    char k;
    do
      k=waitKey(0);
    while (k != ESC);

    //ara sobre la imatge original
    loc.preprocessing(loc.img);    
    loc.getMinimaPoints();
    
    if (outFileName.size() > 0){
      loc.writeMinimaToFile(outFileName);
    }

    cout << "context_win = " << loc.window << " prunning size = " << loc.pTallMin << " kernel size = " << loc.kernel_size << endl;
  } else {
    // cout << "context_win = " << loc.window << " prunning size = " << loc.pTallMin << " kernel size = " << loc.kernel_size << endl;

     loc.preprocessing(loc.img);    
     loc.getMinimaPoints();
    
     if (outFileName.size() == 0){
       // output files
       int pos = inFileName.find_last_of(".");
       string minFileName = inFileName.substr(0,pos)+".min";
       loc.writeMinimaToFile(minFileName);
     }else
       loc.writeMinimaToFile(outFileName);
  }

  return(0);
}


//-------------------------------------------------------------
LocalMinima::LocalMinima(String inFileName,int window=10,int pTallMin=30, int kernel_size=5, bool verbosity=false ){
  this->window = window;
  this->pTallMin=pTallMin;
  this->kernel_size = kernel_size;
  this->verbosity = verbosity;
  
  img=imread( inFileName.c_str(), IMREAD_GRAYSCALE );
  if (!img.data) {
    cerr << "ERROR reading the image file "<< inFileName<< endl;
    exit(-1);
  }

  
  threshold(img, img, 0, 255, THRESH_BINARY | THRESH_OTSU);
  
  //dilatacio vertical
  Mat kernel_v = getStructuringElement( MORPH_RECT,  Size(3,1),  Point(1, 0 ) );
  erode(img, img, kernel_v);
 
 }

//-------------------------------------------------------------
void LocalMinima::preprocessing(Mat & img_gray){
  //threshold(img_gray, img_gray, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
  if (kernel_size == 0)
    kernel_size=1;
  
  Mat kernel = getStructuringElement( MORPH_RECT,  Size(abs(kernel_size),1),  Point(abs(kernel_size/2),0 ) );
   
   dilate(img_gray, img_gray, kernel); //como la imagen está invertida, esto es una erosión
   //erode(img_gray,img_gray,kernel);
   
   GaussianBlur(img_gray, img_gray, Size( 5, 5 ),3,3);
   
   threshold(img_gray, img_gray, 0, 255, THRESH_BINARY | THRESH_OTSU);

}
//-------------------------------------------------------------
void LocalMinima::getMinimaPoints(){
  Mat img_gray = img.clone();
  getMinimaPoints(img_gray);
}
//-------------------------------------------------------------
void LocalMinima::getMinimaPoints(Mat & img_aux){
  selected_min.clear();

  // obtain contours
  vector<vector<Point> > contours(0);
  vector<Vec4i> hierarchy(0);

  
  findContours(img_aux, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);

  
  for (unsigned int c=0; c < contours.size(); c++)
    if (contours[c].size() > pTallMin && hierarchy[c][3] != -1  )  //hierarchy[c][3] != -1 -> no parents
      findMinInContours (contours[c]);      


  if (verbosity){   
    Mat image(img_aux.size(),CV_8U,cv::Scalar(255));
    for (unsigned int c=0; c < contours.size(); c++){
      vector<Point> * purged = purge(contours[c]);
      for (unsigned int c=0; c < purged->size(); c++){      
    	image.at<uchar>((*purged)[c].y, (*purged)[c].x)=0;	
      }
    }
    
    // for (unsigned int c=0; c < selected_min.size(); c++){      
    //         image.at<uchar>(selected_min[c].y, selected_min[c].x)=0;	    
    // } 
  
  
    imwrite("contours.jpg", image);
  }

}
//-----------------------------------------------------------
void LocalMinima::findMinInContours (vector<Point> & points){


  vector<Point> * purged = purge(points);

  for ( int p=0; p<purged->size(); p++ ) {
    bool  min=true, context=false;
    
     for (int j = 0; j < purged->size(); j++)
      if (abs((*purged)[j].x - (*purged)[p].x) <= window/2 && abs((*purged)[j].y - (*purged)[p].y) <= window){      
  	  context=true;	 
  	  if ((*purged)[j].y > (*purged)[p].y){
  	    min=false;
	    break;
	  }
  	}
      
    
    if (context && min) // && contPuntsDinsWin>=MinNumPuntsInWin)
       if(exterior((*purged)[p], false))
  	 selected_min.push_back((*purged)[p]);	
      
  }

  delete(purged); 
  
}


//-----------------------------------------------------------


//per al sort
bool compare_pointsX(Point a, Point b){
  return a.x < b.x;
}
//-----------------------------------------------------------
vector<Point> * LocalMinima::purge(vector<Point> &points){

  vector<Point>::iterator it = points.begin();
  //llevem duplicats
   std::sort(points.begin(), points.end(), compare_pointsX);
   it = points.begin();
   while(it != points.end()){
     if ((*it).y == (*(it+1)).y && (*it).x == (*(it+1)).x) {
      points.erase(it);
    }else{
      it++;
    }
  }

  //creem un multimap per trobar tots el punts amb una 'y' determinada
  multimap<int,int> map;
  it = points.begin();

  int y_max=0, y_min=INT_MAX;
  while(it != points.end()){
    map.insert (pair<int,int>((*it).y, (*it).x)); 
    if ((*it).y > y_max) y_max=(*it).y;
    if ((*it).y < y_min) y_min=(*it).y;
    it++;
  }


  vector<Point> * aDevolver = new vector<Point> ();

  //for (int row=0; row< image.rows; row++){
  for (int row=y_min; row<=y_max; row++){
    pair <multimap<int,int>::iterator, multimap<int,int>::iterator> ret;
    ret = map.equal_range(row);
    int numPoints=map.count(row);

    if (numPoints > 1){
      vector<Point> rep; 

      //convertim ret (y iguals a row) a un vector (rep)
      for (multimap<int,int>::iterator it2=ret.first; it2!=ret.second; ++it2){
	rep.push_back(Point(it2->second, it2->first));
      }
      
      //cada altura (y) ordenat per columna (x)
      std::sort(rep.begin(), rep.end(), compare_pointsX);

      //punts consecutius a la mateixa altura -> deixem el del mig
      it = rep.begin();
      int contPoints=0;
      while(contPoints < numPoints){
	int cont =1;
	while (cont <= numPoints && (*(it+cont-1)).x == (*(it+cont)).x-1){
	  cont++;
	  contPoints++;
	}
	//seleccionats.insert (Point((*it).x + (cont)/2,(*it).y));
	aDevolver->push_back(Point((*it).x + (cont)/2,(*it).y));

	if (cont == 1)
	  it++;
	else 
	  it+=(cont);
      
	contPoints++;
      }
    } else if ( numPoints == 1){
      aDevolver->push_back(Point(ret.first->second, row));
    }
  }
  
  return aDevolver;
}

//-----------------------------------------------------------
bool LocalMinima::exterior(Point p, bool isUp){
  float cont_down=0,cont_up=0;

  for (int y=p.y; y<img.rows && y<p.y+5; y++)
    cont_down+= img.at<uchar>(y,p.x);

  for (int y=p.y; y>0 && y>p.y-5; y--)
    cont_up+= img.at<uchar>(y,p.x);

  
  if (isUp) return cont_up > cont_down;
  else return cont_up < cont_down;


}

//-------------------------------------------------------------
void LocalMinima::pintaPunts(Mat & img_gray){
    //pinta punts
  img_gray = img_gray^0xFF;
  
    int point_shape=5;
    for (int i = 0; i < selected_min.size(); i++) {     
      circle( img_gray, selected_min[i], 2, Scalar(25), point_shape );
    }

}

//-------------------------------------------------------------
Mat LocalMinima::pintaPuntsEnContorns(Mat & img_gray){
  cv::Mat image(img_gray.size(),CV_8U,cv::Scalar(255));

 // obtain contours
  vector<vector<Point> > contours(0);
  vector<Vec4i> hierarchy(0);

  findContours(img_gray, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);
    for (unsigned int c=0; c < contours.size(); c++){
      if ( hierarchy[c][3] != -1 ||hierarchy[c][3]==0 ) 
        if (contours[c].size() > pTallMin)
          for(unsigned int p=0; p<contours[c].size(); p++ ) {
            if (contours[c][p].y > 1 && contours[c][p].y < image.rows - 2 && contours[c][p].x > 1 && contours[c][p].x < image.cols - 2) {
              image.at<uchar>(contours[c][p].y, contours[c][p].x)=0;              
            } 
          }
    }
    return image;
    
}


//-------------------------------------------------------------
void LocalMinima::writeMinimaToFile(String outFileName){
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
