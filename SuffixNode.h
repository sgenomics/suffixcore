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

#ifndef SUFFIXNODE
#define SUFFIXNODE

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <stdlib.h>
#include <stdint.h>
#include "tialloc.h"
#include "SuffixNodeStoreMemVec.h"

using namespace std;

class SymbolPair {
  public:

  SymbolPair(symbol_type s,int32_t i) : symbol(s),index(i) {
  }

  symbol_type symbol;
  int32_t index;
} __attribute__((__packed__));

class normal_node_data {
public:
  int32_t m_depth;
  int32_t m_parent;
  int32_t m_suffix_link;
  int32_t m_label_start;
  int32_t m_next_right_leaf;
  int32_t m_next_left_leaf;
  int32_t m_label_end;
} __attribute__((__packed__));

class end_node_data {
public:
  int32_t m_parent;
  int32_t m_suffix_link;
  int32_t m_label_start;
  int32_t m_next_right_leaf;
} __attribute__((__packed__));

class SuffixNode {

public:

  SuffixNode(int res=0) {
    data = 0;
    resize_for_symbols (res);
    set_symbols_size   (0);
    set_parent         (-1);
    set_label_start    (-1);
    set_label_end      (-1);
    set_next_left_leaf (-1);
    set_next_right_leaf(-1);
    set_depth          (-1);
  }

  SuffixNode(const SuffixNode& other) {
    data = 0;
    *this = other;
  }

  SuffixNode(int32_t parent_in,int32_t label_start_in,int32_t depth_in) {

    data = 0;
    resize_for_symbols(0);

    set_parent(parent_in);
    set_label_start(label_start_in);
    set_depth_raw(depth_in);

    set_suffix_link(0);

    set_label_end       (-1);
    set_next_left_leaf  (-1);
    set_next_right_leaf (-1);
  }

  void clear() {
    set_suffix_link(0);

    set_label_end       (-1);
    set_next_left_leaf  (-1);
    set_next_right_leaf (-1);
  }

  ~SuffixNode() {
    // TODO: fix delete
    if(data != 0) tialloc::instance()->free(data);
    data = 0;
  }

  bool is_leaf() const {
    if(get_symbols_size() == 0) return true;
    return false;
  }

  int get_label_length() {
    if(get_label_start() == -1) return 0;

    if(get_label_end() == end_marker) {
      return end_marker_value-get_label_end(); 
    }

    return get_label_end()-get_label_start();
  }

  int get_label_length_r() {
    if(get_label_start() == -1) return 0;

    if(get_label_end() == end_marker) {
      return end_marker_value-get_label_start(); 
    }

    return get_label_end()-get_label_start();
  }

  int find_child(int c) {
    for(size_t n=0;n<get_symbols_size();n++) {
      if(get_symbol_by_idx(n).index == c) return get_symbol_by_idx(n).symbol;
    }
    return -1;
  }

  int32_t child_count() {
    return get_symbols_size();
  }

  int32_t get_child(symbol_type symbol) {
    if(get_symbols_size() == 0) return -1;

    for(size_t n=0;n<get_symbols_size();n++) {
      if(get_symbol_by_idx(n).symbol == symbol) return get_symbol_by_idx(n).index;
    }
    return -1;
  }

  int32_t child_local_idx(symbol_type symbol) {
    for(size_t n=0;n<get_symbols_size();n++) {
      if(get_symbol_by_idx(n).symbol == symbol) return n;
    }
    return -1;
  }

  void set_child(symbol_type n,int32_t m) {

    if(get_symbols_size() == 0) {
      if(m == -1) return;
 
      resize_for_symbols(1);
      set_symbols_size(1);
      get_symbol_by_idx(0).symbol = n;
      get_symbol_by_idx(0).index  = m;
    }

    int child = child_local_idx(n);
    if(child != -1) {
      if(m != -1) {
        get_symbol_by_idx(child).index = m;
      } else {
        // index for -1 means erase the entry.
        for(size_t i=child;i<(get_symbols_size()-1);i++) {
          get_symbol_by_idx(i) = get_symbol_by_idx(i+1);
        }
        resize_for_symbols(get_symbols_size()-1);
        set_symbols_size(get_symbols_size()-1);
      }
    } else {
      resize_for_symbols(get_symbols_size()+1);
      int idx = get_symbols_size();
      get_symbol_by_idx(idx).symbol = n;
      get_symbol_by_idx(idx).index  = m;
      set_symbols_size(idx+1);
    }
  }

  bool operator==(SuffixNode &other) {
    return equal(other); 
  }

