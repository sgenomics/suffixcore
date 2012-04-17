/* -
 * Copyright (c) 2012 Nava Whiteford <new@sgenomics.org>
 * suffixcore - core suffixtree algorithms
 *
 * A license to use this software is granted to users given access to the git repository at: https://github.com/new299/suffixcore
 * A complete copy of this license is located in the license file of this package.
 *
 * In Summary this software:
 *
 * Can be used for creating unlimited applications.
 * Can be distributed in binary or object form only.
 * Commercial use is allowed.
 * Can modify source-code but cannot distribute modifications (derivative works).
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "SuffixTree.h"
#include <stdint.h>
#include <fstream>

using namespace std;

int main(int argc,char ** argv) {

  ifstream input_file(argv[1]);

  SuffixTree st;
  //st.set_compactmode(false);

  unsigned char s;
  for(;!input_file.eof();) {
    s = static_cast<char>(input_file.get());
    if(!input_file.eof()) st.insert(s);
  }

  st.finalise();
  st.compact();
/*
  vector<char> t;
  string ss = argv[2];
  cout << "searching for: " << ss << endl;
  for(size_t n=0;n<ss.size();n++) {
    t.push_back(ss[n]);
  }

  bool r = st.exists(t);
  if(r == true) cout << "found" << endl;
  if(r == false) cout << "not found" << endl;
*/
  //st.validate_tree(true);
  st.process_positions();
  //vector<int> poss = st.all_occurs(t);

  st.dump_stats();

  tialloc::instance()->dump_stats();

  //for(;;);
  //for(int n=0;n<poss.size();n++) cout << "position: " << poss[n] << endl;
}
