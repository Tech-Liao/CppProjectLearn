#include <iostream>
#include <iterator>

namespace my {
template <typename T, typename category = std::random_access_iterator_tag>
class iterator {
private:
    T* ptr;

public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::ptrdiff_t difference_type;
    typedef category iterator_category;

    iterator(T* p) : ptr(p) {}
    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    iterator& operator++() {
        ++ptr;
        return *this;
    }
    iterator operator++(int) {
        iterator temp = *this;
        ++ptr;
        return temp;
    }
    iterator& operator--() {
        --ptr;
        return *this;
    }
    iterator operator--(int) {
        iterator temp = *this;
        --ptr;
        return temp;
    }
    bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    bool operator==(const iterator& other) const { return ptr == other.ptr; }
    iterator operator+(difference_type n) const { return iterator(ptr + n); }
    iterator operator-(difference_type n) const { return iterator(ptr - n); }

    iterator& operator+=(difference_type n) {
        ptr += n;
        return *this;
    }
    iterator& operator-=(difference_type n) {
        ptr -= n;
        return *this;
    }
    difference_type operator-(const iterator& other) const {
        return ptr - other.ptr;  // 返回迭代器之间的距离（即元素个数）
    }
    reference operator[](difference_type n) { return *(ptr + n); }
    // 比较操作符：小于、大于等
    bool operator<(const iterator& other) const { return ptr < other.ptr; }

    bool operator>(const iterator& other) const { return ptr > other.ptr; }

    bool operator<=(const iterator& other) const { return ptr <= other.ptr; }

    bool operator>=(const iterator& other) const { return ptr >= other.ptr; }
};  // end class iterator
}  // end namespace my