  bool is_child(int32_t idx) {
    for(size_t n=0;n<get_symbols_size();n++) if(get_symbol_by_idx(n).index == idx) return true;
    return false;
  }

  int32_t next_child(int32_t idx) {
    bool next=false;
    for(size_t n=0;n<get_symbols_size();n++) {
      if(next==true) {
        return get_symbol_by_idx(n).index;
      }
      if(get_symbol_by_idx(n).index == idx) { next=true; }
    }
    return -1;
  }

  int32_t get_first_child() {
    if(get_symbols_size() == 0) return -1;
    return get_symbol_by_idx(0).index;
  }

  int32_t get_last_child() {
    if(get_symbols_size() == 0) return -1;
    return get_symbol_by_idx(get_symbols_size()-1).index;
  }

  bool equal(SuffixNode &other,bool dump=false) {
    if(get_parent()          != other.get_parent()     )    { if(dump)  cout << "parent match failure" << endl;          return false; }
    if(get_label_start()     != other.get_label_start())    { if(dump)  cout << "label_start match failure" << endl;     return false; }
    if(get_label_end()       != other.get_label_end()  )    { if(dump)  cout << "label_end match failure mine: " << get_label_end() << " other: " << other.get_label_end() << endl; return false; }
// if(m_children != 0)   if((m_children->equal(*(other.m_children),dump) == false))    { if(dump)  cout << "children match failure" << endl; return false; }

    if(get_symbols_size()  != other.get_symbols_size()) { if(dump)  cout << "children match failure" << endl; return false; }
    for(size_t n=0;n<get_symbols_size();n++) { if(get_symbol_by_idx(n).index != other.get_symbol_by_idx(n).index) {if(dump)  cout << "children match failure" << endl; return false; }    }
    for(size_t n=0;n<get_symbols_size();n++) { if(get_symbol_by_idx(n).symbol != other.get_symbol_by_idx(n).symbol) {if(dump)  cout << "children match failure" << endl; return false; }    }

    if(get_suffix_link()     != other.get_suffix_link())    { if(dump)  cout << "suffix_link match failure" << endl;     return false; }
    if(get_next_left_leaf()  != other.get_next_left_leaf()) { if(dump)  cout << "next_left_leaf match failure" << endl;  return false; }
    if(get_next_right_leaf() != other.get_next_right_leaf()){ if(dump)  cout << "next_right_leaf match failure" << endl; return false; }
    if(get_depth_raw()       != other.get_depth_raw())      { if(dump)  cout << "depth match failure" << endl;           return false; }

    if(dump) cout << "suffixnodes identical" << endl;
    return true;
  }

  void dump() {
    cout << "SuffixNode" << endl;
    cout << "parent         : " << get_parent() << endl;
    cout << "label_start    : " << get_label_start() << endl;
    cout << "label_end      : " << get_label_end() << endl;
    cout << "suffix_link    : " << get_suffix_link() << endl;
    cout << "next_left_leaf : " << get_next_left_leaf() << endl;
    cout << "next_right_leaf: " << get_next_right_leaf() << endl;
    cout << "depth          : " << get_depth_raw() << endl;
    cout << "children      : ";
    for(size_t n=0;n<get_symbols_size();n++) cout << (int) get_symbol_by_idx(n).symbol << "," << get_symbol_by_idx(n).index << " ";
    cout << endl;
  }


  SuffixNode &operator=(const SuffixNode &other) {
    resize_for_symbols(other.get_symbols_size());

    set_parent          (other.get_parent());
    set_label_start     (other.get_label_start());
    set_label_end       (other.get_label_end());
    set_suffix_link     (other.get_suffix_link());
    set_next_left_leaf  (other.get_next_left_leaf());
    set_next_right_leaf (other.get_next_right_leaf());
    set_depth_raw       (other.get_depth_raw());

    copy_children(other);
    return *this;
  }

  int32_t get_label_end_translated() {
    if(get_label_end() == end_marker) {
      return end_marker_value;
    }
    return get_label_end();
  }

  const vector<SymbolPair> get_symbols() {
    vector<SymbolPair> symbols;

    for(size_t n=0;n<get_symbols_size();n++) symbols.push_back(get_symbol_by_idx(n));

    return symbols;
  }

  void set_symbols(const vector<SymbolPair> &s) {
    clear_children();

    for(size_t n=0;n<s.size();n++) set_child(s[n].symbol,s[n].index);
  }

// accessors
public:
  int32_t get_depth_raw() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_depth;
    if(get_data_type() == 2) {
      if(get_parent() == -1) return 0;
      return store->get(get_parent()).get_depth();
      //return (   (end_node_data *)data)->m_depth;
    }

