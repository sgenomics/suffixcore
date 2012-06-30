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

#ifndef SUFFIXTREE
#define SUFFIXTREE

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <stdlib.h>
#include "SuffixNodeStoreMemVec.h"
#include "SuffixNode.h"
#include "stringify.h"
#include "ProcessPositions.h"

using namespace std;

class SuffixTree {

public:
  SuffixTree(bool uninit=false) {

    if(uninit==true) return;
 
    SuffixNode::store = &store;
    SuffixNode::s     = &s;
   
    SuffixNode root(0,invalid_idx,invalid_idx);
    root.set_suffix_link(0);
    root.set_parent(0);
    root_node = store.push_back(root);

    split_distance = 0;
    split_point_node     = 0;
    split_point_position = 0;

    SuffixNode::end_marker_value = invalid_idx;
    first_non_leaf_node = 0;
    first_non_leaf_n = 0;
  }


  index_type find_tree_position(vector<symbol_type> ss) {
    // follow labels from root down, edge labels.

    index_type current_node = store.get(SuffixNode::root).get_child(ss[0]);

    symbol_type label;
    if(ss.size() == 1) {
      label = 0;
    } else {
      label = ss[1];
    }
 
    size_t search_string_position = 0;
    if(current_node == invalid_idx) return invalid_idx;
    for(;search_string_position < ss.size();) {
      suffixnode_t current_node_tmp = store.get(current_node);
      // follow edge label
      for(index_type position=current_node_tmp.get_label_start();position <= current_node_tmp.get_label_end_translated();position++) {

        if(s[position] != ss[search_string_position]) {return invalid_idx;}
        else {
          search_string_position++;
          if(search_string_position == ss.size()) {
            return current_node;
          }
        }
      }

      label = ss[search_string_position];

      current_node = current_node_tmp.get_child(label);
      if(current_node == invalid_idx) return invalid_idx;
    }

    return invalid_idx;
  }

  string get_substr(index_type start,index_type end) {
    string res;
    if(start > s.size()) return res;
    if(start > end) return res;
    if(end > s.size()) end = s.size();
    for(size_t pos=start;pos<=end;pos++) {
      if((pos < s.size()) && (pos >= 0)) res += s[pos];
    }

    return res;
  }

  vector<size_t> all_occurs(vector<symbol_type> ss,ProcessPositions<suffixnodestore_type,suffixnode_t> &propos,size_t max_hits=-1) {
    //cout << "find all occurs..." << endl;
    vector<size_t> res;

    index_type p = find_tree_position(ss);

    if(p == invalid_idx) {
      return res;
    }

    // grab left and right...

    suffixnode_t p_tmp = store.get(p);
    
    index_type nl = propos.get_next_left_leaf(p);
    index_type nr = propos.get_next_right_leaf(p);


    if(p_tmp.is_leaf()) {
      res.push_back(s.size()-p_tmp.get_depth());
      return res;
    }

    // itterate from nl until we reach nr

    index_type c = nl;

    bool stop=false;
    for(;stop==false;) {

      suffixnode_t c_tmp = store.get(c);
      if(c==nr) { stop=true; }

      if(c_tmp.get_label_start() != invalid_idx) { res.push_back(s.size()-c_tmp.get_depth()); }

      c = propos.get_next_right_leaf(c);

      if(res.size() > max_hits) return res;
    }

    return res;
  }

  bool exists(vector<symbol_type> t) {
    index_type res = exists_node(t);

    if(res == invalid_idx) return false;
    return true;
  }

  index_type exists_node(vector<symbol_type> t) {

    // follow labels from root down, edge labels.

    index_type current_node = store.get(SuffixNode::root).get_child(t[0]);
 
    if(current_node == invalid_idx) return invalid_idx;

    index_type search_string_position = 0;

    for(;search_string_position < t.size();) {
      // follow edge label

      suffixnode_t current_node_tmp = store.get(current_node);
      for(index_type position=current_node_tmp.get_label_start();position <= current_node_tmp.get_label_end_translated();position++) {
        if(s[position] != t[search_string_position]) { return invalid_idx; }
        else {
          search_string_position++;
          if(search_string_position == t.size()) { return current_node; }
        }
      }

      current_node = current_node_tmp.get_child(t[search_string_position]);
      if(current_node == invalid_idx) return invalid_idx;
    }

    return invalid_idx;
  }

