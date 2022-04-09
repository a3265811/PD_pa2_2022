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

bool compare(Block *blk1, Block *blk2){
	return blk1->getWidth() * blk1->getHeight() > blk2->getWidth() * blk2->getHeight();
}

void Floorplanner::best_init(){
	vector<Block*> blkArray;
	for(map<string,Block*>::iterator map_it = _blkMap.begin(); map_it != _blkMap.end(); map_it++){
		blkArray.push_back(map_it->second);
	}
	sort(blkArray.begin(), blkArray.end(), compare);

	int counter = 0;
	int maxX = 0;
	int maxY = 0;
	Node* leftMost;
	Node* parent;
	for(int i = 0; i < blkArray.size(); i++){
		cout << "name: " << blkArray[i]->getName() << endl;
		if(_best_root == NULL){
			blkArray[i]->setX2(blkArray[i]->getWidth());
			_best_root = new Node(blkArray[i], counter);
			leftMost = _best_root;
			parent = leftMost;
		}
		else{
			Block* parBlock = parent->getBlock();
			Node* mvNode = new Node(blkArray[i], counter);
			if(parBlock->getX2() + blkArray[i]->getWidth() <= _outlineX){
				cout << "parBlock: " << parBlock->getName() << " " << parBlock->getX2() << endl;
				blkArray[i]->setX1(parBlock->getX2());
				blkArray[i]->setX2(blkArray[i]->getX1() + blkArray[i]->getWidth());
				parent->setLeft(mvNode);
				mvNode->setParent(parent);
				parent = mvNode;
			}
			else{
				Block* lmBlock = leftMost->getBlock();
				blkArray[i]->setX1(lmBlock->getX1());
				blkArray[i]->setX2(blkArray[i]->getX1() + blkArray[i]->getWidth());
				leftMost->setRight(mvNode);
				mvNode->setParent(leftMost);
				leftMost = mvNode;
				parent = leftMost;
			}
		}
		_contour.clear();
		buildContour(_best_root);
		counter++;
	}
	//printTree(_best_root);
	plot();
}

