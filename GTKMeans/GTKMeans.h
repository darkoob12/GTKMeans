#ifndef GTKMEANS_H
#define GTKMEANS_H
#include "boost/tokenizer.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/variate_generator.hpp"
#include "libgambit\libgambit.h"		//what happen???????
#include "nfgsimpdiv.h"
#include <string>
#include <ctime>
#include <cmath>
#include <list>
#include <map>
#include <sstream>

#define INF 1.0e14


typedef boost::tokenizer<boost::char_separator<char>> tokenize;
typedef boost::char_separator<char> separator;
typedef std::list<int> intList;
typedef std::map<int, int*> palyerMap;
typedef std::map<int, double**> payoffMap;
typedef boost::mt19937	base_gen_type;		//method of generating pseudo-random numbers


class GTKMeans {
private:
	int* mCardOfClusters;		//cardinality of each cluster; just in case;
	int mNumData;	//number of examples
	int mDimension;		//dimension of data examples
	int mK;		//number of clusters
	int mLideal;	//ideal number of examples in each cluster
	int **mWp;		//temp reallocation of examples to the clusters after the games N*K
	double tol;		//tolerance is used in stopping condition
	void initialize();
	int** copyMatrix(int** Mat, int rows, int cols);	//make a copy of Matrix and returns a pointer to it

	//these list will be used through the run by all games!
	intList lPlayers;			//storing id of players
	intList lResources;			//storing id of resources

public:
	double mSSE, mSSEold;		//the old versions are kept to compute the improvements..
	double mL, mLold;			//..and checking stop condition
	double **mData;		// a multi dimensional array for saving data
	double **mCenters;	//cluster centers
	int **mW;		//matrix indicating membership of examples in the clusters  N*K
	void run();
	double distance(double* X,double* Y);	//euclidean distance between two points
	void setCluster(int exmp, int cluster,bool temp = false);	//associates an example with a cluster and ensures that 
																//example dose not belong to another cluster
	int whichCluster(int examp);		//determines which cluster an example should belong to
	void computeCardinals();		//fill the pre_specified array
	std::string tryAllocate();			//this function tries to allocate examples to players from 
	void readData(std::string Path);	
	int* sortByDistance(std::map<int, double> mapDist);
	void transfer(int fromID, int toID, int count, bool temp = false);	//transfer an example from one cluster to another one
	bool isItBetter();			//check whether the overall objective is improved or not
	void updateCenters();		//update cluster centers by averaging between their members
	std::stringstream formGame(std::string strGameInfo);		//create a .nfg file
	double computeSSE();		//computes sum of squared error from center for each example
	double computeL();			//Compute the l metric of the clustering

};									


struct Pair {
	int player;		//the players name
	int numEx;		//number of examples she needs from this resource
};

#endif