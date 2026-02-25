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
#include <vector>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include "pugixml.hpp"
#include "libPoints.h"
#include "libXmlPAGE.h"
using namespace std;

LineStruct::LineStruct(string n, vector<cv::Point > points){
	name = n;
	linePoints = points;
}
/*LineStruct::LineStruct(const LineStruct & L){
  name = L.name;
  linePoints = L.linePoints;
  }*/

int getWidth(pugi::xml_document & page){
  return atoi(page.child("PcGts").child("Page").attribute("imageWidth").value());
}

void getLineIds(pugi::xml_node & text_region,  vector< string > & linesId){
  for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
    linesId.push_back(line_region.attribute("TextLine id").value());
  }
}
void getBaselinesFromRegion_id(pugi::xml_node & text_region, vector< LineStruct > & baselines){
  vector<cv::Point> tmp_baseline;
  
  for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
      tmp_baseline.clear();
      std::vector<std::string> string_values;
      string line_id = line_region.attribute("id").value();
      string point_string = line_region.child("Baseline").attribute("points").value();
     
      if(point_string != ""){

        istringstream point_stream(point_string);
        std::string string_point;
        while (point_stream >> string_point){
          int cont_comas=0;
          for (int i = 0; i < string_point.size(); i++) {
            if (string_point[i] == ','){
              string_point[i] = ' ';
              cont_comas++;
            }
          }

          if (cont_comas != 1){
            cerr << "drawBaselines ERROR: regions cords bad format"<< endl;
            exit(-1);
          }     

          int x,y;
          istringstream(string_point) >> x >> y;;
          tmp_baseline.push_back(cv::Point(x,y));
        }
        baselines.push_back( LineStruct(line_id,tmp_baseline)); 
      }      
    }
}

void getBaselinesFromRegion(pugi::xml_node & text_region, vector< vector <cv::Point > > & baselines){
  vector<cv::Point> tmp_baseline;
  
  for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
      tmp_baseline.clear();
      std::vector<std::string> string_values;
      string point_string = line_region.child("Baseline").attribute("points").value();
     
      if(point_string != ""){

        istringstream point_stream(point_string);
        std::string string_point;
        while (point_stream >> string_point){
          int cont_comas=0;
          for (int i = 0; i < string_point.size(); i++) {
            if (string_point[i] == ','){
              string_point[i] = ' ';
              cont_comas++;
            }
          }

          if (cont_comas != 1){
            cerr << "drawBaselines ERROR: regions cords bad format"<< endl;
            exit(-1);
          }     

          int x,y;
          istringstream(string_point) >> x >> y;;
          tmp_baseline.push_back(cv::Point(x,y));
        }
        baselines.push_back(tmp_baseline);        
      }      
    }

}

//----------------------------------------------------------
vector <LineStruct> getBaselines_id(pugi::xml_document & page){

  vector< LineStruct > baselines;

  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    getBaselinesFromRegion_id(text_region,baselines);
    

  }
  return baselines;
} 
//----------------------------------------------------------
vector <vector <cv::Point > > getBaselines(pugi::xml_document & page){

  vector< vector <cv::Point > > baselines;

  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    getBaselinesFromRegion(text_region,baselines);
    

  }
  return baselines;
} 
//----------------------------------------------------------
void getRegionCoords(pugi::xml_node & text_region, vector<cv::Point> & region){
  
  string point_string = text_region.child("Coords").attribute("points").value();
  
  if(point_string != ""){

    istringstream point_stream(point_string);
    string string_point;
      while (point_stream >> string_point){
	int cont_comas=0;
	for (uint i = 0; i < string_point.size(); i++) {
	  if (string_point[i] == ','){
	    string_point[i] = ' ';
	    cont_comas++;
	  }
	}

	if (cont_comas != 1){
	  cerr << "drawBaselines ERROR: regions cords bad format"<< endl;
	  exit(-1);
	}

        int x,y;
        istringstream(string_point) >> x >> y;;
	region.push_back(cv::Point(x,y));
	
      }    
    }
}
//----------------------------------------------------------
vector <vector <cv::Point > > getRegions(pugi::xml_document & page){
  vector< vector <cv::Point > > regions;
  vector<cv::Point> tmp_region;

  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    tmp_region.clear();
    getRegionCoords(text_region, tmp_region );
    regions.push_back(tmp_region);
  }
  return regions;
}

//--------------------------------------------------------------------
void updateXml(pugi::xml_document & page,  std::vector< std::vector< std::vector<cv::Point> > > & lines_finals, int numPointsPerLine){

  if (lines_finals.size() <= 0 ) return;

  if (numPointsPerLine != -1)
    for(uint r = 0; r <lines_finals.size(); r++)
      for (uint l = 0; l < lines_finals[r].size(); l++)
	if (lines_finals[r][l].size() > numPointsPerLine)
	  normaliza_traza( lines_finals[r][l],numPointsPerLine );

  
  // if (page.child("PcGts").child("Page").child("TextRegion") == 0){ // no regions given
  //   pugi::xml_node reg = page.child("PcGts").child("Page").append_child("TextRegion");
  //   pugi::xml_attribute id_attr = reg.append_attribute("id");
  //   id_attr.set_value("1");
  //   pugi::xml_node reg_coords=reg.append_child("Coords");
  //   pugi::xml_attribute reg_points_attr = reg_coords.append_attribute("points");
  //   reg_points_attr.set_value("xx");
  // } else {
    for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
      for (pugi::xml_node text_line = text_region.child("TextLine"); text_line;) {
	pugi::xml_node next = text_line.next_sibling("TextLine");      //raro, pero no tocar
	text_line.parent().remove_child(text_line);
	text_line = next; //això mateix
      }
    }
    //}
  
  int n_reg=0;
  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    
    for (uint l = 0; l < lines_finals[n_reg].size(); l++) {
      pugi::xml_node line = text_region.append_child("TextLine");
      
      pugi::xml_attribute id_attr = line.append_attribute("id");
      stringstream ss;
      ss << "r" << n_reg<<"_l"<< setw(2) << setfill('0') << l;
      id_attr.set_value(ss.str().c_str());
      
      pugi::xml_node line_coords=line.append_child("Coords");
      pugi::xml_attribute line_points_attr = line_coords.append_attribute("points");
      line_points_attr.set_value("");

      //BASE LINE
      pugi::xml_node baseline = line.append_child("Baseline");
      stringstream points_stream;
      
      for (uint p = 0; p < lines_finals[n_reg][l].size()-1 ; p++) {
    	points_stream << lines_finals[n_reg][l][p].x << "," << lines_finals[n_reg][l][p].y<<" ";
      }

      points_stream << lines_finals[n_reg][l][lines_finals[n_reg][l].size()-1].x << "," << lines_finals[n_reg][l][lines_finals[n_reg][l].size()-1].y; //per a que no afegisca " " a la fi

      pugi::xml_attribute points_attr = baseline.append_attribute("points");
      points_attr.set_value(points_stream.str().c_str());         
    }
    n_reg++;
  }
}