void Floorplanner::floorplan(){
	// transform map to array
	srand(time(NULL));
	vector<Block*> ori_blkArray;
	vector<Block*> blkArray;
	double Anorm = 0, Wnorm = 0;
	double Best_cost = INT_MAX;
	int roundCounter;
	double alpha = _alpha;
	double beta = 1 - alpha;
	bool solve = false;
	for(map<string,Block*>::iterator map_it = _blkMap.begin(); map_it != _blkMap.end(); map_it++){
		ori_blkArray.push_back(map_it->second);
		cout << map_it->first << " " << map_it->second->getWidth() << " " << map_it->second->getHeight() << endl;
	}

	pair<double,double> bestPair = calCost();
	Best_cost = alpha * bestPair.first + beta * bestPair.second;
	
	for(int r = 0; r < 100; r++){
		_root = NULL;
		_nodeArray.clear();
		_contour.clear();
		_leafArray.clear();
		clearCoor();
		Block::setMaxX(0);
		Block::setMaxY(0);
		blkArray = ori_blkArray;
		random_shuffle(blkArray.begin(), blkArray.end());
		for(int i = 0; i < blkArray.size(); i++){
			Node* tempNode = new Node(blkArray[i],i);
			insertNode(tempNode, 0);
		}
		buildContour(_root);
		pair<double,double> cost;
		double nowCost = -1;
		cost = calCost();
		Anorm += cost.first;
		Wnorm += cost.second;
		nowCost = alpha * cost.first + beta * cost.second;
		//cout << "round " << r << " A: " << cost.first << " W: " << cost.second <<" cost: " << nowCost << endl;
		if(Best_cost > nowCost){
			Best_cost = nowCost;
			deepClone(_root, _best_root, NULL, -1);
			copyBestBlock();
			if(Block::getMaxX() <= _outlineX && Block::getMaxY() <= _outlineY)
				solve = true;
		}
	}
	Anorm /= 100;
	Wnorm /= 100;
	roundCounter = 100;
/*	printTree(_best_root);
	Block::setMaxX(0);
	Block::setMaxY(0);
	_contour.clear();
	clearCoor();
	buildContour(_best_root);
	printContour();*/
	cout << "Best_cost: " << Best_cost << endl;
	cout << "Anorm: " << Anorm << " Wnorm: " << Wnorm << endl;
	cout << "-------------Initial end-----------------" << endl;
	
	// SA
	cout << "-------------Simulated Annealing start-----------------" << endl;
	double T = 1000, T1 = T;
	int rangeOP1 = 33, rangeOP2 = 66;
	int counter = 0;
	double avgUphill = 0;
	double totalDeltaCost = 0;
	int roundTimes = 10;
	pair<double,double> last_pair;
	double last_cost = INT_MAX;
	int acceptCount = 0;
	int rejectCount = 0;
	// get best data
	writeBackBlock();
	_root = NULL;
	deepClone(_best_root, _root, NULL, -1);
	printTree(_root);
	_contour.clear();
	clearCoor();
	Block::setMaxX(0);
	Block::setMaxY(0);
	buildContour(_root);
	printContour();
	plot();
//	cout << "maxX & maxY: " << Block::getMaxX() << " " << Block::getMaxY() << endl;
	last_pair = calCost();
//	cout << "last_cost: " << last_pair.first << " " << last_pair.second << endl;
//	cout << "Anorm: " << Anorm << " Wnorm: " << Wnorm << endl;
	last_cost = alpha * (last_pair.first / Anorm) + beta * (last_pair.second / Wnorm);
//	cout << "last_cost: " << last_cost << endl;
	deepClone(_root, _last_root, NULL, -1);
	if(Block::getMaxX() <= _outlineX && Block::getMaxY() <= _outlineY)
		solve = true;
	while(1){
		if(counter > 50 && solve){
			break;
		}
		if(T < 0.03){
			T = T1;
		}
		if(counter > 100 && counter % 100 == 0){
			int abc;
			cin >> abc;
			plot();
		}
		double curDeltaCost = 0;
		bool isRotate= false;
		for(int r = 0; r < roundTimes; r++){
			//plot();
			pair<double,double> new_pair;
			double new_cost = 0;
			Block* mvBlock;
			// get new tree
			int rnum = rand() % 100;
			if(rnum < rangeOP1){
				mvBlock = rotateBlock();
				isRotate = true;
			}
			else if(rangeOP1 < rnum && rnum < rangeOP2){
				moveBlock();
				isRotate = false;
			}
			else{
				swapBlock();
				isRotate = false;
			}
			// get new tree data
			_contour.clear();
			clearCoor();
			Block::setMaxX(0);
			Block::setMaxY(0);
			buildContour(_root);
			new_pair = calCost();
			new_cost = alpha * (new_pair.first / Anorm) + beta * (new_pair.second / Wnorm);
			curDeltaCost += new_cost - last_cost;
			/*if(roundCounter > 100000 && roundCounter % 100000 == 0)
				plot();*/
			cout << "new_cost: " << new_cost << " last_cost: " << last_cost << endl;
			if(new_cost < last_cost){
				cout << "best cost: " << Best_cost << endl;
				if(Best_cost > new_cost){
					Best_cost = new_cost;
					deepClone(_root, _best_root, NULL, -1);
					copyBestBlock();
					if(roundCounter > 100000){
						plot();
					}
				}
				if(Block::getMaxX() <= _outlineX && Block::getMaxY() <= _outlineY)
					solve = true;
				deepClone(_root, _last_root, NULL, -1);
				last_cost = new_cost;
				acceptCount++;
			}
			else{
				// check if uphill
				double prob = exp(-(new_cost-last_cost)/T) * 100;
				rnum = rand() % 100;
				cout << "prob: " << prob << endl;
				if(rnum < prob){
					deepClone(_root, _last_root, NULL, -1);
					cout << "rnum: " << rnum << " prob: " << prob << endl;
			//		cout << "accept" << endl;
					acceptCount++;
				}
				else{
					if(isRotate){
						int width = mvBlock->getWidth();
						mvBlock->setWidth(mvBlock->getWidth(true));
						mvBlock->setHeight(width);
						isRotate = false;
					}
					// regression
					_root = NULL;
					deepClone(_last_root, _root, NULL, -1);
					_contour.clear();
					clearCoor();
					Block::setMaxX(0);
					Block::setMaxY(0);
					buildContour(_root);
					rejectCount++;
				}
				// sum uphill cost
				avgUphill += new_cost - last_cost;
			//	cout << "avgUphill: " << avgUphill << endl;
			//	cout << "alpha: " << alpha << " beta: " << beta << endl;
			//	cout << "new_cost: " << new_cost << " last_cost: " << last_cost << endl;
			}
			roundCounter++;
		}
		cout << "------------------------------------------------" << endl;
		cout << "T round: " << counter << endl;
		cout << "before T: " << T << endl;
		if(counter == 0){
			totalDeltaCost += avgUphill;
			avgUphill /= double(roundTimes);
			cout << "total avgUphill: " << avgUphill << endl;
			T = avgUphill / abs(log(0.95));
		}
		else if(1 <= counter && counter <= 7){
			totalDeltaCost += curDeltaCost;
			cout << "curDeltaCost: " << curDeltaCost/roundTimes << endl;
			T = T1 * (curDeltaCost / roundTimes) / counter / 100;
		}
		else{
			totalDeltaCost += curDeltaCost;
			cout << "curDeltaCost: " << curDeltaCost/roundTimes << endl;
			T = T1 * (curDeltaCost / roundTimes) / counter;
		}
		counter++;
		cout << "after T: " << T << endl;
	}
	writeBackBlock();
	_root = NULL;
	deepClone(_best_root, _root, NULL, -1);
	_contour.clear();
	clearCoor();
	Block::setMaxX(0);
	Block::setMaxY(0);
	buildContour(_root);
	plot();


	
//	cout << "_last_root: " << _last_root->getBlock()->getName() << " (" << _last_root->getBlock()->getX1() << "," << _last_root->getBlock()->getY1() << "," << _last_root->getBlock()->getX2() << "," << _last_root->getBlock()->getY2() << ") <" << _last_root->getBlock()->getWidth() << "," << _last_root->getBlock()->getHeight() << ">" << endl;
//	cout << "root: " << _root->getBlock()->getName() << " (" << _root->getBlock()->getX1() << "," << _root->getBlock()->getY1() << "," << _root->getBlock()->getX2() << "," << _root->getBlock()->getY2() << ") <" << _root->getBlock()->getWidth() << "," << _root->getBlock()->getHeight() << ">" << endl;
//	cout << "----------------------------------------" << endl;
//	cout << "root: " << _root->getBlock()->getName() << " (" << _root->getBlock()->getX1() << "," << _root->getBlock()->getY1() << "," << _root->getBlock()->getX2() << "," << _root->getBlock()->getY2() << ") <" << _root->getBlock()->getWidth() << "," << _root->getBlock()->getHeight() << ">" << endl;
	/*string b1 = "bk2";
	string b2 = "bk14b";
	cout << "bk2: " << _blkMap[b1]->getName() << " (" << _blkMap[b1]->getX1() << "," << _blkMap[b1]->getY1() << "," << _blkMap[b1]->getX2() << "," << _blkMap[b1]->getY2() << ")" << endl;
	cout << "bk14b: " << _blkMap[b2]->getName() << " (" << _blkMap[b2]->getX1() << "," << _blkMap[b2]->getY1() << "," << _blkMap[b2]->getX2() << "," << _blkMap[b2]->getY2() << ")" << endl;*/
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
		int randPos = rand() % _nodeArray.size();
		Node* oriNode = _nodeArray[randPos];
		Node* parNode = oriNode->getParent();
		//cout << "randNode: " << oriNode->getBlock()->getName() << endl;
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
		// insert new node to leaf node and node array
		bool isInArray = false;
		for(int i = 0; i < _leafArray.size(); i++){
			if(_leafArray[i] == mvNode)
				isInArray = true;
		}
		if(!isInArray)
			_leafArray.push_back(mvNode);
		_nodeArray.push_back(mvNode);
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

	// clear pointer
	mvNode->setParent(NULL);
	mvNode->setLeft(NULL);
	mvNode->setRight(NULL);

	// remove from node array
	for(int i = 0; i < _nodeArray.size(); i++){
		if(_nodeArray[i] == mvNode){
			_nodeArray.erase(_nodeArray.begin()+i);
			break;
		}
	}
}

