#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H
#include "module.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

using namespace std;

class Floorplanner
{
public:
	Floorplanner(fstream& input_blk, fstream& input_net){
		parser(input_blk, input_net);
	}
	void parser(fstream& input_blk, fstream& input_net);
	void floorplan();

private:
	int _outlineX;
	int _outlineY;
	int _termNum;
	int _blkNum;
	int _netNum;
	map<string, Terminal*> _termArray;
	map<string, Block*> _blkArray;
	vector<Net*> _netArray;
	Node* _root;
	Segment* _contour;
};

#endif
