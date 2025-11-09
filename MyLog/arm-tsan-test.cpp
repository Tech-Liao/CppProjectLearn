#include <thread>
#include <iostream>
volatile int x = 0;
void f() { x++; }  // 故意制造竞争
int main() {
    std::thread t1(f), t2(f);
    t1.join(); t2.join();
    std::cout << "Done: " << x << std::endl;
    return 0;
}
