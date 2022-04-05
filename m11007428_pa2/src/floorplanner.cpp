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
		if(str.find("NumNets") != -1)
			ss >> token >> _netNum;
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
	Node* tempNode = new Node(0);
	Node* tempNode1 = new Node(1);
	Node* tempNode2 = new Node(2);
	Node* tempNode3 = new Node(3);
	Node* tempNode4 = new Node(4);
	Node* tempNode5 = new Node(5);
	Node* tempNode6 = new Node(6);
	insertNode(tempNode,0);
	insertNode(tempNode1,0);
	insertNode(tempNode2,0);
	insertNode(tempNode3,0);
	insertNode(tempNode4,0);
	insertNode(tempNode5,0);
	insertNode(tempNode6,0);
	printTree(_root);
	cout << "---------------------------" << endl;
	Node* tempNode7 = new Node(7);
	insertNode(tempNode7,1);
	printTree(_root);

}

void Floorplanner::insertNode(Node* mvNode, int mode){
	// root insert
	if(_root == NULL){
		_root = mvNode;
		_nodeArray.push_back(mvNode);
		_leafSet.insert(mvNode);
		return;
	}

	// leaf insert
	if(mode == 0){ // leaf insert
		Node* leafNode = *(_leafSet.begin());
		if(leafNode == NULL)
			cout << "insert error with NULL leaf pointer" << endl;
		else{
			if(leafNode->getLeft() == NULL){
				leafNode->setLeft(mvNode);
				mvNode->setParent(leafNode);
			}
			else if(leafNode->getRight() == NULL){
				leafNode->setRight(mvNode);
				mvNode->setParent(leafNode);
			}
			else{
				cout << "insert error with no place to put new node(full child)" << endl;
			}

			// check whether leaf node full
			if(leafNode->getLeft() != NULL && leafNode->getRight() != NULL)
				_leafSet.erase(leafNode);

			// insert new node to node array and leaf node
			_nodeArray.push_back(mvNode);
			_leafSet.insert(mvNode);
		}
	}
	else if(mode == 1){ // jump insert
		srand(time(NULL));
		int randPos = rand() % _nodeArray.size();
		cout << "randPos: " << randPos << endl;
		cout << "size: " << _nodeArray.size() << endl;
		Node* oriNode = _nodeArray[randPos];
		Node* parNode = oriNode->getParent();
		bool dir = false;	// false for left child, true for right child
		if(parNode != NULL){
			if(parNode->getLeft() == oriNode)
				parNode->setLeft(mvNode);
			else if(parNode->getRight() == oriNode){
				parNode->setRight(mvNode);
				dir = true;
			}
		}
		mvNode->setParent(parNode);
		oriNode->setParent(mvNode);
		// maintain original tree structure
		if(!dir)
			mvNode->setLeft(oriNode);
		else
			mvNode->setRight(oriNode);
		// insert new node to leaf node
		_leafSet.insert(mvNode);
	}
	else
		cout << "insert mode error: " << mode << endl;
}

void Floorplanner::deleteNode(Node* mvNode){
	// only root
	
	// 
}

void Floorplanner::printTree(Node* root){
	if(!root) return;
	cout << "root id: " << root->getId();
	if(root->getParent())
		cout << " pre: " << root->getParent()->getId();
	if(root->getLeft())
		cout << " left: " << root->getLeft()->getId();
	if(root->getRight())
		cout << " right: " << root->getRight()->getId();
	cout << endl;
	printTree(root->getLeft());
	printTree(root->getRight());
}
