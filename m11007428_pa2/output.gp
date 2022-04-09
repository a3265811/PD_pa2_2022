reset
set tics
unset key
set title "The result of Floorplan"
set object 1 rect from 6076,462 to 7112,924 fs empty
set label "clkc" at 6594,693 center
set object 2 rect from 0,0 to 378,700 fs empty
set label "clkd" at 189,350 center
set object 3 rect from 6762,0 to 7742,210 fs empty
set label "cmp1" at 7252,105 center
set object 4 rect from 3080,756 to 4060,966 fs empty
set label "cmp2" at 3570,861 center
set object 5 rect from 3080,546 to 4060,756 fs empty
set label "cmp3" at 3570,651 center
set object 6 rect from 4060,924 to 7364,1470 fs empty
set label "cntd" at 5712,1197 center
set object 7 rect from 378,0 to 3682,546 fs empty
set label "cntu" at 2030,273 center
set object 8 rect from 4060,462 to 6076,714 fs empty
set label "npd" at 5068,588 center
set object 9 rect from 3682,0 to 6762,462 fs empty
set label "nps" at 5222,231 center
set object 10 rect from 0,1162 to 2016,1414 fs empty
set label "ppd" at 1008,1288 center
set object 11 rect from 0,700 to 3080,1162 fs empty
set label "pps" at 1540,931 center
set arrow from 0,1414 to 2016,1414 nohead lt 1 lw 3
set arrow from 2016,1162 to 3080,1162 nohead lt 2 lw 3
set arrow from 3080,966 to 4060,966 nohead lt 3 lw 3
set arrow from 4060,1470 to 7364,1470 nohead lt 4 lw 3
set arrow from 7364,210 to 7742,210 nohead lt 5 lw 3
set object 17 rect from 0,0 to 9290,9290 fs empty border 3 
set style line 1 lc rgb "red" lw 3
set border ls 1
set terminal png
set output "graph.png"
plot [0:9290][0:9290]'line' w l lt 2 lw 1
set size square
set terminal x11 persist
replot
exit