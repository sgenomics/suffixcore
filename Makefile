# -
# Copyright (c) 2012 Nava Whiteford <new@sgenomics.org>
# suffixcore - core suffixtree algorithms
#
# A license to use this software is granted to users given access to the git repository at: https://github.com/sgenomics/suffixcore
# A complete copy of this license is located in the license file of this package.
#
# In Summary this software:
#
# Can be used for creating unlimited applications.
# Can be distributed in binary or object form only.
# Commercial use is allowed.
# Can modify source-code but cannot distribute modifications (derivative works).

insertfile: insertfile.cpp SuffixTree.h SuffixNode.h *.h *.cpp
	g++ -m32 -O3 insertfile.cpp ./tialloc/tialloc.cpp ./SuffixNode.cpp -I./tialloc -o insertfile
