#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "UnitTest.h"
#include "global_defs.h"
#include "test_suffixnode.h"
#include "test_suffixtree_simple.h"
#include "test_suffixtree_random.h"
#include "test_suffixtree_time.h"
#include "test_suffixtree_processpositions.h"
#include "test_suffixtree_alloccurs.h"
#include "test_suffixtree_lrs.h"

using namespace std;

int main(int argc,char ** argv) {
  UnitTest utf("41J Tests");

  test_suffixnode(utf);
  test_suffixtree_simple(utf);
  test_suffixtree_lrs(utf);
  test_suffixtree_processpositions(utf);
  test_suffixtree_alloccurs(utf);
  test_suffixtree_random(utf);
 // test_suffixtree_time(utf);
  utf.test_report();
}
