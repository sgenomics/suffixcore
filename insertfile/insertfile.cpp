/* -
 * Copyright (c) 2012 Nava Whiteford <new@sgenomics.org>
 * suffixcore - core suffixtree algorithms
 *
 * A license to use this software is granted to users given access to the git repository at: https://github.com/sgenomics/suffixcore
 * A complete copy of this license is located in the license.txt file of this package.
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
#include "global_defs.h"
#include "SuffixTree.h"
#include <stdint.h>
#include <fstream>
#include "SearchTrans.h"

using namespace std;

int main(int argc,char ** argv) {

  ifstream input_file(argv[1]);

  SuffixTree st;
  //SearchTrans st;

  cout << "SuffixNode size      : " << sizeof(SuffixNode) << endl;
  cout << "normal_node_data size: " << sizeof(normal_node_data) << endl;
  cout << "end_node_data size   : " << sizeof(end_node_data) << endl;

  unsigned char s;
  for(;!input_file.eof();) {
    s = static_cast<char>(input_file.get());
    if(!input_file.eof()) st.insert(s);
  }

  st.finalise();
  st.compact();

  vector<symbol_type> t;
  string ss = argv[2];
  cout << "searching for: " << ss << endl;
  for(size_t n=0;n<ss.size();n++) {
    t.push_back(ss[n]);
  }

  bool r = st.exists(t);
  if(r == true) cout << "found" << endl;
  if(r == false) cout << "not found" << endl;

  //st.validate_tree(true);
  st.process_positions();
  vector<size_t> poss = st.all_occurs(t);

  st.dump_stats();

  //tialloc::instance()->dump_stats();

  for(int n=0;n<poss.size();n++) cout << "position: " << poss[n] << endl;
}
