#ifndef _LIB_XML_PAGE_H_
#define _LIB_XML_PAGE_H_

#include <vector>
#include <pugixml.hpp>
#include <opencv2/opencv.hpp>
using namespace std;

class LineStruct{
public:
  string name;
  vector <cv::Point > linePoints;
  LineStruct(string n, vector<cv::Point > points);
  // LineStruct(const LineStruct & L);
  LineStruct(){};
};

int getWidth(pugi::xml_document & page);
void getBaselinesFromRegion_id(pugi::xml_node & text_region, vector< LineStruct > & baselines);
void getLineIds(pugi::xml_node & text_region,  vector< string > & linesId);
vector <vector <cv::Point > > getBaselines(pugi::xml_document & page);
vector <LineStruct> getBaselines_id(pugi::xml_document & page);
void getBaselinesFromRegion(pugi::xml_node & text_region, vector< vector <cv::Point > > & baselines);
vector <vector <cv::Point > > getRegions(pugi::xml_document & page);

void getRegionCoords(pugi::xml_node & text_region, vector<cv::Point> & region);

void updateXml(pugi::xml_document & page,  vector< vector< vector<cv::Point> > > & lines_finals, int numPointsPerLine=-1);

#endif
