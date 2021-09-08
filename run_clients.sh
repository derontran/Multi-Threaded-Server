#!/bin/sh

./server &
./client -c 0 < tests/data0.txt >& tests/out0.txt &
./client -c 1 < tests/data1.txt >& tests/out1.txt &
./client -c 2 < tests/data2.txt >& tests/out2.txt &
./client -c 3 < tests/data3.txt >& tests/out3.txt &
./client -c 4 < tests/data4.txt >& tests/out4.txt &
./client -c 5 < tests/data5.txt >& tests/out5.txt &
./client -c 6 < tests/data6.txt >& tests/out6.txt &
./client -c 7 < tests/data7.txt >& tests/out7.txt &
./client -c 8 < tests/data8.txt >& tests/out8.txt &
#./client -c 9 < data.txt >& out.txt & 



