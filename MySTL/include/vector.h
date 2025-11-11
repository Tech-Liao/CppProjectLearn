#pragma once

#include <iostream>
#include <stdexcept>

#include "iterator.h"
namespace my {
template <class T>
class vector {
private:
    T* data;
    size_t capacity;
    size_t size;

public:
    vector() : data(nullptr), capacity(0), size(0) {}
    ~vector() { delete[] data; }
    vector(const vector& other)
        : data(new T[other.capacity]),
          capacity(other.capacity),
          size(other.size) {
        for (size_t i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }
    vector& operator=(const vector& other) {
        if (this != other) {
            delete[] data;
            capacity = other.capacity;
            size = other.size;
            data = new T[capacity];
            for (size_t i; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }

    void push_back(const T& value) {
        if (size == capacity) {
            capacity = (capacity == 0 ? 2 : capacity * 2);
            T* newdata = new T[capacity];
            for (size_t i = 0; i < size; ++i) newdata[i] = data[i];
            delete[] data;
            data = newdata;
        }
        data[size++] = value;
    }

    void pop_back() {
        if (size > 0) --size;
    }
    void resize(size_t newSize) {
        if (newSize > capacity) {
            capacity = newSize;
            T* newdata = new T[capacity];
            for (size_t i = 0; i < size; ++i) {
                newdata[i] = data[i];
            }
            delete[] data;
            data = newdata;
        }
        size = newSize;
    }

    T& operator[](size_t index) { return data[index]; }
    size_t get_size() const { return size; }
    size_t get_capacity() const { return capacity; }
    bool empty() const { return size == 0; }

public:
    //迭代器接口
    iterator<T> begin() { return iterator<T>(data); }
    iterator<T> end() { return iterator<T>(data + size); }
};

}  // namespace my