  index_type extend2(index_type insertion_point,index_type symbol_index_start,index_type symbol_index_end,bool &split,index_type &fnode,index_type &fpos) {
    fnode = insertion_point;
    fpos  = symbol_index_start;

    suffixnode_t insertion_point_tmp0 = store.get(insertion_point);
    index_type label_start = insertion_point_tmp0.get_label_start();
    index_type edge_length = insertion_point_tmp0.get_label_length();

    index_type insert_len = symbol_index_end - symbol_index_start;
    // Check edge label
    {
      suffixnode_t insertion_point_tmp = store.get(insertion_point);
      // this means we're at the root node, it's kind of special!
      if(insertion_point_tmp.get_label_start() == invalid_idx) {

        // if a child exists, go to it, without consuming
        index_type child = insertion_point_tmp.get_child(s[symbol_index_start]);
        if(child != invalid_idx) {
          return extend2(child,symbol_index_start,symbol_index_end,split,fnode,fpos);
        } else {
          // if it doesn't exist add it.
          SuffixNode sn(insertion_point,symbol_index_start,0);
          sn.set_label_start(symbol_index_start);
          sn.set_label_end(invalid_idx);
          sn.set_suffix_link(0);
          split = true;
          sn.set_depth(insertion_point_tmp.get_depth());

          index_type sn_idx = store.push_back(sn);
          suffixnode_t insertion_point_tmp1 = store.get(insertion_point);
          insertion_point_tmp1.set_child(s[symbol_index_start],sn_idx);// next insert

          store.set(insertion_point,insertion_point_tmp1);
          return sn_idx;
        }
      }
    }

    bool dontdoit=false;
    if(edge_length == 0) {
      suffixnode_t insertion_point_tmp = store.get(insertion_point);
      // match at label start position?
      if((s[insertion_point_tmp.get_label_start()] == s[symbol_index_start])) {
	symbol_index_start++;
	dontdoit=true;
      } else {
	if((s[insertion_point_tmp.get_label_start()] != s[symbol_index_start])) {
	  // mismatch at label start position - add new child to parent!

	  index_type parent = insertion_point_tmp.get_parent();
	  suffixnode_t parent_tmp = store.get(parent);
	  index_type child  = parent_tmp.get_child(s[symbol_index_start]);
	  if(child == invalid_idx) {
	    // no child here, add one.
	    SuffixNode sn(parent,symbol_index_start,0);
	    sn.set_label_start(symbol_index_start);
	    split=true;
	    sn.set_depth(parent_tmp.get_depth());
	    index_type sn_idx = store.push_back(sn);

	    suffixnode_t parent_tmp1 = store.get(parent);
	    parent_tmp1.set_child(s[symbol_index_start],sn_idx);
	    store.set(parent,parent_tmp1);
	    return sn_idx;
	  } else {
	    return extend2(child,symbol_index_start+1,symbol_index_end,split,fnode,fpos);
	  }
	}
      }
    }

    index_type start_val =0;
    start_val = symbol_index_end - symbol_index_start; 
    // consume edge label
    if(!dontdoit)
    for(index_type n=start_val;(n<=edge_length) && (n<=(insert_len));) {
      if(s[symbol_index_start+n] != s[label_start+n]) {
        // mismatch on edge label
        //Extend2 condition 1: Mismatch in edge label

        index_type b_idx = store.push_back_norm();
        index_type c_idx = store.push_back_end();

        suffixnode_t insertion_point_tmp = store.get(insertion_point);
        index_type old_parent            = insertion_point_tmp.get_parent();
        suffixnode_t old_parent_tmp      = store.get(insertion_point_tmp.get_parent());

        index_type old_label_start = insertion_point_tmp.get_label_start();

        suffixnode_t b = store.get(b_idx);
        suffixnode_t c = store.get(c_idx);
        b.set_label_start (old_label_start);
        b.set_label_end   (old_label_start+n-1);
        c.set_label_start (symbol_index_start+n);
        c.set_label_end   (SuffixNode::end_marker);

        insertion_point_tmp.set_label_start(old_label_start+n);
        store.set(insertion_point,insertion_point_tmp); // WHY IS THIS REQUIRED HERE?

        index_type old_parent_child_symbol = old_parent_tmp.find_child(insertion_point); // TODO: make constant time please?

        old_parent_tmp.set_child(old_parent_child_symbol,b_idx);
        store.set(old_parent,old_parent_tmp);

        b.set_child(s[old_label_start+n],insertion_point);
        b.set_child(s[symbol_index_start+n],c_idx);

        insertion_point_tmp.set_parent(b_idx);
        c.set_parent(b_idx);
        b.set_parent(old_parent);
        b.set_suffix_link(0);// (this is pointed after the next insertion in insert)
        c.set_suffix_link(0);

        // ADD NODE: b_idx
        // ADD NODE: c_idx 
        split=true;
        b.set_depth(old_parent_tmp.get_depth()+b.get_label_length_r()+1);
        c.set_depth(b.get_depth());
        store.set(b_idx,b);
        store.set(c_idx,c);

        if(insertion_point_tmp.get_label_end() == SuffixNode::end_marker) { insertion_point_tmp.set_depth(b.get_depth());}
        else                                                              { insertion_point_tmp.set_depth(b.get_depth()+insertion_point_tmp.get_label_length_r()+1);}
        store.set(insertion_point,insertion_point_tmp);
        //extend2 endpoint 3
        return c_idx;
      }

      n++;
    }

    // Edge label matched insertion string completely.
    if((edge_length+1) > insert_len) { 
      split=false;

      return insertion_point;
    }

    //Extend2 condition 2: checking children
    index_type pos = symbol_index_start + edge_length + 1;

    if(dontdoit) pos--;
    if(pos > symbol_index_end) {
      //extend2 endpoint 5: broken?
      return insertion_point;
    }

    symbol_type child_sym = s[pos];

    {
      suffixnode_t insertion_point_tmp = store.get(insertion_point);
      if(insertion_point_tmp.get_child(child_sym) == invalid_idx) {

	// we seem to get here even though insertion point has a label length of 0... how?
	// we should replace the edge label with our own...

	if(insertion_point_tmp.get_label_end() == invalid_idx) {
	  store.set(insertion_point,insertion_point_tmp);
	  return insertion_point;
	}

	if(insertion_point_tmp.is_leaf()) {
	  cout << "WTF it's a leaf?!?" << endl;
	  exit(0);
	}

	//Extend2 condition 2b: no children at point past edge label

	// if we get here we really MUST have other children.
	SuffixNode newnode(insertion_point,pos,0);
	newnode.set_label_end(SuffixNode::end_marker);

	split=true;
	newnode.set_depth(insertion_point_tmp.get_depth());
	index_type n_idx = store.push_back(newnode);

	//ADD NODE: n_idx
	suffixnode_t insertion_point_tmp1 = store.get(insertion_point);
	insertion_point_tmp1.set_child(child_sym,n_idx);
	store.set(insertion_point,insertion_point_tmp1);

	//extend2 endpoint 6
	return n_idx;

      }
    }

    // if a child does exist, recurse

    suffixnode_t insertion_point_tmp = store.get(insertion_point);
    return extend2(insertion_point_tmp.get_child(child_sym),pos,symbol_index_end,split,fnode,fpos);
  }

