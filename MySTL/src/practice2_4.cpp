// 编写一个程序，实现容器间的转换，例如 `vector` 转 `list`，`queue` 转 stack
#include <iostream>
#include <list>
#include <queue>
#include <stack>
#include <vector>

void vector2list(const std::vector<int> &vec, std::list<int> &lst) {
    lst.assign(vec.begin(), vec.end());
}
void list2vector(const std::list<int> &lst, std::vector<int> &vec) {
    vec.assign(lst.begin(), lst.end());
}
void queue2stack(std::queue<int> &q, std::stack<int> &s) {
    while (!q.empty()) {
        s.push(q.front());
        q.pop();
    }
}
void stack2queue(std::stack<int> &s, std::queue<int> &q) {
    while (!s.empty()) {
        q.push(s.top());
        s.pop();
    }
}
int main() {
    // 测试 vector 和 list 之间的转换
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::list<int> lst;
    vector2list(vec, lst);
    std::cout << "List contents: ";
    for (const auto &num : lst) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    std::vector<int> vec2;
    list2vector(lst, vec2);
    std::cout << "Vector contents: ";
    for (const auto &num : vec2) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    // 测试 queue 和 stack 之间的转换
    std::queue<int> q;
    for (int i = 1; i <= 5; ++i) {
        q.push(i);
    }
    std::stack<int> s;
    queue2stack(q, s);
    std::cout << "Stack contents (from top to bottom): ";
    while (!s.empty()) {
        std::cout << s.top() << " ";
        s.pop();
    }
    std::cout << std::endl;
    std::stack<int> s2;
    for (int i = 1; i <= 5; ++i) {
        s2.push(i);
    }
    std::queue<int> q2;
    stack2queue(s2, q2);
    std::cout << "Queue contents: ";
    while (!q2.empty()) {
        std::cout << q2.front() << " ";
        q2.pop();
    }
    std::cout << std::endl;
    return 0;
}