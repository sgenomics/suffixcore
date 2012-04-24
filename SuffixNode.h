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

using namespace std;

class SymbolPair {
  public:

  SymbolPair(uint8_t s,int32_t i) : symbol(s),index(i) {
  }

  uint8_t symbol;
  int32_t index;
} __attribute__((__packed__));

class SuffixNode {

public:

  SuffixNode() {
    set_label_start     (-1);
    set_label_end       (-1);
    set_next_left_leaf  (-1);
    set_next_right_leaf (-1);
    set_depth           (-1);
    set_parent          (-1);
    m_symbols = 0;
    m_symbols_size = 0;
  }

  SuffixNode(const SuffixNode& other) {
    m_symbols = 0;
    m_symbols_size = 0;
    *this = other;
  }


  ~SuffixNode() {
    clear_children();
  }

  SuffixNode(int parent_in,int32_t label_start_in,int32_t depth_in) {

    set_parent(parent_in);
    set_label_start(label_start_in);
    set_depth_raw(depth_in);

    set_suffix_link(0);
    m_symbols = 0;
    m_symbols_size = 0;

    set_label_end       (-1);
    set_next_left_leaf  (-1);
    set_next_right_leaf (-1);
  }

  bool is_leaf() {
    if(m_symbols_size == 0) return true;
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

  void clear_children() {
    m_symbols_size = 0;
    #ifdef use_tialloc
    if(m_symbols != 0) tialloc::instance()->free(m_symbols);
    #else
    if(m_symbols != 0) free(m_symbols);
    #endif
    
    m_symbols = 0;
  }

  void copy_children(const SuffixNode &other) {

    if(other.m_symbols_size == 0) {m_symbols_size = 0; clear_children(); return;}

    if(other.m_symbols_size != m_symbols_size) {
      clear_children();
      #ifdef use_tialloc
      m_symbols = (SymbolPair *) tialloc::instance()->alloc(other.m_symbols_size*sizeof(SymbolPair));
      #else
      m_symbols = (SymbolPair *) malloc(other.m_symbols_size*sizeof(SymbolPair));
      #endif
    }


    for(size_t n=0;n<other.m_symbols_size;n++) {
      m_symbols[n].symbol = other.m_symbols[n].symbol;
      m_symbols[n].index  = other.m_symbols[n].index;
    }
    m_symbols_size = other.m_symbols_size;
  }

  int find_child(int c) {

    for(int n=0;n<m_symbols_size;n++) {
      if(m_symbols[n].index == c) return m_symbols[n].symbol;
    }
    return -1;
  }


  int32_t child_count() {
    return m_symbols_size;
  }

  

  int32_t get_child(uint8_t symbol) {
    if(m_symbols_size == 0) return -1;

    for(int n=0;n<m_symbols_size;n++) {
      if(m_symbols[n].symbol == symbol) return m_symbols[n].index;
    }
    return -1;
  }

  int32_t child_local_idx(uint8_t symbol) {
    for(size_t n=0;n<m_symbols_size;n++) {
      if(m_symbols[n].symbol == symbol) return n;
    }
    return -1;
  }

  void set_child(uint8_t n,int32_t m) {
    if(m_symbols_size == 0) {
      if(m == -1) return;

      #ifdef use_tialloc
      m_symbols = (SymbolPair *) tialloc::instance()->alloc(sizeof(SymbolPair)*2);
      #else
      m_symbols = (SymbolPair *) malloc(sizeof(SymbolPair)*2);
      #endif
      m_symbols_size = 1;
      m_symbols[0].symbol = n;
      m_symbols[0].index  = m;
    }

    int child = child_local_idx(n);
    if(child != -1) {
      if(m != -1) {
        m_symbols[child].index = m;
      } else {
        // index for -1 means erase the entry.
        for(size_t i=child;i<((size_t)m_symbols_size-1);i++) {
          m_symbols[i] = m_symbols[i+1];
        }
        m_symbols_size--;

      }
    } else {
        #ifdef use_tialloc
        m_symbols = (SymbolPair *) tialloc::instance()->realloc(m_symbols,(m_symbols_size+1)*sizeof(SymbolPair));
        #else
        m_symbols = (SymbolPair *) realloc(m_symbols,(m_symbols_size+1)*sizeof(SymbolPair));
        #endif

        m_symbols_size++;
        m_symbols[m_symbols_size-1].symbol = n;
        m_symbols[m_symbols_size-1].index  = m;
    }
  }

  bool operator==(SuffixNode &other) {
    return equal(other); 
  }

  bool is_child(int32_t idx) {
    for(size_t n=0;n<m_symbols_size;n++) if(m_symbols[n].index == idx) return true;
    return false;
  }

  int32_t next_child(int32_t idx) {
    bool next=false;
    for(size_t n=0;n<m_symbols_size;n++) {
      if(next==true) {
        return m_symbols[n].index;
      }
      if(m_symbols[n].index == idx) { next=true; }
    }
    return -1;
  }

  int32_t get_first_child() {
    if(m_symbols_size==0) return -1;
    return m_symbols[0].index;
  }

  int32_t get_last_child() {
    if(m_symbols_size==0) return -1;
    return m_symbols[m_symbols_size-1].index;
  }

  bool equal(SuffixNode &other,bool dump=false) {
    if(get_parent()          != other.get_parent()     )    { if(dump)  cout << "parent match failure" << endl;          return false; }
    if(get_label_start()     != other.get_label_start())    { if(dump)  cout << "label_start match failure" << endl;     return false; }
    if(get_label_end()       != other.get_label_end()  )    { if(dump)  cout << "label_end match failure mine: " << get_label_end() << " other: " << other.get_label_end() << endl; return false; }
// if(m_children != 0)   if((m_children->equal(*(other.m_children),dump) == false))    { if(dump)  cout << "children match failure" << endl; return false; }

    if(m_symbols_size  != other.m_symbols_size) { if(dump)  cout << "children match failure" << endl; return false; }
    for(size_t n=0;n<m_symbols_size;n++) { if(m_symbols[n].index != other.m_symbols[n].index) {if(dump)  cout << "children match failure" << endl; return false; }    }
    for(size_t n=0;n<m_symbols_size;n++) { if(m_symbols[n].symbol != other.m_symbols[n].symbol) {if(dump)  cout << "children match failure" << endl; return false; }    }

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
    for(size_t n=0;n<m_symbols_size;n++) cout << (int) m_symbols[n].symbol << "," << m_symbols[n].index << " ";
    cout << endl;
  }

  const vector<SymbolPair> get_symbols() {
    vector<SymbolPair> symbols;

    for(size_t n=0;n<m_symbols_size;n++) symbols.push_back(symbols[n]);

    return symbols;
  }

  void set_symbols(const vector<SymbolPair> &s) {
    clear_children();

    for(size_t n=0;n<s.size();n++) set_child(s[n].symbol,s[n].index);
  }

  SuffixNode &operator=(const SuffixNode &other) {
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

  void wipe() {
    m_symbols =0;
    m_symbols_size=0;
  }

  int32_t get_label_end_translated() {
    if(get_label_end() == end_marker) {
      return end_marker_value;
    }
    return get_label_end();
  }

// accessors
public:
  int32_t get_depth_raw() const {
    return m_depth;
  }

  void set_depth_raw(int32_t depth_in) {
    m_depth = depth_in;
  }

  int32_t get_depth() const {
    if(get_label_start() == -1) return 0;
    if(get_label_end()   == -1) return m_depth + (end_marker_value-get_label_start())+1;
    return m_depth;
  }

  void set_depth(int32_t depth_in) {
    m_depth = depth_in;
  }

  int32_t get_parent() const {
    return m_parent;
  }

  void set_parent(int32_t parent_in) {
    m_parent = parent_in;
  }

  int32_t get_suffix_link() const {
    return m_suffix_link;
  }
  
  void set_suffix_link(int32_t suffix_link_in) {
    m_suffix_link = suffix_link_in;
  }

  int32_t get_label_start() const {
    return m_label_start;
  }

  void set_label_start(int32_t label_start_in) {
    m_label_start = label_start_in;
  }

  int32_t get_label_end() const {
    return m_label_end;
  }

  void set_label_end(int32_t label_end_in) {
    m_label_end = label_end_in;
  }

  int32_t get_next_left_leaf() const {
    return m_next_left_leaf;
  }

  void set_next_left_leaf(int32_t next_left_leaf_in) {
    m_next_left_leaf = next_left_leaf_in;
  }

  int32_t get_next_right_leaf() const {
    return m_next_right_leaf;
  }

  void set_next_right_leaf(int32_t next_right_leaf_in) {
    m_next_right_leaf = next_right_leaf_in;
  }

private:
  SymbolPair *m_symbols;
  unsigned char m_symbols_size;

  int32_t m_depth;
  int32_t m_parent;
  int32_t m_suffix_link;
  int32_t m_label_start;
  int32_t m_label_end  ;
  int32_t m_next_left_leaf;
  int32_t m_next_right_leaf;

public:
  static int32_t end_marker;
  static int32_t end_marker_value;
  static int32_t root;
} __attribute__((__packed__));

#endif