    return -1;
  }

  void set_depth_raw(int32_t depth_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_depth = depth_in;
    //if(get_data_type() == 2) ((   end_node_data *)data)->m_depth = depth_in;
    // something clever for end_node?
  }

  int32_t get_depth() const {
    //TODO: fix the store.get
 //   if(get_data_type() == 2) return store.get(get_parent()).get_depth() + (end_marker_value-get_label_start());

    if(get_label_start() == -1) return 0;
    if(get_label_end()   == -1) return get_depth_raw() + (end_marker_value-get_label_start())+1;
    return get_depth_raw();
  }

  void set_depth(int32_t depth_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_depth = depth_in;
    //if(get_data_type() == 2) ((   end_node_data *)data)->m_depth = depth_in;
    // something clever for end_node?
  }

  int32_t get_parent() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_parent;
    if(get_data_type() == 2) return ((   end_node_data *)data)->m_parent;
    return -1;
  }

  void set_parent(int32_t parent_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_parent = parent_in;
    if(get_data_type() == 2) ((   end_node_data *)data)->m_parent = parent_in;
  }

  int32_t get_suffix_link() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_suffix_link;
    if(get_data_type() == 2) return ((   end_node_data *)data)->m_suffix_link;
    return -1;
  }
  
  void set_suffix_link(int32_t suffix_link_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_suffix_link = suffix_link_in;
    if(get_data_type() == 2) ((   end_node_data *)data)->m_suffix_link = suffix_link_in;
  }

  int32_t get_label_start() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_label_start;
    if(get_data_type() == 2) return ((   end_node_data *)data)->m_label_start;
    return -1;
  }

  void set_label_start(int32_t label_start_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_label_start = label_start_in;
    if(get_data_type() == 2) ((   end_node_data *)data)->m_label_start = label_start_in;
  }

  int32_t get_label_end() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_label_end;
    if(get_data_type() == 2) return -1;
    // something clever for end_node?
    return -1;
  }

  void set_label_end(int32_t label_end_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_label_end = label_end_in;
    if(get_data_type() == 2) if((label_end_in != -1) && (label_end_in != end_marker)) {
      cout << "ERROR SETTING LABEL_END ON END NODE" << endl;
    }
    
    // something clever for end_node?
  }

  int32_t get_next_left_leaf() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_next_left_leaf;
    if(get_data_type() == 2) return ((   end_node_data *)data)->m_next_right_leaf;
    return -1;
  }

  void set_next_left_leaf(int32_t next_left_leaf_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_next_left_leaf = next_left_leaf_in;
 //   if(get_data_type() == 2) ((   end_node_data *)data)->m_next_leaf      = next_left_leaf_in;
  }

  int32_t get_next_right_leaf() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_next_right_leaf;
    if(get_data_type() == 2) return ((   end_node_data *)data)->m_next_right_leaf;
    return -1;
  }

  void set_next_right_leaf(int32_t next_right_leaf_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_next_right_leaf = next_right_leaf_in;
    if(get_data_type() == 2) ((   end_node_data *)data)->m_next_right_leaf = next_right_leaf_in;
  }

  // Symbols access
  void wipe() {
    data = 0; // if the object wasn't properly constructed, wipe the data.
  }

  void clear_children() {
    resize_for_symbols(0);
    set_symbols_size  (0);
  }

  size_t get_allocated_symbol_size() const {
    if(get_data_alloc_size() < sizeof(normal_node_data)) return 0;
    return (get_data_alloc_size()-sizeof(normal_node_data))/sizeof(SymbolPair);
  }

  size_t get_symbols_size() const {
    if(get_data_type() == 2) return 0;
    if(get_data_type() == 1) {
      int t_symbol_size=0;
      t_symbol_size = get_allocated_symbol_size();
      for(int n=t_symbol_size-1;((get_symbol_by_idx(n).symbol == 0) &&
         (get_symbol_by_idx(n).index  == -1)) && (n>=0);n--) {t_symbol_size--;}
      if(t_symbol_size < 0) return 0;
      return t_symbol_size;
    }
    return 0;
  }
