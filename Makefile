
insertfile: insertfile.cpp SuffixTree.h SuffixNode.h *.h *.cpp
	#git clone git@github.com:new299/tialloc.git
	g++ -O3 insertfile.cpp ./tialloc/tialloc.cpp ./SuffixNode.cpp -I./tialloc -o insertfile
