#include "floorplanner.h"

using namespace std;

void Floorplanner::parser(fstream& input_blk, fstream& input_net){
	string str;
	int mode = 0;
	// blocks file read
	while(getline(input_blk, str)){
		stringstream ss;
		string token;
		ss << str;
		if(str.find("Outline") != -1){
			ss >> token >> _outlineX >> _outlineY;
		}
		else if(str.find("NumBlocks") != -1){
			ss >> token >> _blkNum;
		}
		else if(str.find("NumTerminals") != -1){
			ss >> token >> _termNum;
		}
		else if(str.length() == 1){
			mode++;
			continue;
		}

		if(mode == 1){
			string blkName;
			int blkWidth;
			int blkHeight;
			ss >> blkName >> blkWidth >> blkHeight;
			_blkArray.insert(pair<string, Block*>(blkName, new Block(blkName, blkWidth, blkHeight)));
		}
		else if(mode == 2){
			string termName;
			int termX;
			int termY;
			ss >> termName >> token >> termX >> termY;
			_termArray.insert(pair<string, Terminal*>(termName, new Terminal(termName, termX, termY)));
		}
	}

	// nets file read
	while(getline(input_net, str)){
		stringstream ss;
		string token;
		ss << str;
		if(str.find("NumNets") != -1){
			ss >> token >> _netNum;
			cout << "NumNets: " << _netNum << endl;
		}
		else if(str.find("NetDegree") != -1){
			int degree;
			ss >> token >> degree;
			Net* tempNet = new Net(degree);
			for(int i = 0; i < degree; i++){
				getline(input_net, str);
				ss << str;
				ss >> str;
				if(_blkArray.count(str) != 0)
					tempNet->addTerm(_blkArray[str]);
				if(_termArray.count(str) != 0)
					tempNet->addTerm(_termArray[str]);
			}
			_netArray.push_back(tempNet);
		}
	}
}

void Floorplanner::floorplan(){
}
