 /*   Copyright 2017, Moisés Pastor i Gadea
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

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>

#include <opencv2/ml.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define MODE_TRAIN 0
#define MODE_CLASSIF 1

using namespace cv;
using namespace cv::ml;
using namespace std;

bool verbosity=false;

void usage(char * nomProg){
    cerr << "Usage: " << nomProg << " [options]" << endl;
    cerr << "    options:" << endl;
    cerr << "       -i input filename (REQUIRED, data for training or classifying)" << endl;
    cerr << "       -l imput file list (REQUIRED, data for training or classifying)" << endl;
    cerr << "       -m mode (\"train\" or \"classify\", default \"classify\")" << endl;
    cerr << "       -t tree filename (REQUIRED, in training mode, the file to save the tree, in classification, the tree to use)" << endl;
    cerr << "       -n number of samples required to split a node (used in training, default 2)" << endl;
    cerr << "       -k number of random splits at each node (used in training, default sqrt(number_of_samples))" << endl;
    cerr << "       -s number of trees (used in training, default 100)" << endl;
    cerr << "       -v verbosity (by defaul none)" << endl;
}

void displayTrainingParameters(string &inFile, string &treeFile, int max_depth, int nMin, int K, int M){
    cout << "Training RT with data from " << inFile << ", with parameters:" << endl;
    cout << "   nMin = " << nMin << endl;
    cout << "   K = " << K << endl;
    cout << "   M = " << M << endl;
    cout << "Output trained RT to " << treeFile << endl;
}

void trainRT(string &inFile, string &treeFile, int nMin, int K, int M){
  // Read the training data
  //MLData dataManager;    
  //int ret_val = dataManager.read_csv(inFile.c_str());

  // Ptr<TrainData> dataManager =TrainData::loadFromCSV(inFile.c_str(),0,-1,-1,"ord[1-]cat[0]");
  
  Ptr<TrainData> dataManager =TrainData::loadFromCSV(inFile.c_str(),0,0);
 
  if(dataManager == NULL){
    cerr << "Error reading the input data " << endl;
    return;
  }
   Mat* data = new Mat(dataManager->getSamples());
   
   
  //  dataManager->change_var_type(0, VAR_CATEGORICAL); // The first column has the labels
  //  dataManager->set_response_idx(0); ATENCIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

     /* max_depth shouldn't be used, nMin limits the tree depth
       
	calc_var_importance should be set to false, but setting it to
       true could be interesting in the future to see if there are
       areas that are systematically non informative. 

       If we set term_criteria to CV_TERMCRIT_ITER, M is used to
       determine when to stop (max numer of trees), if we set it to
       CV_TERMCRIT_EPS, forest_accuracy is used. We can use both (one
       | theother) too.
       */
    
    int max_depth = 1000;
    int max_categories = 2;
    bool calc_var_importance = true;    
    float forest_accuracy = 1e-6; // Not used
    int term_criteria = TermCriteria::MAX_ITER + TermCriteria::EPS;
    
    //    ml::RTParams* parameters = new CvRTParams(max_depth, nMin, 0.0, false, max_categories,
    //					    0, calc_var_importance, 
    //                                        K, M, forest_accuracy, term_criteria);
    
    Ptr<RTrees> rt = RTrees::create();
    //NOU
    rt->setMaxDepth(max_depth);
    rt->setMinSampleCount(nMin);
    rt->setMaxCategories(max_categories);
    rt->setTermCriteria(TermCriteria(term_criteria, 100, forest_accuracy));
    //falta calc_var_importance i term_criteria
      ///

    displayTrainingParameters(inFile, treeFile, max_depth, nMin, K, M);

    //rt->train(&dataManager,*parameters);
    rt->train(dataManager);
    rt->save(treeFile.c_str());
    
    if (calc_var_importance && verbosity){
      Mat vars = rt->getVarImportance();
      for (int i = 0; i < vars.rows; i++) {
	cout << vars.row(i) << endl;
      }
    }
    
    //delete parameters;
    //delete rt;
    //delete dataManager;
}


