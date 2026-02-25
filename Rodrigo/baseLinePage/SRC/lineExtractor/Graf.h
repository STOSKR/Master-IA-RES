#include <vector>

class Graf{
private:
  class Node{
  public:
    int dest;
    Node * next;
    double pes;
    Node(int d, int p, Node * n){
      dest=d;
      pes=p;
      next=n;
    }
  };

  Node ** adjacents;
  int numVert;
  bool * visitats;
  int * camiMin;
  double * distMin;
  double INFINIT;
  std::vector<int> adjacentsDe(int v);
  double pes(int u,int v);
public:
  Graf(int nVert);
  ~Graf();
  bool addEdge(int orig, int dest, double pes);
  int * getPath();
  double * getDist();
  void dijkstra(int origen);
};