Block* Floorplanner::rotateBlock(){
	int rnum = rand() % _nodeArray.size();
	Node* mvNode = _nodeArray[rnum];
	Block* mvBlock = mvNode->getBlock();
	int newWidth = mvBlock->getWidth(true);
	int newHeight = mvBlock->getHeight(true);
	//cout << "mvBlock: " << mvBlock->getName() << " <" << mvBlock->getWidth() << "," << mvBlock->getHeight() << ">" << endl;
	mvBlock->setWidth(newWidth);
	mvBlock->setHeight(newHeight);
	return mvBlock;
	//cout << "mvBlock: " << mvBlock->getName() << " <" << mvBlock->getWidth() << "," << mvBlock->getHeight() << ">" << endl;
}

void Floorplanner::moveBlock(){
	int rnum = rand() % _nodeArray.size();
	Node* mvNode = _nodeArray[rnum];
	Block* mvBlock = mvNode->getBlock();
	deleteNode(mvNode);
	insertNode(mvNode, 1);
	//cout << "mvBlock: " << mvBlock->getName() << " pointer: " << mvNode->getParent() << " " << mvNode->getLeft() << " " << mvNode->getRight() << endl;
}

void Floorplanner::swapBlock(){
	bool check = true;
	Node* mvNode1;
	Node* mvNode2;
	// choose two block and check relation between two node
	while(check){
		int rnum1 = rand() % _nodeArray.size();
		mvNode1 = _nodeArray[rnum1];
		_nodeArray.erase(_nodeArray.begin() + rnum1);
		int rnum2 = rand() % _nodeArray.size();
		mvNode2 = _nodeArray[rnum2];
		_nodeArray.erase(_nodeArray.begin() + rnum2);
		check = checkNode(mvNode1, mvNode2);
		if(check){
			_nodeArray.push_back(mvNode1);
			_nodeArray.push_back(mvNode2);
		}
	}
	/*printNode(mvNode1);
	printNode(mvNode2);
	cout << "-------------------------------------" << endl;*/
	//cout << "swap two node: " << mvNode1->getBlock()->getName() << "," << mvNode2->getBlock()->getName() << endl;
	Node* n1PNode = mvNode1->getParent();
	Node* n1LNode = mvNode1->getLeft();
	Node* n1RNode = mvNode1->getRight();
	Node* n2PNode = mvNode2->getParent();
	Node* n2LNode = mvNode2->getLeft();
	Node* n2RNode = mvNode2->getRight();
	mvNode1->setParent(n2PNode);
	if(n2PNode != NULL){
		if(n2PNode->getLeft() == mvNode2)
			n2PNode->setLeft(mvNode1);
		else if(n2PNode->getRight() == mvNode2)
			n2PNode->setRight(mvNode1);
		else
			cout << "omg who are u!!" << endl;
	}
	mvNode1->setLeft(n2LNode);
	if(n2LNode != NULL)
		n2LNode->setParent(mvNode1);
	mvNode1->setRight(n2RNode);
	if(n2RNode != NULL)
		n2RNode->setParent(mvNode1);
	mvNode2->setParent(n1PNode);
	if(n1PNode != NULL){
		if(n1PNode->getLeft() == mvNode1)
			n1PNode->setLeft(mvNode2);
		else if(n1PNode->getRight() == mvNode1)
			n1PNode->setRight(mvNode2);
		else
			cout << "omg who are u!!" << endl;
	}
	mvNode2->setLeft(n1LNode);
	if(n1LNode != NULL)
		n1LNode->setParent(mvNode2);
	mvNode2->setRight(n1RNode);
	if(n1RNode != NULL)
		n1RNode->setParent(mvNode2);
	_nodeArray.push_back(mvNode1);
	_nodeArray.push_back(mvNode2);
	/*printNode(mvNode1);
	printNode(mvNode2);
	cout << "-------------------------------------" << endl;*/
}

