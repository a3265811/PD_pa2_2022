#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H
#include "module.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <math.h>

using namespace std;

class Floorplanner
{
public:
	Floorplanner(double alpha, fstream& input_blk, fstream& input_net){
		parser(input_blk, input_net);
		_root = NULL;
		_alpha = alpha;
		_R_star = _outlineY / _outlineX;
	}
	~Floorplanner() { }
	void parser(fstream& input_blk, fstream& input_net);
	void floorplan();
	pair<double,double> calCost();
	void deepClone(Node* old_root, Node*& new_root, Node* new_parent, int mode);
	void copyBestBlock();
	void writeBackBlock();
	void clearCoor();
	void setStart(clock_t time) { _start = time; }
	void setEnd(clock_t time) { _end = time; }

	// B*-tree function
	Block* rotateBlock();
	void moveBlock();
	void swapBlock();
	void insertNode(Node* mvNode, int mode);
	void deleteNode(Node* mvNode);
	void printNode(Node* mvNode);
	void printTree(Node* root);
	bool checkNode(Node* node1, Node* node2);
	void best_init();
	void findOuter();

	// Contour line function
	void insertContour(Block* mvBlock);
	void printContour();
	void buildContour(Node* node);
	int findHeight(int x1, int x2);

	// graph
	void printResult(fstream& out);
	void plot();

private:
	double _alpha;
	int _outlineX;
	int _outlineY;
	int _termNum;
	int _blkNum;
	int _netNum;
	double _R_star;
	double _R;
	map<string, Terminal*> _termMap;
	map<string, Block*> _blkMap;
	vector<Net*> _netArray;
	vector<Block*> _bestBlk;
	Node* _root;
	Node* _last_root;
	Node* _best_root;
	vector<Node*> _nodeArray; // all nodes in B*-tree
	vector<Node*> _leafArray; // leaf nodes in B*-tree
	list<pair<int,int>> _contour;
	pair<double,double> _bestPair;
	double _Anorm;
	double _Wnorm;
	clock_t _start;
	clock_t _end;

};

#endif
