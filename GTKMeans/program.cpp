#include <iostream>
#include <fstream>
#include <string>
#include "GTKMeans.h"
#include "boost/lexical_cast.hpp"
#include "nfgsimpdiv.h"

using namespace std;


int main(int argc, char* argv[]) {
	
	//////////////////////////////////////////////////////////////////////////
	//reading data
	//iris data set from UCI
	//////////////////////////////////////////////////////////////////////////
	double data[150][4];
	ifstream input("iris.data");
	string line;		//this is used for reading instances one-by-one
	int k = 0;		//counter for instances
	while (!input.eof())
	{
		input >> line;
		separator sep(",");
		tokenize toks(line,sep);
		tokenize::iterator it = toks.begin();
	
		for (int i = 0;i < 4;i++)
		{
			data[k][i] = boost::lexical_cast<double>(*it);		//converting string to double
			it++;		//next token
		} 
		k++;	//next example
	}

	//////////////////////////////////////////////////////////////////////////
	//Game Theoretic K-Means Algorithm
	//////////////////////////////////////////////////////////////////////////
	system("Pause");
	return 0;
}