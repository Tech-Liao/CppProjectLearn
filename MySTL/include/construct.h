#pragma once

#include <new>
#include <type_traits>

namespace my {
template <class T1, class T2>
inline void construct(T1 *p, const T2 &value) {
    //
}

template <class T>
inline void destroy(T *pointer) {
    //
}

template <class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last) {
    ///
}
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T *) {
    ////
}
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last,
                          __false_type) {
    ////
}

template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last,
                          __true_type) {
    ///
}

inline void destroy(char *, char *);
inline void destroy(wchar_t *, wchar_t *);

}  // end namespace my