#include <iostream>
#include <queue>
#include <limits>
#include <values.h>
#include <stdlib.h>
#include <vector>
#include <cstddef>
#include "Graf.h"

using namespace std; 

class DistFinsV{
public:
  int vert;
  double dist;
  DistFinsV(int v, double d){
    vert=v;
    dist=d;
  }
  //PriorityQueue major numero major prioritat, per això cal enganyar-lo
 bool operator < (const DistFinsV & altre)const { return dist > altre.dist;}
};

  Graf::Graf(int nVert){
    INFINIT =DBL_MAX;
    numVert=nVert;
    adjacents=new Node*[nVert];
    for (int u = 0; u < nVert; u++) {
      adjacents[u]=0;
    }	  
  
    visitats=0;
    camiMin=0;
    distMin=0;
  }

Graf::~Graf(){
  if (visitats!=0) delete [] visitats;
  if (camiMin!=0) delete [] camiMin;
  if (distMin!=0) delete [] distMin;

  Node * aux;
  for (int u = 0; u < numVert; u++) {
    while (adjacents[u]!=NULL){
      aux=adjacents[u];
      adjacents[u]=adjacents[u]->next;
      delete aux;
    }
  }
  delete [] adjacents;
}

bool Graf::addEdge(int orig, int dest, double pes){
  if (orig < 0 || orig >= numVert || 
      dest < 0 || dest >= numVert){
    //std::cerr << "vertex outside the graf "<< std::endl;
    return false;
  }  
  adjacents[orig]=new Node(dest,pes, adjacents[orig]);;
  return true;
}

vector<int> Graf::adjacentsDe(int u){
  vector<int> llista;

  Node * aux=adjacents[u];
  while (aux!=NULL){
      llista.push_back((aux->dest));
      aux=aux->next;
  }
  return llista;
}

double Graf::pes(int u,int v){
  Node * aux=adjacents[u];
  while (aux!=NULL && aux->dest != v){
    aux=aux->next;
  }
  if (aux==NULL) return INFINIT;
  else return aux->pes;
}
int * Graf::getPath(){
  return camiMin;
}
double * Graf::getDist(){
  return distMin;
}
void Graf::dijkstra(int origen){
  if (origen < 0 || origen >= numVert){
    //cerr << "Error: node to start dijkstra must be between 0 and " << numVert-1 << " and is " << origen << endl;
    return;
  }
  camiMin= new int[numVert];
  visitats = new bool[numVert];
  distMin = new double[numVert];

  for (int i = 0; i < numVert; i++) {
    camiMin[i]=-1;
    visitats[i]=false;
    distMin[i]=INFINIT;
  }

  distMin[origen]=0;
  priority_queue <DistFinsV>cuaPrior;      //pq is a priority queue of integers

  cuaPrior.push(DistFinsV(origen,0));

  while (cuaPrior.size()>0){
    DistFinsV ob= cuaPrior.top(); 
    cuaPrior.pop();
    int v = ob.vert;
    if (!visitats[v]){
      visitats[v]=true;
      vector<int> adj = adjacentsDe(v);
      for(uint a=0; a<adj.size(); a++){
	int w = adj[a];
	if (distMin[w] > distMin[v]+pes(v,w)){
	  distMin[w] = distMin[v]+pes(v,w);
	  camiMin[w]=v;
	  cuaPrior.push(DistFinsV(w,distMin[w]));     
	}
      }
    }
  }
}


