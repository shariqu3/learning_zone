#include <iostream>
#include <numeric>
#include <thread>
#include <typeinfo>
#include <vector>

// 1. Passing vexing parse object initialisers as arguments to threads.
// 2. A thread can be joined to synchronise with main thread or detached.
// 3. On running detach, the process exits when the end of main function is
// reached (std::exit is called at the end)
// 4. On running join, you need to make sure where join is called at code and
// join is called at all possible paths including execeptions.
// 5. For join, can do RAII (Resource Acquisition Is Initialization) and define
// a destructor that does join.
// 6. rvalues are temporary values that does not have a persistent identity in
// the program. For eg. 3+5;
// 7. lvalues are opposite of rvalues. Eg. x=3+5. x is an lvalue where as 3+5
// is an rvalue.
// 8. There are 6 special member functions of a class
//    a. Default constructor  C::C();
//    b. Destructor           C::~C();
//    c. Copy constructor     C::C(const C&);
//        Called when
//          - passing the object by value
//          - C x = y;   where y is of type C
//    d. Copy assignment      C& operator=(const C&) | C& operator=(const C)
//        Called when already initalised object is written over
//          C a, b;
//          a = b;   -> Copy assignment
//    e. Move constructor     C::C(C&&);
//    f. Move assignment      C& operator=(C&&);
//  9. Doulble ampersand(&&) allows for rvalue references.
//  10. A temporary variable dies at the end of the full expression.
//  11. A temporary may never reach memory, can end up being copied from
//  register.
//  12. In case of unhandled exception in a thread, std::exception is call in
//  that thread and then std::abort in the whole process. If one thread fails
//  the whole process is stopped.
//  13. Data passed as arguments to std::thread function calls are first copied
//  to internal storage of the thread then passed as either lvalue or rvalue to
//  the requesting function. This happens even when the call is by reference.
//  Use std::ref to fix this.
//  14. Typecasting of provided argument also happens in thread's context.
//  15. const on parameter argument ensures that the variable is not modified in
//  the code body.
//  16. Use std::thread::hardware_concurrency(); to get the no. of threads
//  supported in the current system. Which can return 0 in somes cases it
//  doesn't know.
//  17. Run n-1 threads, keep in mind the current main function captures one
//  thread.
//  18. Use vector and accumulate to create sort of a parallel region.
//  19. Transferring ownership of thread is tricky, you'd want to know if the
//  thread is joinable, and a perfect move will require deleting old variable as
//  only on object can hold ownership of an std::thread.
//  20. Identifier of current thread can be achieved using get_id() on
//  std::thread or by calling std::this_thread::get_id().
//  TODO:
//  1. Perfect forwarding: rvalue passed as an argument becomes an lvalue
//  inside the function this results in loss of performance. Perfect forwarding
//  means preserving the original category (lvalue or rvalue) of an argument on
//  passing.
//  2.std::move
//  3.std::bind
//  4.std::ref
//  5.std::invoke
//  6.std::unique
//  7.argument decay
//  8.guaranteed copy elision (RVO)
//  9. how does return works
//  10. explicit
//  11. std::logic-error
//  12. noexcept
//  13. std::forward
//  14. std::vector is move aware. Wdym?
//  15. std::accumulate
//  16. std::reduce
//  17. Iterators and their types

class C {
  int x;
  int *ptr;

public:
  int &content() const { return *ptr; }
  C() { x = 0; }                 // constructor
  ~C() { delete ptr; }           // destructor
  C(const C &) : ptr(new int) {} // copy constructor
  C &operator=(const C t) {
    delete ptr;
    ptr = new int(t.content());
    return *this;
  } // copy assignment by value
  C &operator=(const C &) { return *this; } // copy assignment by reference
  C(C &&) {}                                // move constructor
  // C& opearator=(C&&){} // move assignment
};

int func2(const int x) { return x * 2; }