  void finalise() {
    insert(final_symbol,true);
  }

  void insert(string insertion_string) {
    for(size_t n=0;n<insertion_string.size();n++) {
      insert(insertion_string[n]);
    }
  }

  void insert(symbol_type current_symbol,bool finalise=false) {

    s.push_back(current_symbol);

    SuffixNode::end_marker_value++;

    index_type last_node=0;
//    vector<vector<index_type> > doall;

    last_node = first_non_leaf_node;
    bool first=true;
    bool split=false;

    index_type predict_node = 0;
    index_type predict_pos = 0;

    bool first_non_leaf_flag = true;
    bool at_end=false;
    bool last_at_end=false;
    index_type c=0;
    for(size_t n=first_non_leaf_n;n<s.size();n++) {

      suffixnode_t last_node_tmp1 = store.get(last_node);

      index_type newnode;

      bool last_split=split;
      split=false;

      predict_node = last_node_tmp1.get_suffix_link();
      if(first) predict_node = first_non_leaf_node;

      suffixnode_t predict_node_t1 = store.get(predict_node);

      if(predict_node_t1.get_parent() == 0) { predict_pos = n;} 

      // Now need to perform 'canonisation' analog.

     // lazy canonisation
     predict_pos = n + (predict_node_t1.get_depth()-predict_node_t1.get_label_length_r())-1;
     if((predict_node_t1.get_parent() != 0) && (predict_node_t1.get_suffix_link() != 0))
      for(;;) {
        suffixnode_t predict_node_c = store.get(predict_node);

        predict_pos = n + (predict_node_c.get_depth()-predict_node_c.get_label_length_r())-1;
        index_type ins_size = s.size()-n;
        index_type c_depth  = predict_node_c.get_depth();
        if(ins_size >= c_depth) break;
        predict_node = predict_node_c.get_parent();
      }

      if(n  == (s.size()-1)) {predict_node = 0;}

      suffixnode_t predict_node_t2 = store.get(predict_node);

      if(predict_node_t2.get_parent() == 0) {predict_node = 0; predict_pos = n;} 

      index_type fnode = 0;
      index_type fpos  = 0;
      newnode = extend2(predict_node,predict_pos,s.size()-1,split,fnode,fpos);
      suffixnode_t newnode_tmp = store.get(newnode);
      last_at_end = at_end;
      at_end = false;
      index_type ins_len = s.size()-1-n+1;
      index_type lab_len = newnode_tmp.get_depth();
      if(ins_len == lab_len) at_end = true;


      bool ends_at_magic_leaf = (newnode_tmp.is_leaf() && at_end);
      bool is_the_last_symbol = (n == s.size()-1);
      bool implicit_match     = first_non_leaf_flag && !at_end;

      if(!first) {

        if(!is_the_last_symbol && (ends_at_magic_leaf) && !implicit_match) {  
          // keep going, we don't need to visit these again.
        } else 
        if(first_non_leaf_flag) {
          first_non_leaf_node = last_node;
          first_non_leaf_n    = n-1;
          first_non_leaf_flag = false;
        }
      }

      predict_pos = fpos;

      suffixnode_t last_node_tmp2 = store.get(last_node); // required.
      if((!first) && (split || (at_end && last_at_end && newnode_tmp.is_leaf()))) {
        //if(last_node_tmp2.get_suffix_link() != newnode) {  // only perform set if there is a change
          last_node_tmp2.set_suffix_link(newnode);
          store.set(last_node,last_node_tmp2);
        //}
      }

      if((!first) && last_split) {
 //       if(last_node_tmp2.get_suffix_link() != newnode) {  // only perform set if there is a change
          last_node_tmp2.set_suffix_link(newnode);
          store.set(last_node,last_node_tmp2);
 //       }
        suffixnode_t last_node_tmp_parent = store.get(last_node_tmp2.get_parent());
        //if(last_node_tmp_parent.get_suffix_link() != newnode_tmp.get_parent()) {  // only perform set if there is a change
          last_node_tmp_parent.set_suffix_link(newnode_tmp.get_parent());

          store.set(last_node_tmp2.get_parent(),last_node_tmp_parent);
        //}
      }

      last_node = newnode; // was newnode
      if((!split) && (c != invalid_idx) && !at_end) {
        first=false;
        break;
      }
      c++;

      first=false;
    }
  }

