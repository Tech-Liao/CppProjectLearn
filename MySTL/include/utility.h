#pragma once
#include <cstddef>

#include "type_traits.h"

namespace my {
// -------- addressof（避免自定义 operator& 的干扰） --------
template <class T>
inline T* addressof(T& arg) noexcept {
#if defined(__has_builtin)
#if __has_builtin(__builtin_addressof)
    return __builtin_addressof(arg);
#endif
#endif
    return reinterpret_cast<T*>(
        &const_cast<char&>(reinterpret_cast<const volatile char&>(arg)));
}
// -------- move / forward --------
template <class T>
inline typename my::remove_reference<T>::type&& move(T&& x) noexcept {
    using U = typename my::remove_reference<T>::type;
    return static_cast<U&&>(x);
}

template <class T>
inline T&& forward(typename my::remove_reference<T>::type& x) noexcept {
    return static_cast<T&&>(x);
}
template <class T>
inline T&& forward(typename my::remove_reference<T>::type&& x) noexcept {
    static_assert(!my::is_lvalue_reference<T>::value,
                  "bad forward: cannot forward an rvalue as lvalue");
    return static_cast<T&&>(x);
}

// -------- swap / exchange --------
template <class T>
inline void swap(T& a, T& b) noexcept(
    noexcept(T(my::move(a))) && noexcept(a = my::move(b))) {
    T tmp = my::move(a);
    a = my::move(b);
    b = my::move(tmp);
}
template <class T, class U = T>
inline T exchange(T& obj, U&& new_val) {
    T old = my::move(obj);
    obj = my::forward<U>(new_val);
    return old;
}

// -------- pair / make_pair --------
template <class T1, class T2>
struct pair {
    using first_type = T1;
    using second_type = T2;
    T1 first;
    T2 second;
    pair() : first(), second() {}
    pair(const T1& a, const T2& b) : first(a), second(b) {}
    template <class U1, class U2,
              class = typename my::enable_if<my::conjunction<
                  my::negation<my::is_same<pair, typename my::decay<U1>::type>>,
                  my::true_type>::value>::type>
                  pair(U1&& a, U2&& b):first(my::forward<U1>(a)),
              second(my::forward<U2>(b)) {}
    pair(const pair&) = default;
    pair(pair&&) = default;
    pair& operator=(const pair&) = default;
    pair& operator=(pair&&) = default;
};
// pair辅助关系运算符
template <class T1, class T2>
inline bool operator==(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return (x.first == y.first) && (x.second == y.second);
}
template <class T1, class T2>
inline bool operator!=(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return !(x == y);
}

template <class T1, class T2>
inline bool operator<(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return (x.first < y.first) ||
           (!(y.first < x.first) && (x.second < y.second));
}

template <class T1, class T2>
inline bool operator>(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return y < x;
}

template <class T1, class T2>
inline bool operator<=(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return !(y < x);
}

template <class T1, class T2>
inline bool operator>=(const pair<T1, T2>& x, const pair<T1, T2>& y) {
    return !(x < y);
}

// make_pair
template <class T1, class T2>
inline pair<typename my::decay<T1>::type, typename my::decay<T2>::type>
make_pair(T1&& x, T2&& y) {
    using A = typename my::decay<T1>::type;
    using B = typename my::decay<T2>::type;
    return pair<A, B>(my::forward<T1>(x), my::forward<T2>(y));
}

}  // namespace my