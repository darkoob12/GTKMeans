#include "GTKMeans.h"


/// This is the main function of the algorithm which contains the main loop
void GTKMeans::run() {
	//this function assumes that are variables are initialized -- means the data is loaded into class
	//creating a random number generator
	base_gen_type gen(static_cast<unsigned int>(time(0)));
	boost::uniform_real<> uni_dist(0,10);
	boost::variate_generator<base_gen_type&, boost::uniform_real<>> uni(gen, uni_dist);
	//now we can use uni to generate random numbers between -1 and 1

	bool done = false;
	
	//initialization;
	//generating random points for cluster centers
	for (int i = 0;i < mK;i++)
	{
		for (int j = 0;j < mDimension;j++)
		{
			mCenters[i][j] = uni();
		}
	}
	//main loop
	while (!done) {
		//decide the association of examples to clusters according to distance from cluster center
		for (int i = 0;i < mNumData;i++)
		{
			setCluster(i,whichCluster(i));
		}
		// now we have new clusters
		// we should check for the equipartition objective
		computeCardinals();		//computes cardinality of each cluster;

		// computing the performance measures
		mSSE = computeSSE();
		mL = computeL();

		// check whether the Equipartition objective is satisfied or not?
		if (mL > 0)
		{
			mWp = copyMatrix(mW, mNumData, mK);	//this will make a copy of mW which is pointed by mWp
			
			std::string conflicts = tryAllocate();
			// now the non conflicting transfers are applied to mWp
			// we need to construct a game for each conflicting resource and solve that game
			
			//i need to create a tokenizer for parsing conflict string
			//	;resID:PlayerList:overHead;
			// ; separates conflicts
			// : separates resource from list of players
			// , separates players from each other
			separator sep(";");
			tokenize tok(conflicts, sep);
			tokenize::iterator iterTokens;
	
			//////////////////////////////////////////////////////////////////////////
			/////// SOME DAY I WILL CONSIDER ORDER OF THE GAMES.....!!!!!!.....??????!!!!!!//
			//////////////////////////////////////////////////////////////////////////
			//for each game
			for (iterTokens = tok.begin();iterTokens == tok.end();++iterTokens)
			{
				//this function call will create .nfg 
				std::stringstream strNewGame = formGame(*iterTokens);
				// now we should create and solve a game
				// and make temp allocations
				// i this part i assume that strNewGame contains a standard .nfg game inside it
				Gambit::Game myGame = Gambit::ReadGame(strNewGame);
			}
	
			//check whether these new assignments are made things better or not?
			if (isItBetter())
			{
				mW = mWp;		//this is a pointer assignment and NO PROBLEM
			} 
		}

		//now we should update cluster centers by averaging between their members
		updateCenters();

		//Are We Finished
		// in this part we may check for stopping condition

	}
}  
// sum of square errors --compaction objective
double GTKMeans::computeSSE()
{
	double ret = 0;
	for (int i = 0;i < mNumData;i++)
	{
		for (int k = 0;k < mK;k++)
		{
			if (mW[i][k] == 1)
			{
				ret += distance(mData[i],mCenters[k]);
			}
		}
	}
	return ret;
}

//this function assumes that the cardinal of each cluster is updated
//means the values in the mCardOfClusters are updated
double GTKMeans::computeL()
{
	double L = 0;
	for (int k = 0;k < mK;k++)
	{
		L += pow((double)(mLideal - mCardOfClusters[k]), 2);
	}
	return L;
}


// euclidean distance between two points or vectors
double GTKMeans::distance(double* X, double* Y) 
{
	double ret = 0;
	for (int i = 0;i < mDimension;i++)
	{
		ret += pow(X[i] - Y[i],2);
	}
	return sqrt(ret);
}

// associating an example to a cluster in a way
// that its cluster be unique
void GTKMeans::setCluster(int exmp, int cluster, bool temp) {
	if (temp)
	{
		for (int i = 0;i < mK;i++)
		{
			mWp[exmp][i] = i==cluster?1:0;
		}
	} 
	else
	{
		for (int i = 0;i < mK;i++)
		{
			mW[exmp][i] = i==cluster?1:0;
		}
	}
}

// finds nearest cluster center to the given example
int GTKMeans::whichCluster(int examp) {
	int minID = 0;			// initial cluster
	double minVal = INF;
	for (int i = 0;i < mK;i++)
	{
		double temp = distance(mData[examp], mCenters[i]);
		if (temp < minVal)
		{
			minID = i;
			minVal = temp;
		}
	}
	return minID;
}

// computes cardinal of each cluster
// for testing the equipartition objective
void GTKMeans::computeCardinals() 
{
	for (int j = 0;j <mK;j++)
	{
		int s = 0;	//for summing
		for (int i = 0;i < mNumData;i++)
		{
			s += mW[i][j];
		}
		mCardOfClusters[j] = s;
	}
}


