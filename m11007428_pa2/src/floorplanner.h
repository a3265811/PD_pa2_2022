#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H
#include "module.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <list>

using namespace std;

class Floorplanner
{
public:
	Floorplanner(fstream& input_blk, fstream& input_net){
		parser(input_blk, input_net);
		_root = NULL;
	}
	~Floorplanner() { }
	void parser(fstream& input_blk, fstream& input_net);
	void floorplan();
	pair<long,long> calCost();
	void deepClone(Node* old_root, Node*& new_root, Node* new_parent, int mode);

	// B*-tree function
	void rotateBlock();
	void moveBlock();
	void swapBlock();
	void insertNode(Node* mvNode, int mode);
	void deleteNode(Node* mvNode);
	void printTree(Node* root);

	// Contour line function
	void insertContour(Block* mvBlock);
	void printContour();
	void buildContour(Node* node);

	// graph
	void plot();

private:
	int _outlineX;
	int _outlineY;
	int _termNum;
	int _blkNum;
	int _netNum;
	map<string, Terminal*> _termMap;
	map<string, Block*> _blkMap;
	vector<Net*> _netArray;
	Node* _root;
	Node* _last_root;
	Node* _best_root;
	vector<Node*> _nodeArray; // all nodes in B*-tree
	vector<Node*> _leafArray; // leaf nodes in B*-tree
	list<pair<int,int>> _contour;
	list<pair<int,int>> _bast_contour;

};

#endif
