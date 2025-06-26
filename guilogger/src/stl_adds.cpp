
#include "stl_adds.h"
#include <cstdio>

namespace std {
  
  string itos(int i){
    char str[10];
    snprintf(str, sizeof(str),"%i", i);
    return string(str);
  }

}