// tries to allocate members from resources to near players
// if conflict happens returns a string containing conflict situations
std::string GTKMeans::tryAllocate() 
{
	//this will remove info from last game played
	lPlayers.clear();
	lResources.clear();
	std::string ret = "";		//we will store list of conflicting resources in this string and returns this
	palyerMap myMap;			//storing for each player a list of near resources
	for (int i = 0;i < mK;i++)
	{
		if (mCardOfClusters[i] < mLideal)
		{
			lPlayers.push_front(i);
		} 
		else if (mCardOfClusters[i] > mLideal) 
		{
			lResources.push_front(i);
		}
	}
	//determining near resources for each player
	intList::iterator iterPlayer;
	for (iterPlayer = lPlayers.begin();iterPlayer == lPlayers.end();++iterPlayer)
	{
		int pID = *iterPlayer;
		std::map<int, double> temp;		//storing each distance to sort
		intList::iterator iterRes;
		for (iterRes = lResources.begin();iterRes == lResources.end();++iterRes)
		{
			temp[*iterRes] = distance(mCenters[pID], mCenters[*iterRes]);
		}
		myMap[pID] = sortByDistance(temp);
	}
	//in this point of the function we have list of players and near resources for each of them
	// we should consider nearest resource for each player
	std::map<int, std::list<Pair>> resReq;		//stores requests for each resource;
	for (iterPlayer = lPlayers.begin();iterPlayer == lPlayers.end();++iterPlayer)
	{
		int pID = *iterPlayer;			// current players id
		int resID = myMap[pID][0];		//get the closest resource id for current Player;
		Pair foo;
		foo.player = pID;		//players id set in the pair
		foo.numEx = mLideal - mCardOfClusters[pID];		//needed examples
		resReq[resID].push_front(foo);
	}
	// resReq => contains a key{id of resource} coresponding to a pair playerID:neededExamples
	//now we determine non conflicting resources and apply the requests
	//and returning a list of conflicting ones
	intList::iterator iterRes;
	std::list<Pair>::iterator iterPairs;
	for (iterRes = lResources.begin();iterRes == lResources.end();++iterRes)
	{
		int resID = *iterRes;
		int sum = 0;		//total number of requests for examples in this cluster
		
		for (iterPairs = resReq[resID].begin();iterPairs == resReq[resID].end();++iterPairs)
		{
			sum += iterPairs->numEx;
		}
		int overHead = mLideal - (mCardOfClusters[resID] - sum);
		if (overHead < 0)
		{
			//means there is no conflict
			for (iterPairs = resReq[resID].begin();iterPairs == resReq[resID].end();++iterPairs)
			{
				transfer(resID, iterPairs->player, iterPairs->numEx, true);		//these will happen to temp matrix _ mWp
																				//true as the last argument selects mWp...
			}
		} else {
			//there is a conflict;
			ret += resID + ":";
			for (iterPairs = resReq[resID].begin();iterPairs == resReq[resID].end();++iterPairs)
			{
				ret += iterPairs->player + ",";	//when parsing i will use a method that discard empty tokens
			}
			ret += ":" + boost::lexical_cast<std::string, int>(overHead) + ";";
		}
	}
	return ret;
}

//creating a copy of matrix in memory and returning a pointer to that
int** GTKMeans::copyMatrix(int** Mat, int rows, int cols) 
{
	int** ret = new int*[rows];		//pointer with the same type and size
	for (int i = 0;i < rows;i++)
	{
		memcpy(ret[i],Mat[i],sizeof(int)*cols);
	}
	return ret;
}


//reading data should be specialized for each data set
void GTKMeans::readData(std::string Path)
{

}

// this function should receives a hash table and returns a sorted array of
// keys by their corresponding values
int* GTKMeans::sortByDistance(std::map<int, double> mapDist)
{
	int sizeRet = mapDist.size();
	int* ret = new int[sizeRet];
	std::map<int, double>::iterator iterKV;
	int i = 0;	//counting members of array
	// this loop will copy all the keys in the new array - unsorted
	for (iterKV = mapDist.begin();iterKV == mapDist.end();++iterKV)
	{
		ret[i] = iterKV->first;
	}
	//now we may sort the new array  -- Bubble Sort!!
	for (i = 0;i < sizeRet;i++)
	{
		for (int j = i+1;j < sizeRet;j++)
		{
			if (mapDist[ret[i]] > mapDist[ret[j]])
			{
				// now we should swap ret[i] and ret[j]
				int temp = ret[i];
				ret[i] = ret[j];
				ret[i] = temp;
			}	
		}
	}
	return ret;
}

