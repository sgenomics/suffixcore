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
#include <sstream>
#include "stringify.h"
#include "tialloc.h"
#include <fstream>

using namespace std;

class SymbolPair {
  public:

  SymbolPair(symbol_type s,index_type i) : symbol(s),index(i) {
  }

  symbol_type symbol;
  index_type index;
} __attribute__((__packed__));

class normal_node_data {
public:
  index_type m_parent;
  index_type m_suffix_link;
  index_type m_label_start;
  index_type m_depth;
  index_type m_label_end;
} __attribute__((__packed__));

class end_node_data {
public:
  index_type m_parent;
  index_type m_suffix_link;
  index_type m_label_start;
} __attribute__((__packed__));

class SuffixNode {

public:

  SuffixNode(int res=0) {

    if(res < 0) return;

    data = 0;
    resize_for_symbols (res);
    set_symbols_size   (0);
    set_parent         (invalid_idx);
  }

  SuffixNode(const SuffixNode& other) {
    data = 0;
    *this = other;
  }

  SuffixNode(index_type parent_in,index_type label_start_in,index_type depth_in) {

    data = 0;
    resize_for_symbols(0);

    set_parent(parent_in);
    set_label_start(label_start_in);
    set_depth_raw(depth_in);

    set_suffix_link(0);

    set_label_end       (invalid_idx);
  }

  void clear() {
    set_suffix_link(0);

    set_label_end       (invalid_idx);
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

  index_type get_label_length() {
    if(get_label_start() == invalid_idx) return 0;

    if(get_label_end() == end_marker) {
      return end_marker_value-get_label_end(); 
    }

    return get_label_end()-get_label_start();
  }

  index_type get_label_length_r() {
    if(get_label_start() == invalid_idx) return 0;

    if(get_label_end() == end_marker) {
      return end_marker_value-get_label_start(); 
    }

    return get_label_end()-get_label_start();
  }

  index_type find_child(index_type c) {
    for(size_t n=0;n<get_symbols_size();n++) {
      if(get_symbol_by_idx(n).index == c) return get_symbol_by_idx(n).symbol;
    }
    return invalid_idx;
  }

  int32_t child_count() {
    return get_symbols_size();
  }

  index_type get_child(symbol_type symbol) {
    size_t size = get_symbols_size();

    if(size == 0) return invalid_idx;

    for(size_t n=0;n<size;n++) {
      if(get_symbol_by_idx(n).symbol == symbol) return get_symbol_by_idx(n).index;
    }
    return invalid_idx;
  }

  int32_t child_local_idx(symbol_type symbol) {
    for(size_t n=0;n<get_symbols_size();n++) {
      if(get_symbol_by_idx(n).symbol == symbol) return n;
    }
    return invalid_idx;
  }

  void set_child(symbol_type n,index_type m) {

    size_t original_size = get_symbols_size();

    if(original_size == 0) {
      if(m == invalid_idx) return;
 
      resize_for_symbols(1);
      set_symbols_size(1);
      SymbolPair &p = get_symbol_by_idx(0);
      p.symbol = n;
      p.index  = m;
      return;
    }

    index_type child = child_local_idx(n);
    if(child != invalid_idx) {
      if(m != invalid_idx) {
        get_symbol_by_idx(child).index = m;
      } else {
        // index for invalid_idx means erase the entry.
        for(size_t i=child;i<(original_size-1);i++) {
          get_symbol_by_idx(i) = get_symbol_by_idx(i+1);
        }
        resize_for_symbols(original_size-1);
        //set_symbols_size(original_size-1);
      }
    } else {
      resize_for_symbols(original_size+1);
      int idx = original_size;
      SymbolPair &p = get_symbol_by_idx(idx);
      p.symbol = n;
      p.index  = m;
      // set_symbols_size(idx+1);
    }
  }

  bool operator==(SuffixNode &other) {
    return equal(other); 
  }

  bool is_child(index_type idx) {
    for(size_t n=0;n<get_symbols_size();n++) if(get_symbol_by_idx(n).index == idx) return true;
    return false;
  }

  index_type next_child(index_type idx) {
    bool next=false;
    for(size_t n=0;n<get_symbols_size();n++) {
      if(next==true) {
        return get_symbol_by_idx(n).index;
      }
      if(get_symbol_by_idx(n).index == idx) { next=true; }
    }
    return invalid_idx;
  }

  index_type get_first_child() {
    if(get_symbols_size() == 0) return invalid_idx;
    return get_symbol_by_idx(0).index;
  }

  index_type get_last_child() {
    if(get_symbols_size() == 0) return invalid_idx;
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
    if(get_depth_raw()       != other.get_depth_raw())      { if(dump)  cout << "depth match failure" << endl;           return false; }

    if(dump) cout << "suffixnodes identical" << endl;
    return true;
  }

  void dump() const {
    cout << "SuffixNode" << endl;
    cout << "parent         : " << get_parent() << endl;
    cout << "label_start    : " << get_label_start() << endl;
    cout << "label_end      : " << get_label_end() << endl;
    cout << "suffix_link    : " << get_suffix_link() << endl;
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
    set_depth_raw       (other.get_depth_raw());

    copy_children(other);
    return *this;
  }

  index_type get_label_end_translated() {
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
  index_type get_depth_raw() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_depth;
    if(get_data_type() == 2) {
      if(get_parent() == invalid_idx) return 0;
      return store->get(get_parent()).get_depth();
    }

    return invalid_idx;
  }

  void set_depth_raw(index_type depth_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_depth = depth_in;
    //if(get_data_type() == 2) ((   end_node_data *)data)->m_depth = depth_in;
    // something clever for end_node?
  }

  index_type get_depth() const {
    //TODO: fix the store.get
 //   if(get_data_type() == 2) return store.get(get_parent()).get_depth() + (end_marker_value-get_label_start());

    if(get_label_start() == invalid_idx) return 0;
    if(get_label_end()   == invalid_idx) return get_depth_raw() + (end_marker_value-get_label_start())+1;
    return get_depth_raw();
  }

  void set_depth(index_type depth_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_depth = depth_in;
    //if(get_data_type() == 2) ((   end_node_data *)data)->m_depth = depth_in;
    // something clever for end_node?
  }

  index_type get_parent() const {
    return ((   end_node_data *)data)->m_parent;
  }

  void set_parent(index_type parent_in) {
    ((   end_node_data *)data)->m_parent = parent_in;
  }

  index_type get_suffix_link() const {
    return ((   end_node_data *)data)->m_suffix_link;
  }
  
  void set_suffix_link(index_type suffix_link_in) {
    ((   end_node_data *)data)->m_suffix_link = suffix_link_in;
  }

  index_type get_label_start() const {
    return ((end_node_data *)data)->m_label_start;
  }

  void set_label_start(index_type label_start_in) {
    ((   end_node_data *)data)->m_label_start = label_start_in;
  }

  index_type get_label_end() const {
    if(get_data_type() == 1) return ((normal_node_data *)data)->m_label_end;
    if(get_data_type() == 2) return invalid_idx;
    return invalid_idx;
  }

  void set_label_end(index_type label_end_in) {
    if(get_data_type() == 1) ((normal_node_data *)data)->m_label_end = label_end_in;
    if(get_data_type() == 2) if((label_end_in != invalid_idx) && (label_end_in != end_marker)) {
      cout << "ERROR SETTING LABEL_END ON END NODE" << endl;
    }
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
      
      for(int n=t_symbol_size-1;(n>=0) && ((get_symbol_by_idx(n).symbol == 0) &&
         (get_symbol_by_idx(n).index  == invalid_idx));n--) {t_symbol_size--;}
      if(t_symbol_size < 0) return 0;
      return t_symbol_size;
    }
    return 0;
  }

