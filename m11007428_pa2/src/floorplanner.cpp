#include "floorplanner.h"

using namespace std;

size_t Block::_maxX = 0;
size_t Block::_maxY =  0;

void Floorplanner::parser(fstream& input_blk, fstream& input_net){
	string str,token;
	// blocks file read
	while(getline(input_blk,str)){
		stringstream ss;
		ss << str;
		if(str.find("Outline") != -1){
			ss >> token >> _outlineX >> _outlineY;
		}
		else if(str.find("NumBlocks") != -1){
			ss >> token >>_blkNum;
		}
		else if(str.find("NumTerminals") != -1){
			ss >> token >> _termNum;
		}
		else if(str.find("terminal") != -1){
			string termName;
			int termX;
			int termY;
			ss >> termName >> token >> termX >> termY;
			_termMap.insert(pair<string, Terminal*>(termName, new Terminal(termName, termX, termY)));
		}
		else if(str.length() > 1){
			string blkName;
			int blkWidth;
			int blkHeight;
			ss >> blkName >> blkWidth >> blkHeight;
			_blkMap.insert(pair<string, Block*>(blkName, new Block(blkName, blkWidth, blkHeight)));
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
				if(_blkMap.count(str) != 0)
					tempNet->addTerm(_blkMap[str]);
				if(_termMap.count(str) != 0)
					tempNet->addTerm(_termMap[str]);
			}
			_netArray.push_back(tempNet);
		}
	}
}

void Floorplanner::floorplan(){
	// transform map to array
	vector<Block*> ori_blkArray;
	vector<Block*> blkArray;
	for(map<string,Block*>::iterator map_it = _blkMap.begin(); map_it != _blkMap.end(); map_it++){
		ori_blkArray.push_back(map_it->second);
	}
	for(int r = 0; r < 1; r++){
		blkArray = ori_blkArray;
		srand(time(NULL));
		for(int i = 0; i < blkArray.size(); i++){
			int rnum = rand() % blkArray.size();
			Node* tempNode = new Node(blkArray[rnum],i);
			insertNode(tempNode, 0);
			blkArray.erase(blkArray.begin()+rnum);
			i--;
		}
		pair<long,long> tempCost;
		tempCost = calCost();
	}
	printTree(_root);
	cout << "----------------------------------------" << endl;
	deepClone(_root, _last_root, NULL, -1);
	printTree(_last_root);
//	cout << "_last_root: " << _last_root->getBlock()->getName() << " (" << _last_root->getBlock()->getX1() << "," << _last_root->getBlock()->getY1() << "," << _last_root->getBlock()->getX2() << "," << _last_root->getBlock()->getY2() << ") <" << _last_root->getBlock()->getWidth() << "," << _last_root->getBlock()->getHeight() << ">" << endl;
	buildContour(_root);
	printContour();
	//cout << "root: " << _root->getBlock()->getName() << " (" << _root->getBlock()->getX1() << "," << _root->getBlock()->getY1() << "," << _root->getBlock()->getX2() << "," << _root->getBlock()->getY2() << ") <" << _root->getBlock()->getWidth() << "," << _root->getBlock()->getHeight() << ">" << endl;
	/*string b1 = "bk2";
	string b2 = "bk14b";
	cout << "bk2: " << _blkMap[b1]->getName() << " (" << _blkMap[b1]->getX1() << "," << _blkMap[b1]->getY1() << "," << _blkMap[b1]->getX2() << "," << _blkMap[b1]->getY2() << ")" << endl;
	cout << "bk14b: " << _blkMap[b2]->getName() << " (" << _blkMap[b2]->getX1() << "," << _blkMap[b2]->getY1() << "," << _blkMap[b2]->getX2() << "," << _blkMap[b2]->getY2() << ")" << endl;*/
	plot();
}