void GTKMeans::transfer(int fromID, int toID, int count, bool temp)
{
	//this transfer should be considered carefully
	//examples from source that are closest to the destination should be transfered
	std::map<int, double> distMap;		// a mapping between points and their distance to the given cluster center
	
	for (int i = 0;i < mNumData;i++)
	{
		if (mWp[i][fromID] == 1)
		{	//means instance i belongs to cluster fromID
			distMap[i] = distance(mCenters[toID], mData[i]);
		}
	}

	int* sortedByDistance = sortByDistance(distMap);	//sorting by distance => this may be performed more efficeintly
	for (int j = 0;j < count;j++)
	{
		setCluster(sortedByDistance[j], toID, temp);
	}
}


//how should a reallocation considered as better
// if there is no improvement in any of the objectives then the new solution is not good
bool GTKMeans::isItBetter() 
{
	return false;
}


// averaging between cluster members to determine new centers
void GTKMeans::updateCenters()
{
	//i will use formula(2) in the paper for updating cluster centers
	//iterating through clusters
	for (int k = 0;k < mK;k++)
	{
		int numMem = 0;
		//iterating through data
		for (int i = 0;i < mNumData;i++)
		{
			numMem += mW[i][k];
		}
		
		// iterating through features
		for (int j = 0;j < mDimension;j++)
		{
			double sum = 0;
			for (int i = 0;i < mNumData;i++)
			{
				sum += mW[i][k] * mData[i][j];
			}	
			mCenters[k][j] = sum / ((double)numMem);
		}
	}
}

// will create an in_memory file of type .nfg
// input is a string in form of => resID:p1,p2,p3...
// i will assume that this function will be called after tryAlloc() in each iteration
std::stringstream GTKMeans::formGame(std::string strGameInfo)
{
	std::stringstream ret;
	//first parts of the format
	ret << "NFG 1 R \"ConflictGame\" ";
	separator sep(":");
	tokenize toks(strGameInfo, sep);
	int resID = boost::lexical_cast<int, std::string>(*toks.begin());	//now we have resouce id;
	std::string strPlayers = *(++toks.begin());		//NOT SAFE
	int overHead = boost::lexical_cast<int, std::string>(*toks.end());		//number of examples that all player must loose;
	// now players string should be tokenized
	separator sepPlayer(",");
	tokenize tokPlayers(strPlayers, sepPlayer);
	std::list<int> lstPlayers;			//list of players' id
	std::list<int>::iterator iterList;	//traversing elements of the list
	tokenize::iterator iterPlayers;
	for (iterPlayers = tokPlayers.begin();iterPlayers == tokPlayers.end();++iterPlayers)
	{
		lstPlayers.push_front(boost::lexical_cast<int, std::string>(*iterPlayers));
	}
	//initializing a matrix for saving strategy counts
	int numPlayers = lstPlayers.size();
	iterList = lstPlayers.begin();
	int **matStrategy = new int*[numPlayers];	
	for (int i = 0;i < numPlayers;i++)
	{
		matStrategy[i] = new int[2];		//player id : number of Strategies
		matStrategy[i][0] = *iterList;
		iterList++;
	}
	int numSrategies = 1;	//since i want t use multiplication rule it is set to 1
	// at this point we have the resource id and ids of competing players
	// adding players and their corresponding number of actions to the file
	
	
	std::string tmpStrPlayers = " { ";		//i used these strings to do the iteration once
	std::string tmpStrActions = " { ";
	for (int i = 0;i < numPlayers;i++)
	{
		int exsNeeded = mLideal - mCardOfClusters[matStrategy[i][0]];
		matStrategy[i][1] = (exsNeeded >= overHead)?overHead:exsNeeded;	//number of possible actions for current player
		numSrategies *= matStrategy[i][1];
		tmpStrPlayers += *iterList + " ";
		tmpStrActions += matStrategy[i][1] + " ";
	}
	tmpStrActions += " } ";
	tmpStrPlayers += " } ";
	ret << tmpStrPlayers << " " << tmpStrActions;
	// now we may generate strategy profiles in dictionary order
	// and save their corresponding payoffs
	//in this part i should implement Algorithm2 in the paper
	payoffMap mpPayoffs;
	
	//for all players
	for (int i = 0;i < numPlayers;i++)
	{
		//determine size of payoff matrix
		int rows = matStrategy[i][1];
		int cols = 1;
		for (int kk = 0;kk < numPlayers;kk++)
		{
			if (kk != i)
			{
				cols *= matStrategy[kk][1];		//combination of all possible actions for other players
			}
		}
		//create a payoff matrix of size rows*cols
		double **poi = new double*[rows];
		for (int kk = 0;kk < rows;kk++)
		{
			poi[kk] = new double[cols];
		}
		//the current state of clusters is in the temp allocation => mWp
		for (int j = 0;j < cols;j++)
		{
			for (int k = 0;k < rows;k++)
			{
				double rcBefore = 0;		//increase in SSE if player i choose his strategy before other players -i

			}
		}
	}
	
	//releasing memory
	for (int i = 0;i < numPlayers;i++)
	{
		delete matStrategy[i];
	}
	delete[] matStrategy;

	return ret;
}