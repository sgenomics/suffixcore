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

#ifndef SUFFIXNODESTOREMEMVEC
#define SUFFIXNODESTOREMEMVEC

#include "global_defs.h"
#include <vector>
#include <map>
#include <algorithm>
#include "tialloc.h"

class SuffixNode;

using namespace std;

#include <stdint.h>

class SuffixNodeStoreMemVec {

public:
  SuffixNodeStoreMemVec();
  ~SuffixNodeStoreMemVec() {
     free(storage_area);
  }

  void set_compactmode(bool compact_mode);
  index_type push_back_norm();
  index_type push_back_end();
  index_type push_back(SuffixNode &s,int resize=-1);
  SuffixNode &get(index_type idx);
  void set(index_type idx, SuffixNode &s);
  index_type size();
  index_type next_idx(index_type i);
  index_type last_idx();
  void stats();
  void force_compact();
  void compact();
  void dump();

  SuffixNode *storage_area;
  index_type  storage_area_size;
  index_type  storage_area_real_size;
  bool        compact_enabled;
};


#endif
