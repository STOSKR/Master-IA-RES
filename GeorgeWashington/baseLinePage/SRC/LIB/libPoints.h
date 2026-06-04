#ifndef __LIB_POINTS_H_
#define __LIB_POINTS_H_
#include <opencv2/opencv.hpp>

int calc_y(cv::Point startPoint, cv::Point endPoint, int x);
cv::Point rotatePoint(cv::Point p,double angle, cv::Mat & img);
cv::Point rotatePoint(cv::Point p,double angle, int rows, int cols);
int getPointsFromFile(std::string pointsFileName,std::vector<cv::Point> & points);
void writePointsToFile(std::string outFileName,std::vector<cv::Point>  selected_min);
void writePointsToFile(std::string outFileName,std::vector<cv::Point3d> & selected_min);
int getPointsFromFile(std::string pointsFileName,std::vector<cv::Point3d> & points);
void normaliza_traza(std::vector<cv::Point> & baseline, int NumPuntsNuevo=10);

#endif