/*
  uint8_t get_symbols_size_stored() const {
    if(get_data_type() == 2) return 0;
    if(get_data_type() == 1) {
      uint8_t m_symbol_size =  ((normal_node_data *)data)->m_symbols_size;
      if(m_symbol_size != get_symbols_size()) {
        cout << "ERROR: size mismatch: " << (int) m_symbol_size << " " << (int) get_symbols_size() << endl;
        for(size_t n=0;n<get_allocated_symbol_size();n++) {
          cout << (int) get_symbol_by_idx(n).symbol << " ";
          cout << (int) get_symbol_by_idx(n).index << endl;
        }
        int *i=0;*i=0;
      }
      return m_symbol_size;
    }
    return 0;
  }
*/
  void set_symbols_size(size_t size) {
    if(get_data_type() == 1) {
      clear_symbols(size);
   //   ((normal_node_data *)data)->m_symbols_size = size;
    }
  }

  SymbolPair &get_symbol_by_idx(int32_t idx) const {
    //if(get_data_type() == 0) return SymbolPair(-1,-1); // uh oh
    //if(get_data_type() == 2) return SymbolPair(-1,-1);
    //if(get_data_type() == 1) {
    return *((SymbolPair *) (((uint8_t *)data)+(sizeof(normal_node_data)+(idx*sizeof(SymbolPair)))));
    //}
  }

  void copy_children(const SuffixNode &other) {

    if(other.is_leaf()) { clear_children(); return; }

    if(other.get_symbols_size() != get_symbols_size()) {
      resize_for_symbols(other.get_symbols_size());
      set_symbols_size(other.get_symbols_size());
    }

    for(size_t n=0;n<other.get_symbols_size();n++) {
      get_symbol_by_idx(n).symbol = other.get_symbol_by_idx(n).symbol;
      get_symbol_by_idx(n).index  = other.get_symbol_by_idx(n).index;
    }
  }

  void resize_for_symbols(int32_t new_symbol_size) {

    int32_t old_symbol_size = get_symbols_size();
    int old_data_type = get_data_type();

    // SuffixNodes with two children will almost immediately receive another.
    // These unused allocations will cause fragmention in tialloc, so we allocate 2 to start with.
    if(new_symbol_size == 1) new_symbol_size = 2;

    uint32_t alloc_size = 0;

    if(data == 0) {
      if(new_symbol_size == 0) alloc_size = sizeof(end_node_data);
                          else alloc_size = sizeof(normal_node_data) + (sizeof(SymbolPair)*new_symbol_size);

      #ifdef use_tialloc
      data = tialloc::instance()->alloc(alloc_size);
      #else
      data = malloc(alloc_size);
      #endif

      clear();
      set_symbols_size(old_symbol_size);
      clear_symbols(old_symbol_size);
 
    } else {
      if(new_symbol_size == 0) alloc_size = sizeof(end_node_data);
                          else alloc_size = sizeof(normal_node_data) + (sizeof(SymbolPair)*new_symbol_size);
      
      #ifdef use_tialloc
      data = tialloc::instance()->realloc(data,alloc_size);
      #else
      data = realloc(data,alloc_size);
      #endif

      set_symbols_size(old_symbol_size);
      int new_data_type = get_data_type();

      if((old_data_type == 2) && (new_data_type == 1)) reformat_endnode_to_normalnode();
      if((old_data_type == 1) && (new_data_type == 2)) reformat_normalnode_to_endnode();

      clear_symbols(old_symbol_size);
    }
  }

  void clear_symbols(size_t old_size) {
    if(get_allocated_symbol_size() <= 0) return;

    for(size_t n=old_size;n<get_allocated_symbol_size();n++) {
      get_symbol_by_idx(n).symbol = 0;
      get_symbol_by_idx(n).index  = -1;
    }
  }
 
  void reformat_endnode_to_normalnode() {
    set_next_left_leaf (-1);
    set_label_end      (-1);
    set_symbols_size   (0);
  }

  void reformat_normalnode_to_endnode() {}

  uint32_t get_data_alloc_size() const {
    #ifdef use_tialloc
    return tialloc::instance()->alloc_size(data);
    #else
    return 0;
    //m_symbols = (SymbolPair *) malloc_posix_something...();
    #endif
  }

  // return 0 for unknown, 1 for normal_node_data, 2 for end_node_data
  uint32_t get_data_type() const {
    if(data == 0) return 0;
    if(get_data_alloc_size() > sizeof(end_node_data)) return 1;
                                                 else return 2;

    return 0;
  }

  void *get_data() {
    return data;
  }

  void set_data(void *d) {
    data = d;
  }

private:
  void *data;

public:
  static suffixnodestore_type *store;
  static int32_t end_marker;
  static int32_t end_marker_value;
  static int32_t root;
} __attribute__((__packed__));


#endif
