reset
set tics
unset key
set title "The result of Floorplan"
set object 1 rect from 4284,0 to 5320,462 fs empty
set label "clkc" at 4802,231 center
set object 2 rect from 5320,0 to 5698,700 fs empty
set label "clkd" at 5509,350 center
set object 3 rect from 3304,798 to 4284,1008 fs empty
set label "cmp1" at 3794,903 center
set object 4 rect from 0,0 to 980,210 fs empty
set label "cmp2" at 490,105 center
set object 5 rect from 980,546 to 1960,756 fs empty
set label "cmp3" at 1470,651 center
set object 6 rect from 0,1050 to 3304,1596 fs empty
set label "cntd" at 1652,1323 center
set object 7 rect from 980,0 to 4284,546 fs empty
set label "cntu" at 2632,273 center
set object 8 rect from 1960,546 to 3976,798 fs empty
set label "npd" at 2968,672 center
set object 9 rect from 0,1596 to 3080,2058 fs empty
set label "nps" at 1540,1827 center
set object 10 rect from 980,798 to 2996,1050 fs empty
set label "ppd" at 1988,924 center
set object 11 rect from 4284,700 to 7364,1162 fs empty
set label "pps" at 5824,931 center
set object 12 rect from 0,0 to 10824,7408 fs empty border 3 
set style line 1 lc rgb "red" lw 3
set border ls 1
set terminal png
set output "graph.png"
plot [0:10824][0:7408]'line' w l lt 2 lw 1
set terminal x11 persist
replot
exit