void Floorplanner::insertNode(Node* mvNode, int mode){
	// root insert
	if(_root == NULL){
		_root = mvNode;
		_nodeArray.push_back(mvNode);
		_leafArray.push_back(mvNode);
		return;
	}

	// leaf insert
	if(mode == 0){ // leaf insert
		Node* leafNode = _leafArray[0];
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
				_leafArray.erase(_leafArray.begin());

			// insert new node to node array and leaf node
			mvNode->setPos(_nodeArray.size());
			_nodeArray.push_back(mvNode);
			bool isInArray = false;
			for(int i = 0; i < _leafArray.size(); i++){
				if(_leafArray[i] == mvNode)
					isInArray = true;
			}
			if(!isInArray)
				_leafArray.push_back(mvNode);
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
		bool isInArray = false;
		for(int i = 0; i < _leafArray.size(); i++){
			if(_leafArray[i] == mvNode)
				isInArray = true;
		}
		if(!isInArray)
			_leafArray.push_back(mvNode);
	}
	else
		cout << "insert mode error: " << mode << endl;
}

void Floorplanner::deleteNode(Node* mvNode){
	// mvNode swap with child until becoming leaf node
	while(mvNode->getLeft() != NULL || mvNode->getRight() != NULL ){
		Node* parNode = mvNode->getParent();
		Node* leftNode = mvNode->getLeft();
		Node* rightNode = mvNode->getRight();
		Node* swapNode = NULL;
		// 0: swapNode is mvNode left
		// 1: swapNode is mvNode right
		// 2: mvNode is leaf node
		int mode = -1; 
		if(leftNode != NULL && rightNode != NULL){
			srand(time(NULL));
			if((rand() % 2) == 0){
				swapNode = leftNode;
				mode = 0;
			}
			else{
				swapNode = rightNode;
				mode = 1;
			}
		}
		else if(leftNode != NULL){
			mode = 0;
			swapNode = leftNode;
		}
		else if(rightNode != NULL){
			mode = 1;
			swapNode = rightNode;
		}
		else{
			cout << "delete function mode error" << endl;
			exit(1);
		}
		// swap node
		if(mvNode == _root)
			_root = swapNode;
		mvNode->setParent(swapNode);
		swapNode->setParent(parNode);
		if(parNode != NULL){
			if(parNode->getLeft() == mvNode)
				parNode->setLeft(swapNode);
			else if(parNode->getRight() == mvNode)
				parNode->setRight(swapNode);
			else
				cout << "delete function missing mvNode connection with parent" << endl;
		}
		mvNode->setLeft(swapNode->getLeft());
		if(swapNode->getLeft() != NULL)
			swapNode->getLeft()->setParent(mvNode);
		mvNode->setRight(swapNode->getRight());
		if(swapNode->getRight() != NULL)
			swapNode->getRight()->setParent(mvNode);
		if(mode == 0){
			swapNode->setLeft(mvNode);
			swapNode->setRight(rightNode);
			if(rightNode != NULL)
				rightNode->setParent(swapNode);
		}
		else if(mode == 1){
			swapNode->setLeft(leftNode);
			if(leftNode != NULL)
				leftNode->setParent(swapNode);
			swapNode->setRight(mvNode);
		}
	}
	// mvNode deletion
	if(mvNode->getParent() != NULL){
		if(mvNode->getParent()->getLeft() == mvNode)
			mvNode->getParent()->setLeft(NULL);
		else if(mvNode->getParent()->getRight() == mvNode)
			mvNode->getParent()->setRight(NULL);
	}
	else{ // root deletion
		_root = NULL;
	}
}

void Floorplanner::printTree(Node* root){
	if(!root) return;
	cout << "root name: " << root->getBlock()->getName();
	if(root->getParent())
		cout << " pre: " << root->getParent()->getBlock()->getName();
	if(root->getLeft())
		cout << " left: " << root->getLeft()->getBlock()->getName();
	if(root->getRight())
		cout << " right: " << root->getRight()->getBlock()->getName();
	cout << " root address: " << root;
	cout << endl;
	printTree(root->getLeft());
	printTree(root->getRight());
}