  void dump() {
    cout << "****************************** Tree dump" << endl;
    dump_child(0);
    dump(0);
    cout << "****************************** Tree dump complete" << endl;
  }

  void dump(index_type n) {

    suffixnode_t n_tmp = store.get(n);

    for(int i=0;i<symbol_size;i++) {
      if(n_tmp.get_child(i) != invalid_idx) { 
        dump_child(n_tmp.get_child(i));
        dump      (n_tmp.get_child(i));
      }
    }
  }


  void dump_child(index_type n) {
    suffixnode_t n_tmp = store.get(n);
    cout << "node: " << n << endl;
    cout << "label: " << n_tmp.get_label_start() << " ";
    if(n_tmp.get_label_end() == SuffixNode::end_marker) cout << n_tmp.get_label_end() << "(" << SuffixNode::end_marker_value << ")" << endl;
                                                  else cout << n_tmp.get_label_end() << endl;
   // cout << "suffix_link    : " << n_tmp.suffix_link    << endl;
   // cout << "parent         : " << n_tmp.parent         << endl;
   // cout << "depth          : " << n_tmp.depth          << endl;

    bool has_child=false;
    for(int i=0;i<symbol_size;i++) {
      if(n_tmp.get_child(i) != invalid_idx) {
        cout << "children[" << i << "]:" << n_tmp.get_child(i) << endl; 
        has_child=true;
      }
    }

    if(has_child==false) {
      cout << "************** no children" << endl;
    }
  }

