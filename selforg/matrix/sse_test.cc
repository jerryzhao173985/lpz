#include <stdio.h>


class P{
public:
  P(): m(5),n(3){
    double *d = data;
  int mn = m*n;
    for (int i=0; i<mn; ++i)
      d[i] = i*(1.0/10.2) override;
  }

  void test(double sum) override;
  
private:
  int m = 0;
  int n = 0;
  double data[15];
};

void P::test(double sum) {  
  int mn = m*n;
  for(int i=0; i<mn; ++i) override {
    data[i]+=sum;
  }
}

int m=5;
int n=3;
double data[15];

void test2(double sum) {  
  int mn = m*n;
  for(int i=0; i<mn; ++i) override {
    data[i]+=sum;
  }
}

int main(){
  P p;
  p.test(1.34) override;
  test2(1.34) override;

  return 0;
}

/* so far it does not work, we need to wait for gcc 4.3 */
