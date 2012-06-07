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

#include "global_defs.h"
#include "SuffixNode.h"

index_type SuffixNode::end_marker = invalid_idx;
index_type SuffixNode::end_marker_value = -1;
index_type SuffixNode::root = 0;
suffixnodestore_type *SuffixNode::store = 0;
  
