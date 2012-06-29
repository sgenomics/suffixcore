#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "SuffixTree.h"
#include "SuffixNodeStoreMemVec.h"
#include "ProcessPositions.h"
#include "UnitTest.h"
#include <algorithm>

using namespace std;

void test_suffixtree_lrs(UnitTest &utf) {

  utf.begin_test_set("LRS SuffixTree tests");

  {
    string ms = "banana";
    SuffixTree s1;

    for(size_t n=0;n<ms.size();n++) {
      s1.insert(ms[n]);
    }
    s1.finalise();

    vector<symbol_type> res = s1.lrs();

    utf.test_equality(res[0],(symbol_type)'a');
    utf.test_equality(res[1],(symbol_type)'n');
    utf.test_equality(res[2],(symbol_type)'a');
  }

  {
    string ms = "the quick brown fox jumped over the quick brown dog.";
    SuffixTree s1;

    for(size_t n=0;n<ms.size();n++) {
      s1.insert(ms[n]);
    }
    s1.finalise();

    vector<symbol_type> res = s1.lrs();

    utf.test_equality(res.size(),(size_t)16);
    utf.test_equality(res[ 0],(symbol_type)'t');
    utf.test_equality(res[ 1],(symbol_type)'h');
    utf.test_equality(res[ 2],(symbol_type)'e');
    utf.test_equality(res[ 3],(symbol_type)' ');
    utf.test_equality(res[ 4],(symbol_type)'q');
    utf.test_equality(res[ 5],(symbol_type)'u');
    utf.test_equality(res[ 6],(symbol_type)'i');
    utf.test_equality(res[ 7],(symbol_type)'c');
    utf.test_equality(res[ 8],(symbol_type)'k');
    utf.test_equality(res[ 9],(symbol_type)' ');
    utf.test_equality(res[10],(symbol_type)'b');
    utf.test_equality(res[11],(symbol_type)'r');
    utf.test_equality(res[12],(symbol_type)'o');
    utf.test_equality(res[13],(symbol_type)'w');
    utf.test_equality(res[14],(symbol_type)'n');
    utf.test_equality(res[15],(symbol_type)' ');
  }

  utf.end_test_set();
}
