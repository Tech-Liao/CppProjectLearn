#pragma once

#include "type_traits.h"
#include "utility.h"

namespace my {
template <class T, class... Args>
inline void construct(T *p, Args &&... args) {
    new (static_cast<void *>(p)) T(my::forward<Args>(args)...);
}

template <class T>
inline void destory(T *p) {
    p->~T();
}

template <class It>
inline void destory(It first, It last) {
    for (; first != last; ++first) my::destory(&(*first));
}

}  // end namespace my