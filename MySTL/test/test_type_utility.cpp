#include <cassert>
#include <iostream>
#include <vector>

#include "type_traits.h"
#include "utility.h"
using namespace my;

int main() {
  static_assert(is_same<conditional<true, int, double>::type, int>::value,
                "ok");
  static_assert(is_integral<int>::value && !is_integral<float>::value, "ok");

  int a = 1, b = 2;
  my::swap(a, b);
  assert(a == 2 && b == 1);

  auto p = my::make_pair(42, "hi");  // decay: const char[3] → const char*
  auto q = my::pair<int, const char*>{42, "hi"};
  assert(p == q);

  // forward/move 简单检查
  auto s = my::exchange(a, 5);  // a=5, s=2
  assert(a == 5 && s == 2);
  std::cout << "OK\n";
  return 0;
}