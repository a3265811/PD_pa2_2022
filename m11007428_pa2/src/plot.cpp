#include "floorplanner.h"
#include <fstream>

void Floorplanner::plot(){
/////////////info. to show for gnu/////////////
    int boundWidth = 2*_outlineX;// user-define value (boundary info)
    int boundHeight = 2*_outlineY;// same above
 /////////////////////////////////////////////
 //gnuplot preset
	fstream outgraph("output.gp", ios::out);
	outgraph << "reset\n";
	outgraph << "set tics\n";
	outgraph << "unset key\n";
	outgraph << "set title \"The result of Floorplan\"\n";
	int index = 1;
 // wirte block info into output.gp
	for (auto b: _blkMap)// for block
	{
	    string NodeName = b.second->getName();
	    int x0 = b.second->getX1();
	    int y0 = b.second->getY1();
		int x1 = b.second->getX2();
		int y1 = b.second->getY2();
		int midX = (x0+x1)/2;
		int midY = (y0+y1)/2;
		
		outgraph << "set object " << index << " rect from " 
		  		 << x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty\n"
				 << "set label " << "\"" << NodeName << "\"" << " at " << midX << "," << midY << " center\n";
		index++;  
	}
 // write script to output.gp and execute by system call
	outgraph << "set object " << index << " rect from " 
		  		 << "0" << "," << "0" << " to " << boundWidth << "," << boundHeight << " fs empty border 3 \n";
				 //<< "set label " << "\"" << "Outline" << "\"" << " at " << boundWidth/2+50 << "," << boundHeight+100 << "right\n";

	outgraph << "set style line 1 lc rgb \"red\" lw 3\n";
	outgraph << "set border ls 1\n";
	outgraph << "set terminal png\n";
	outgraph << "set output \"graph.png\"\n";
	outgraph << "plot [0:" << boundWidth << "][0:" << boundHeight << "]\'line\' w l lt 2 lw 1\n";
	outgraph << "set terminal x11 persist\n";
	outgraph << "replot\n";
	outgraph << "exit";
	outgraph.close();

	fstream outline("line", ios::out);
	outline << 0 << " " << _outlineY << endl;
	outline << _outlineX << " " << _outlineY << endl << endl;
	outline << _outlineX << " " << _outlineY << endl;
	outline << _outlineX << " " << 0 << endl;
	int x = system("gnuplot output.gp");
}
