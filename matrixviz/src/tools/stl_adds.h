#ifndef __STL_ADDS_H
#define __STL_ADDS_H

#ifndef AVR

#include<list>
#include<string>

#define FOREACH(colltype, coll, it) for( colltype::iterator it = (coll).begin(); it!= (coll).end(); ++it)
#define FOREACHC(colltype, coll, it) for( colltype::const_iterator it = (coll).begin(); it!= (coll).end() ; ++it )


namespace std {

  /// absolute function for all types
  template<typename T>
  inline T abs(T v)
  { return ((v>0)?v:-v); }

  /// += operators for list (list concat)
  template <class T>
  inline list<T>& operator+=(list<T>& l1, const list<T>& l2) 
  { l1.insert(l1.end(), l2.begin(), l2.end()); return l1;}

  /// += operators for list (append)
  template <class T>
  inline list<T>& operator+=(list<T>& l1, const T& v) 
  { l1.push_back(v); return l1;}

  string itos(int i);

}

#else

#include "avrtypes.h"

namespace std {

  /// absolute function for all types
  template<typename T>
  inline T abs(T v)
  { return ((v>0)?v:-v); }

  charArray itos(int i);

}

#endif

#endif