  string get_path_label(index_type n) {

    if(n==0) return "";

    vector<index_type> parents;
    index_type parent = n;
    for(;parent != 0;) {
      parents.push_back(parent);
      suffixnode_t parent_tmp = store.get(parent);
      parent = parent_tmp.get_parent();
    }

    string my_path_label;
    for(size_t n=parents.size()-1;n>=0;n--) {

      suffixnode_t parentsn_tmp = store.get(parents[n]);
      index_type start = parentsn_tmp.get_label_start();
      index_type end   = parentsn_tmp.get_label_end_translated();
      for(index_type i=start;i<=end;i++) {
        my_path_label += s[i];
      }
    }
    return my_path_label;
  }

  string get_label(index_type n) {
    if(n==0) return "";

    suffixnode_t n_tmp = store.get(n);
    index_type start = n_tmp.get_label_start();
    index_type end   = n_tmp.get_label_end_translated();
    string my_path_label;
    for(index_type i=start;i<=end;i++) {
      my_path_label += s[i];
    }
    return my_path_label;
  }

  bool validate_suffix_link(index_type n,bool dump=false) {
 
   // get my path label
   // get parent chain to root.

    suffixnode_t n_tmp = store.get(n);
    string my_path_label     = get_path_label(n) ;
    string suffix_path_label = get_path_label(n_tmp.get_suffix_link());
  
    if(dump) cout << "validating link from/to: " << n << "," << (index_type) n_tmp.get_suffix_link() << " label lens: " << my_path_label.size() << "," << suffix_path_label.size() << endl;
    if((static_cast<index_type>(my_path_label.size())-1) > 0) {
      if((suffix_path_label.size()) < ((my_path_label.size()-1))) {
	if(dump) cout << "********************************************* SUFFIXLINK DOES NOT VALIDATE, SIZE ERROR1" << endl;
	return false;
      } else
      if((suffix_path_label.size()) != ((my_path_label.size()-1))) {
	if(dump) cout << "********************************************* SUFFIXLINK DOES NOT VALIDATE, SIZE ERROR2" << endl;
	return false;
      } else {
	for(size_t n=0;n<my_path_label.size()-1;n++) {
	  if(suffix_path_label[n] != my_path_label[n+1]) {
	    if(dump) cout << "****************************************************** SUFFIXLINK DOES NOT VALIDATE" << endl;
	    return false;
	  }
	}
      }
    }
    return true;
  }

  bool validate_parent(index_type n,bool dump=false) {

    suffixnode_t n_tmp = store.get(n);
    index_type parent =  n_tmp.get_parent();

    suffixnode_t parent_tmp = store.get(parent);
    bool ok=false;
    for(int i=0;i<=symbol_size;i++) { if(parent_tmp.get_child(i) == n) ok = true; }

    if(n == 0) ok = true;
    if(ok != true) {
      if(dump) cout << "******************************** ERROR PARENT LINK NOT VALIDATED: node " << n << " stored parent: " << parent << " but this node not child" << endl;
      return false;
    }
    return true;
  }