void Floorplanner::buildContour(Node* root){
	if(!root) return;
	Block* mvBlock = root->getBlock();
	Node* parNode = root->getParent();
	if(parNode != NULL){
		Block* parBlock = parNode->getBlock();
		if(root == parNode->getLeft()){
			mvBlock->setPos(parBlock->getX2(), 0, parBlock->getX2() + mvBlock->getWidth(), 0);
		}
		else if(root == parNode->getRight()){
			mvBlock->setPos(parBlock->getX1(), 0, parBlock->getX1() + mvBlock->getWidth(), 0);
		}
	}
	else{
		mvBlock->setX2(mvBlock->getX1() + mvBlock->getWidth());
	}
	insertContour(mvBlock);
	buildContour(root->getLeft());
	buildContour(root->getRight());
}

void Floorplanner::insertContour(Block* mvBlock){
	list<pair<int,int>>::iterator start_it, end_it;
	start_it = _contour.end();
	end_it = _contour.end();
	int x1 = mvBlock->getX1();
	int x2 = mvBlock->getX2();
	int nowX = 0;
	int startX = 0, endX = 0;
	int localMaxH = 0;
	int counter = 0;
	// cout << "x1: " << x1 << " x2: " << x2 << endl;
	bool debug = false;
	if(mvBlock->getName() == "bk2")
		debug = true;
	if(debug){
		printContour();
		cout << "mvBlock: " << mvBlock->getName() << endl;
		string b1 = "bk9d";
		//string b2 = "bk14b";
		cout << "bk9d: " << _blkMap[b1]->getName() << " (" << _blkMap[b1]->getX1() << "," << _blkMap[b1]->getY1() << "," << _blkMap[b1]->getX2() << "," << _blkMap[b1]->getY2() << ")" << endl;
		plot();
		//cout << "bk14b: " << _blkMap[b2]->getName() << " (" << _blkMap[b2]->getX1() << "," << _blkMap[b2]->getY1() << "," << _blkMap[b2]->getX2() << "," << _blkMap[b2]->getY2() << ")" << endl;
	}
	// find where is the mvBlock
	for(list<pair<int,int>>::iterator it = _contour.begin(); it != _contour.end(); it++){
		int segLength = it->first;
		if(nowX == x1){
			start_it = it;
			startX = nowX;
			if(localMaxH < it->second)
				localMaxH = it->second;
			counter++;
		}
		if(nowX < x2 && x2 <= nowX + segLength){
			end_it = it;
			endX = nowX;
			if(counter == 1 && localMaxH < it->second)
				localMaxH = it->second;
			counter++;
		}
		else{
			if(counter == 1 && localMaxH < it->second)
				localMaxH = it->second;
			if(counter == 1){
				it = _contour.erase(it);
				it--;
			}
		}
		if(counter == 2)
			break;
		nowX += segLength;
	}
	if(debug)
		cout << "start_it->second: " << start_it->second << endl;
	// update
	if(start_it == _contour.end()){
		mvBlock->setY1(0);
		mvBlock->setY2(mvBlock->getHeight());
		_contour.push_back(pair<int,int>(mvBlock->getWidth(), mvBlock->getHeight()));
		if(debug)
			cout << "insertContour mode: 1" << endl;
	}
	else{
		if(end_it == _contour.end()){
			mvBlock->setY1(localMaxH);
			mvBlock->setY2(localMaxH + mvBlock->getHeight());
			_contour.push_back(pair<int,int>(mvBlock->getWidth(), mvBlock->getY2()));
			if(debug)
				cout << "insertContour mode: 2" << endl;
		}
		else if(x2 == endX + end_it->first){
			if(start_it != end_it){
				mvBlock->setY1(localMaxH);
				mvBlock->setY2(localMaxH + mvBlock->getHeight());
				end_it->first = mvBlock->getWidth();
			}
			else{
				mvBlock->setY1(end_it->second);
				mvBlock->setY2(end_it->second + mvBlock->getHeight());
			}
			end_it->second = mvBlock->getY2();
			if(debug)
				cout << "insertContour mode: 3" << endl;
		}
		else{
			if(start_it != end_it){
				mvBlock->setY1(localMaxH);
				mvBlock->setY2(localMaxH + mvBlock->getHeight());
			}
			else{
				mvBlock->setY1(end_it->second);
				mvBlock->setY2(end_it->second + mvBlock->getHeight());
			}
			end_it->first = endX + end_it->first - x2;
			_contour.insert(end_it, pair<int,int>(mvBlock->getWidth(), mvBlock->getY2()));
			if(debug)
				cout << "insertContour mode: 3" << endl;
		}
	}
	if(Block::getMaxX() < mvBlock->getX2())
		Block::setMaxX(mvBlock->getX2());
	if(Block::getMaxY() < mvBlock->getY2())
		Block::setMaxY(mvBlock->getY2());
}

