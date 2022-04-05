#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H
#include "module.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>

using namespace std;

class Floorplanner
{
public:
	Floorplanner(fstream& input_blk, fstream& input_net){
		parser(input_blk, input_net);
		_root = NULL;
		_contour = NULL;
	}
	~Floorplanner() { }
	void parser(fstream& input_blk, fstream& input_net);
	void floorplan();

	// B*-tree function
	void rotateBlock();
	void moveBlock();
	void swapBlock();
	void insertNode(Node* mvNode, int mode);
	void deleteNode(Node* mvNode);
	void printTree(Node* root);


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
	vector<Node*> _nodeArray; // all nodes in B*-tree
	set<Node*> _leafSet; // leaf nodes in B*-tree
	Segment* _contour;

};

#endif