void classifyRT( vector<string> & files, string &treeFile, float minProb){
  Ptr<RTrees> rt = NULL;
  try{
    rt = RTrees::load(treeFile.c_str());
  } catch (cv::Exception e){
    cerr << "ERROR reading " << treeFile.c_str() << endl;
    exit(-1);
  }

  for(int i=0;i<files.size();i++){
    //CvMLData dataManager;
    
    Ptr<TrainData> dataManager =TrainData::loadFromCSV(files[i].c_str(),0,0);
	    
    if(dataManager == NULL){
      cerr << "Error reading the input data for file " << files[i] << endl;
    }else{
      //Mat* data = new Mat(dataManager->getValues());
      Mat* data = new Mat(dataManager->getSamples());

      
      string outname = files[i];
      outname.append(".clas");
      ofstream out;
      out.open(outname.c_str());

      for(int j=0;j<data->rows;j++){  //CvForestTree* get_tree(int i) const;
	float clas = rt->predict(data->row(j));
	/*
	float cont=0;
	int ntrees = rt->get_tree_count();
	for (int t=0; t < ntrees; t++){
	  ForestTree* tree = rt->get_tree(t);
	  CvDTreeNode* node= tree->predict(data->row(j));
	  if (node->value == clas)
	    cont ++;
    
	}
	*/
	// if (clas == 10 && cont/(float)ntrees <=minProb)          
	//   clas = 2;

	out << clas << endl; //"  "<<cont/ntrees<< endl;
	
      }
      out.close();
      delete data;
      //delete rt;
      // delete dataManager;
    }    
  }
}


int main(int argc, char ** argv){
    int option;
    string inFile="";
    string listInFile="";
    string treeFile="";
    int mode=2; // Default
    
    // RT parameters for training
    int nMin = 2;
    int K = 0; // Default value of sqrt N
    int M = 100;
    float minProb=0.5;

    string m; // Auxiliar
    if(argc<3){
        usage(argv[0]);
        return -1;
    }
    while ((option=getopt(argc,argv,"hi:l:m:t:n:k:s:p:v"))!=-1)
        switch (option)  {
            case 'i': // Input data for training, testing or classification
                inFile=optarg;
                break;
	     case 'l':
                listInFile=optarg;
                break;
	     case 'm': // Select mode
                m=optarg;
                if(m=="train")
                    mode = MODE_TRAIN;
                else if(m=="classify")
                    mode = MODE_CLASSIF;
                else{
                    cerr << "Wrong mode. Possible choices are \"train\" or \"classify\"" << endl;
                    return -1;
                }            
                break;
            case 't': // tree file name (for saving in training, or the file that contains the trained tree for test/classification
                treeFile=optarg;
                break;
            case 'n': // Number of samples required to split a node
                nMin = atoi(optarg);
                break;
            case 'k': // Number of random splits at each node
                K = atoi(optarg);
                break;
            case 's': // Number of trees (size of the forest)
                M = atoi(optarg);
                break;
	    case 'p':
                minProb=atof(optarg);
                break;
	    case 'v':
	        verbosity=true;
		break;
            case 'h':
            default:
                usage(argv[0]);
                return 1;
        }
    if(inFile=="" && listInFile==""){
        cerr << "ERROR: no input data filename" << endl;
        return -1;
    }
    if(treeFile==""){
        cerr << "ERROR: no tree filename" << endl;
        return -1;
    }


    ifstream inFileCheck(inFile);
    ifstream listInFileCheck(listInFile);
    if(!inFileCheck.is_open() &&  !listInFileCheck.is_open()){
      cerr << "ERROR: File \""<<inFile << "\" nor \"" <<  listInFile << " not found" << endl;

      return -1;
    }
    inFileCheck.close();
    
    switch(mode){
        case MODE_TRAIN:
            trainRT(inFile, treeFile, nMin, K, M);
            break;
        case MODE_CLASSIF:{      	   
	    vector<string> files;
	  
	    if(inFile.size()!=0)
	       files.push_back(inFile);
	    else{
	      ifstream in;
	      in.open(listInFile.c_str());
	      while(in.good()){
	         string tmp;
	         in >> tmp;
	         if(tmp!="")
                   files.push_back(tmp);
	      }
	    }
	    classifyRT(files, treeFile,minProb);
	    break;
	}
       default:

	 cerr << argv[0]<< " --> ERROR: mode must be specified"<< endl;
    } 
    return 0;

      
}
