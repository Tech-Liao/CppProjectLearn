#include<iostream>
#include<cassert>
#include<vector>
#include"Iterator_Traits.h"
#include"Reverse_iterator.h"

int main(){
  int a[]{1,2,3,4,5};
  auto n1 = my::distance(a+0, a+5);     // 5
  assert(n1==5);

  auto it = a+0;
  my::advance(it, 3);
  assert(*it==4);

  auto rbeg =  my::Reverse_Iterator<int*>(a+5);
  auto rend =  my::Reverse_Iterator<int*>(a+0);
  std::vector<int> v;
  for (auto r=rbeg; r!=rend; ++r) v.push_back(*r);
  // v: 5 4 3 2 1
  assert(v.front()==5 && v.back()==1);

  std::cout << "OK\n";
}