bool Floorplanner::checkNode(Node* node1, Node* node2){
	bool check = false;
	if(node1->getParent() == node2)
		check = true;
	else if(node2->getParent() == node1)
		check = true;
	return check;
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
	//if(mvBlock->getName() == "bk2")
	//	debug = true;
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

pair<double,double> Floorplanner::calCost(){
	double area = double(Block::getMaxX()) * double(Block::getMaxY());
	/*cout << "maxX: " << Block::getMaxX() << " maxY: " << Block::getMaxY() << endl;
	cout << "area: " << area << endl;*/
	double HPWL = 0;
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
	_R_star = Block::getMaxY() / Block::getMaxX();
	//cout << "HPWL: " << HPWL << endl;
	return pair<double,double>(area,HPWL);
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

void Floorplanner::copyBestBlock(){
	_bestBlk.clear();
	for(map<string,Block*>::iterator map_it = _blkMap.begin(); map_it != _blkMap.end(); map_it++){
		string str = map_it->second->getName();
		_bestBlk.push_back(new Block(str, map_it->second->getWidth(), map_it->second->getHeight()));
	}
}

void Floorplanner::writeBackBlock(){
	for(int i = 0; i < _bestBlk.size(); i++){
		_blkMap[_bestBlk[i]->getName()]->setWidth(_bestBlk[i]->getWidth());
		_blkMap[_bestBlk[i]->getName()]->setHeight(_bestBlk[i]->getHeight());
	}
}

void Floorplanner::clearCoor(){
	for(map<string,Block*>::iterator map_it = _blkMap.begin(); map_it != _blkMap.end(); map_it++){
		map_it->second->setPos(0,0,0,0);
	}
}

void Floorplanner::printNode(Node* mvNode){
	Node* parent = mvNode->getParent();
	Node* left = mvNode->getLeft();
	Node* right = mvNode->getRight();
	cout << "node name: " << mvNode->getBlock()->getName();
	if(parent)
		cout << " par: " << parent->getBlock()->getName();
	if(left)
		cout << " left: " << left->getBlock()->getName();
	if(right)
		cout << " right: " << right->getBlock()->getName();
	cout << endl;

}

void Floorplanner::printTree(Node* root){
	if(!root) return;
	printNode(root);
	//cout << " root address: " << root;
	//cout << endl;
	printTree(root->getLeft());
	printTree(root->getRight());
}

void Floorplanner::printContour(){
	for(list<pair<int,int>>::iterator it = _contour.begin(); it != _contour.end(); it++){
		cout << "<" << it->first << "," << it->second << "> ";
	}
	cout << endl;
}
