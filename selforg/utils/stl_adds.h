#ifndef STL_ADDS_H
#define STL_ADDS_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <string>
#include <vector>

// iterators for stl containers. Do not use for removal because the end is determined at the
// beginning.
//  use C++11 syntax now: for( auto &val : coll)
#define FOREACH(colltype, coll, it)                                                                \
  for (colltype::iterator it = (coll).begin(), __end = (coll).end(); it != __end; ++it)

// Iteration with index
// unfortunatelly we cannot initialize the index within the for loop (different type than iterator)
#define FOREACHI(colltype, coll, it, index)                                                        \
  int index = 0;                                                                                   \
  for (colltype::iterator it = (coll).begin(), __end = (coll).end(); it != __end; ++it, ++index)
// using C++11 auto typing!
#define FOREACHIa(coll, it, index)                                                                 \
  int index = 0;                                                                                   \
  for (auto it = (coll).begin(), __end = (coll).end(); it != __end; ++it, ++index)

#define FOREACHC(colltype, coll, it)                                                               \
  for (colltype::const_iterator it = (coll).begin(), __end = (coll).end(); it != __end; ++it)
#define FOREACHCI(colltype, coll, it, index)                                                       \
  int index = 0;                                                                                   \
  for (colltype::const_iterator it = (coll).begin(), __end = (coll).end(); it != __end;            \
       ++it, ++index)

// using C++11 auto typing!
#define FOREACH2(coll1, coll2, it1, it2)                                                           \
  for (auto it1 = (coll1).begin(),                                                                 \
            __end1 = (coll1).end(),                                                                \
            it2 = (coll2).begin(),                                                                 \
            __end2 = (coll2).end();                                                                \
       it1 != __end1 && it2 != __end2;                                                             \
       ++it1, ++it2)

/// contains some additions to the standard template library
namespace std {

/// absolute function for all types
template<typename T>
inline T
abs(T v) {
  return ((v > 0) ? v : -v);
}

/// += operators for list (list concat)
template<class T, class A>
list<T, A>&
operator+=(list<T, A>& l1, const list<T, A>& l2) {
  l1.insert(l1.end(), l2.begin(), l2.end());
  return l1;
}

/// += operators for list (append element)
template<class T, class A>
list<T, A>&
operator+=(list<T, A>& l1, const T& v) {
  l1.push_back(v);
  return l1;
}

/// + operators for lists (list concat)
template<class T, class A>
list<T, A>
operator+(const list<T, A>& l1, const list<T, A>& l2) {
  list<T, A> rv(l1.begin(), l1.end());
  rv += l2;
  return rv;
}

// These initializer are obsolete with C11 use list initializers {a1,a2,...}
// returns a list with a single element
//  template <typename T>
// std::list<T> _1tolist(T a){ std::list<T> l; l.push_back(a); return l; }
// ...

/// integer to string with default formating
string itos(int i);
/// integer to string with printf formating string
string itos(int i, const char*);
/// integer to string with default formating
string ftos(double i);
/// integer to string with printf formating string
string ftos(double i, const char*);

template<typename Col, typename T>
bool
removeElement(Col& col, const T& elem) {
  // search for element
  typename Col::iterator i = find(col.begin(), col.end(), elem);
  if (i != col.end()) {
    col.erase(i);
    return true;
  } else {
    return false;
  }
}

template<class T>
struct join {
  using argument_type = T;
  using result_type = void;
  join(const T& delimit_) : delimit(delimit_) {}
  void operator()(const T& s) {
    if (count == 0) {
      joined = s;
    } else {
      joined += delimit + s;
    }
    ++count;
  }
  T delimit;
  T joined;
  int count{ 0 };
};

template<typename A, typename E>
A
reduceList(const std::list<E>& list, A acc, std::function<A(A, E)> f) {
  for (auto i : list) {
    acc = f(acc, i);
  }
  return acc;
}

template<typename O, typename I>
std::list<O>
mapList(const std::list<I>& l, std::function<O(const I&)> fun) {
  std::list<O> res;
  std::transform(l.begin(), l.end(), std::back_inserter(res), fun);
  return res;
}

} // namespace std

#endif
