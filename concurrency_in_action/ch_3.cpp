#include <algorithm>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
/* Chapter covers
  Problem sharing data b/w thread and fixes */

// 1. Invariants:  Statements that are always true about a particular data
// structure
// 2. Race condition: If the outcome depends on the relative ordering of two
// reads running in parallel
// 3. When race condition breaks invariants then it is a problem
// 4. Data race: Race condition that arises to due concurrent modifications to a
// single object
// 5. lock-free programming: Design data structures such that invariants
// modifications codes as a sereis of indivisible changes.
// 6. Dealing with race conditions:
//    1. Protection mechanism to allow one thread to modify data
//    2. Lock free programming
//    3. Softawre Transactional Memory: Transaction based updates, the
//    intermediate steps are recorded and executed altogether, if any issue
//    occurs it is rolleback.
//    4. Using mutex (mutually exclusive)
//
// 7. To work with mutex, we need to ensure the following
//    1. The functions don't return a pointers/references to the protected data
//    2. The function don't pass  pointers/references into another functions. If
//    the function to execute is provided at runtime, this'd be impossible to
//    check.
//    3. Mutexes won't avoid race conditions unless you protect the appropriate
//    data.
//    4. Making each individual module won't avoid race conditions, for eg. if a
//    code triggers some locs if the data strcuture is empty by using .empty()
//    call, it is possible that other threads have modified the ds in between
//    the empty call and running lines of code.

class Node {
  int val;
  Node *prev, *next;
  Node() : val(0), prev(nullptr), next(nullptr) {}
};

class DLL {
  Node *head;
  DLL() : head(nullptr) {}
  // void  /
};

class lis {

  std::list<int> l;
  std::mutex m;

public:
  void add_to_list(int x) {
    std::lock_guard<std::mutex> guard(m);
    l.push_back(x);
  }

  bool contains(int x) {
    std::lock_guard<std::mutex> guard(m);
    return std::find(l.begin(), l.end(), x) != l.end();
  }
};

int main() { return 0; }