void Floorplanner::printContour(){
	for(list<pair<int,int>>::iterator it = _contour.begin(); it != _contour.end(); it++){
		cout << "<" << it->first << "," << it->second << "> ";
	}
	cout << endl;
}

pair<long,long> Floorplanner::calCost(){
	long area = long(Block::getMaxX()) * long(Block::getMaxY());
	/*cout << "maxX: " << Block::getMaxX() << " maxY: " << Block::getMaxY() << endl;
	cout << "area: " << area << endl;*/
	long HPWL = 0;
	for(int i = 0; i < _netArray.size(); i++){
		vector<Terminal*> termList = _netArray[i]->getTermList();
		int minX = INT_MAX;
		int maxX = 0;
		int minY = INT_MAX;
		int maxY = 0;
		for(int j = 0; j < termList.size(); j++){
			string termName = termList[j]->getName();
			if(_blkMap.count(termName) != 0){
				minX = minX <= _blkMap[termName]->getX1() ? minX : _blkMap[termName]->getX1();
				maxX = maxX >= _blkMap[termName]->getX2() ? maxX : _blkMap[termName]->getX2();
				minY = minY <= _blkMap[termName]->getY1() ? minY : _blkMap[termName]->getY1();
				maxY = maxY >= _blkMap[termName]->getY2() ? maxY : _blkMap[termName]->getY2();
			}
			else if(_termMap.count(termName) != 0){
				minX = minX <= _termMap[termName]->getX1() ? minX :_termMap[termName]->getX1();
				maxX = maxX >= _termMap[termName]->getX2() ? maxX :_termMap[termName]->getX2();
				minY = minY <= _termMap[termName]->getY1() ? minY :_termMap[termName]->getY1();
				maxY = maxY >= _termMap[termName]->getY2() ? maxY :_termMap[termName]->getY2();
			}
		}
		HPWL += maxX - minX + maxY - minY;
	}
//	cout << "HPWL: " << HPWL << endl;
	return pair<long,long>(area,HPWL);
}

void Floorplanner::deepClone(Node* old_root, Node*& new_root, Node* new_parent, int mode){
	if(!old_root) return;
	new_root = new Node(old_root->getBlock(), old_root->getId());
	new_root->setParent(new_parent);
	if(mode == 0){
		new_parent->setLeft(new_root);
	}
	else if(mode == 1){
		new_parent->setRight(new_root);
	}
	Node* new_left = new_root->getLeft();
	Node* new_right = new_root->getRight();
	if(old_root->getLeft() != NULL)
		deepClone(old_root->getLeft(), new_left, new_root, 0);
	if(old_root->getRight() != NULL)
		deepClone(old_root->getRight(),new_right, new_root, 1);
}
