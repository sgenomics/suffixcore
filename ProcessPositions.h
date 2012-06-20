#ifndef PROCESSPOSITIONS
#define PROCESSPOSITIONS

#include "global_defs.h"

class positions {
public:

  positions() {}

  positions(index_type l,index_type r) : next_left(l), next_right(r) {}

  index_type next_left;
  index_type next_right;
};

template<class snstore_type,class sn_t>
class ProcessPositions {

public:

  snstore_type &m_store;
  positions_store m_pstore;
  
  ProcessPositions(snstore_type &s, positions_store &pp) : m_store(s), m_pstore(pp) {}

  ProcessPositions(snstore_type &s) :m_store(s),m_pstore(s.size()+1,positions(invalid_idx,invalid_idx)) {
  }

  template<class other_type>
  void copyto(other_type &o) {
    for(size_t n=0;n<m_pstore.size();n++) {
      o.push_back(m_pstore[n]);
    }
  }


  void label_all_left_most_down(vector<index_type> &label_these,index_type label) {
    for(size_t n=0;n<label_these.size();n++) {
      m_pstore[label_these[n]].next_left = label;
    }
  }

  bool first_is_leaf(SuffixNode &n) {
    index_type i = n.get_first_child();
    if(i == invalid_idx) return false;

    sn_t na = m_store.get(i);
    return na.is_leaf();
  }

  bool last_is_leaf(SuffixNode &n) {
    index_type i = n.get_last_child();
    if(i == invalid_idx) return false;

    sn_t na = m_store.get(i);
    return na.is_leaf();
  }

  index_type get_next_left_leaf(index_type i) { return m_pstore[i].next_left; }
  index_type get_next_right_leaf(index_type i) { return m_pstore[i].next_right; }

  void process_positions() {
    index_type c    = SuffixNode::root;  // start at root vertex.
    index_type last = invalid_idx;

    vector<index_type> unlabeled_left;

    index_type root_node = SuffixNode::root;

    index_type current_right_most = SuffixNode::root;
    index_type last_right         = SuffixNode::root;
    uint64_t visit_count=0;
    for(;;) {
      sn_t c_node = m_store.get(c);
      visit_count++;
      //cout << "c: " << c << endl;
      if(visit_count%10000 == 0) cout << "visit_count: " << visit_count << endl;
      // labeling code

      // left labeling.
      if(m_pstore[c].next_left == invalid_idx) unlabeled_left.push_back(c);

      if(first_is_leaf(c_node)) {
	label_all_left_most_down(unlabeled_left,c_node.get_first_child());
	unlabeled_left.clear();
      }

      // right labeling.
      m_pstore[c].next_right = current_right_most;

      if(last_is_leaf(c_node)) {
	current_right_most = c_node.get_last_child();
      }

      // next_right labeling.
      if(c_node.is_leaf()) {
	m_pstore[last_right].next_right = c;
	last_right = c;
      }

      // walking code
      if(last == invalid_idx) {

	last = c;
	index_type tc = c_node.get_first_child();
        //cout << "first child: " << tc << endl;
	if(tc != invalid_idx) {c = tc;} else {
	  if(c == root_node) return;
	  if(tc==invalid_idx) c = c_node.get_parent();
	   if(c==invalid_idx) {return;}
	}
      } else {
	if(c_node.is_child(last)) {
	  index_type tc = c_node.next_child(last);
          //cout << "next child: " << tc << endl;
	  last = invalid_idx;
	  if(tc!=invalid_idx) { c = tc; }
		   else       { if(c==root_node) return; last = c; c = c_node.get_parent(); if(c == invalid_idx) {return;} }
	} else {

	  last=c;
	  index_type tc = c_node.get_first_child();
          //cout << "first child: " << tc << endl;
	  if(tc!=invalid_idx) {c = tc;}
	  else {
	     if(c == root_node) return;
	     c = c_node.get_parent();
             //cout << "parent: " << c << endl;
	     if(c==invalid_idx) return;
	  }
	}
      }
    }
  }
};

#endif