  void set_symbols_size(size_t size) {
    if(get_data_type() == 1) {
      clear_symbols(size);
    }
  }

  SymbolPair &get_symbol_by_idx(index_type idx) const {
    return *((SymbolPair *) (((uint8_t *)data)+(sizeof(normal_node_data)+(idx*sizeof(SymbolPair)))));
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

    int old_data_type=get_data_type(); //0;

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

     // set_symbols_size(old_symbol_size);
      int new_data_type = get_data_type();

      if((old_data_type == 2) && (new_data_type == 1)) reformat_endnode_to_normalnode();

      if(new_symbol_size > old_symbol_size) clear_symbols(old_symbol_size);
    }
  }

  void clear_symbols(size_t old_size) {
    if(get_allocated_symbol_size() <= 0) return;

    for(size_t n=old_size;n<get_allocated_symbol_size();n++) {
      get_symbol_by_idx(n).symbol = 0;
      get_symbol_by_idx(n).index  = invalid_idx;
    }
  }
 
  void reformat_endnode_to_normalnode() {
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

  static void save_members(string filename) {
    ofstream membersfile(filename.c_str(),ios_base::app); // open for append
    membersfile << "suffixnode_end_marker="       << end_marker       << endl;
    membersfile << "suffixnode_end_marker_value=" << end_marker_value << endl;
    membersfile << "suffixnode_root="             << root             << endl;
    membersfile.close();
  }

  static void load_members(string filename) {
    ifstream membersfile(filename.c_str());
  
    for(;!membersfile.eof();) {
      string line;
      getline(membersfile,line);

      stringstream cline(line);

      string member;
      string value;
      getline(cline,member,'=');
      getline(cline,value);

      if(member == "suffixnode_end_marker"      ) end_marker       = convertTo<index_type>(value); 
      if(member == "suffixnode_end_marker_value") end_marker_value = convertTo<index_type>(value); 
      if(member == "suffixnode_root"            ) root             = convertTo<index_type>(value); 
    }
    membersfile.close();
  }

private:
  void *data;

public:
  static suffixnodestore_type *store;
  static store_type *s;
  static index_type end_marker;
  static index_type end_marker_value;
  static index_type root;
} __attribute__((__packed__));


#endif
