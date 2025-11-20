#include <iostream>
#include <unordered_map>

int find(std::unordered_map<int, double> &map, int key) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    } else {
        map[key] = 0;
        return 0;
    }
}

int main() {
    std::unordered_map<int, double> studentscore;
    studentscore[1] = 90;
    studentscore[2] = 91;
    studentscore[3] = 92;
    studentscore[4] = 93;
    int score = find(studentscore, 4);
    std::cout << score << std::endl;
    return 0;
}