struct func {
  int &i;
  func(int &i_) : i(i_) {}
  void operator()() {
    for (int j = 0; j < 100; j++)
      i++;
    std::cout << i << '\n';
  }
  ~func() { std::cout << "Destructor called\n"; }
};

// A functor
class background_task {
public:
  background_task() { std::cout << "Constructed\n"; }
  ~background_task() { std::cout << "Destructed\n"; }
  void operator()() const { std::cout << "Invoked\n"; }
};

void f() {
  int a = 0;
  func abc(a);
  std::thread t(abc);
  try {
  } catch (...) {
    t.join();
    throw;
  }
  t.join();
}

class thread_guard_old {
  std::thread &t;

public:
  explicit thread_guard_old(std::thread &_t) : t(_t) {}
  ~thread_guard_old() {
    if (t.joinable()) {
      t.join();
    }
  }
  // copy
  thread_guard_old(thread_guard_old const &) = delete;
  // assignment
  thread_guard_old &operator=(thread_guard_old const &) = delete;
};

class scope_thread {
  std::thread t;

public:
  explicit scope_thread(std::thread _t) : t(std::move(_t)) {
    if (!t.joinable())
      throw std::logic_error("No thread");
  }
  scope_thread(scope_thread const &) = delete;
  scope_thread &operator=(scope_thread const &) = delete;
  ~scope_thread() { t.join(); }
};

int mul_2(int y) { return y * 2; }

class A {
public:
  int a;
  A() : a(3) { std::cout << "Created\n"; }
  void mux(int b) { a *= b; }
};

template <typename Iterator, typename T> struct accumulate_block {
  void operator()(Iterator first, Iterator last, T &result) {
    result = std::accumulate(first, last, result);
  }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
  unsigned long const length = std::distance(first, last);
  if (!length)
    return init;
  unsigned long const min_per_thread = 25;
  unsigned long const max_threads =
      (length + min_per_thread - 1) / min_per_thread;
  unsigned long const hardware_threads = std::thread::hardware_concurrency();
  unsigned long const num_threads =
      std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
  unsigned long const block_size = length / num_threads;
  std::vector<T> results(num_threads);
  std::vector<std::thread> threads(num_threads - 1);
  Iterator block_start = first;
  for (unsigned long i = 0; i < (num_threads - 1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start,
                             block_end, std::ref(results[i]));
    block_start = block_end;
  }
  accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);

  for (auto &entry : threads)
    entry.join();
  return std::accumulate(results.begin(), results.end(), init);
}

void proc_atomic(std::vector<int>::iterator start,
                 std::vector<int>::iterator end, std::vector<int> &in,
                 std::vector<int> &out) {
  for (auto itr = start; itr != end; itr++) {
  }
}

std::vector<int> proc(std::vector<int> list) {
  int n_threads = std::thread::hardware_concurrency();
  int n = list.size();
  int min_threads = 10;
  int threads = std::min(n_threads, (n + min_threads - 1) / min_threads);
  std::vector<int> result(n);
  std::vector<std::thread> thread_pool(threads - 1);

  auto itr = list.begin();
  auto end_itr = list.end();
  int block_size = n / threads;

  for (int i = 0; i < threads - 1; i++) {
    auto start = itr;
  }
  for (auto &i : thread_pool)
    if (i.joinable())
      i.join();
  return result;
}

int main() {
  // int x = 0;
  // std::cout << func2(x);
  // func obj(x);
  // std::thread t1((func(x)));
  // std::cout << t1.joinable();
  // t1.join();
  // f();
  // func *a = new func(x);
  // a->~func(); operator delete(a);
  // A b;
  // std::thread t(mul_2, 10);
  // std::thread g(&A::mux, b, 3);
  std::thread::id main_id = std::this_thread::get_id();
  std::cout << main_id << '\n';
  std::cout << std::thread::hardware_concurrency();
  return 0;
}