  bool validate_depth(index_type n,bool dump=false) {

    //bool valid = true;
    suffixnode_t n_tmp = store.get(n);
    index_type s_depth = n_tmp.get_depth();

    index_type depth = 0;

    bool fin=false;

    if(n_tmp.get_suffix_link() == n) fin = true;
    for(index_type c=n;fin==false;) {
      suffixnode_t c_tmp = store.get(c);
      if(c_tmp.get_suffix_link() == c) fin = true;
      else {

        depth += c_tmp.get_label_length_r()+1;

        c = c_tmp.get_parent();
      }
    }

    if(s_depth != depth) {
      if(dump) cout << "ERROR, DEPTH VALIDATION FAILED node = " << n << " calc depth: " << depth << " stored depth: " << s_depth << endl;
      return false;
    }

    return true;
  }

  size_t size() {
    return s.size();
  }

  void dump_stats() {
    cout << "string size: " << s.size() << endl; 
    cout << "node count : " << store.size() << endl;

    store.stats();
  }

  bool validate_tree(bool dump=false) {
    for(index_type n=1;n!=store.last_idx();n=store.next_idx(n)) {
      //if(dump) cout << "validating: " << n << endl;
      //bool v1 = validate_depth(n,dump);
      //bool v2 = validate_suffix_link(n,dump);
      bool v3 = validate_parent(n,dump);
      //if(v1 == false) {cout << "failed depth validation" << endl;       }
      //if(v2 == false) {cout << "failed suffix link validation" << endl; }
      if(v3 == false) {cout << "failed parent validation" << endl;      }

      //if(v1 == false) return false;
      //if(v2 == false) return false;
      if(v3 == false) return false;
    }
    return true;
  }

  void compact() {
    store.force_compact();
  }

  void set_compactmode(bool compactmode) {
    store.set_compactmode(compactmode);
  }

  void save_members(string filename) {
    // write my members
    ofstream membersfile(filename.c_str(),ios_base::app); // open for append
    membersfile << "suffixtree_root_node="            << root_node            << endl;
    membersfile << "suffixtree_split_point_node="     << split_point_node     << endl;
    membersfile << "suffixtree_split_point_position=" << split_point_position << endl;
    membersfile << "suffixtree_split_distance="       << split_distance       << endl;
    membersfile << "suffixtree_first_non_leaf_node="  << first_non_leaf_node  << endl;
    membersfile << "suffixtree_first_non_leaf_n="     << first_non_leaf_n     << endl;
    membersfile.close();
    SuffixNode::save_members(filename);
  }

  void load_members(string filename) {
    ifstream membersfile(filename.c_str());
  
    for(;!membersfile.eof();) {
      string line;
      getline(membersfile,line);

      stringstream cline(line);

      string member;
      string value;
      getline(cline,member,'=');
      getline(cline,value);

      if(member == "suffixtree_root_nodex"           ) root_node            = convertTo<int>(value); 
      if(member == "suffixtree_split_point_node"     ) split_point_node     = convertTo<int>(value); 
      if(member == "suffixtree_split_point_position" ) split_point_position = convertTo<int>(value); 
      if(member == "suffixtree_split_distance"       ) split_distance       = convertTo<int>(value); 
      if(member == "suffixtree_first_non_leaf_node"  ) first_non_leaf_node  = convertTo<int>(value); 
      if(member == "suffixtree_first_non_leaf_n"     ) first_non_leaf_n     = convertTo<int>(value); 
    }
    membersfile.close();
    SuffixNode::load_members(filename);
  }

  store_type s;
  suffixnodestore_type store;

  index_type root_node;             ///< The root node, usually 0
  index_type split_point_node;      ///< Point of last insertion/split in tree (node index)
  index_type split_point_position;  ///< Point of last insertion/split in tree (label position)
  index_type split_distance;        ///< distance to last split point
  index_type first_non_leaf_node;
  index_type first_non_leaf_n;
};

#endif
