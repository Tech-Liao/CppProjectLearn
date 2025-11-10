#pragma once
#include <cstddef>
#include <iterator> // 仅用标准标签
#include "type_traits.h"
#include<stdexcept>

namespace my
{
    template <class It>
    struct iterator_traits
    {
        using difference_type = typename It::difference_type;
        using value_type = typename It::value_type;
        using pointer = typename It::pointer;
        using reference = typename It::reference;
        using iterator_category = typename It::iterator_category;
    };
    template <class T>
    struct iterator_traits<T *>
    {
        using difference_type = std::ptrdiff_t; // #define __PTRDIFF_TYPE__ long int
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::random_access_iterator_tag; //  源于struct input_iterator_tag { };
    };
    template <class T>
    struct iterator_traits<const T *>
    {
        using difference_type = std::ptrdiff_t; // #define __PTRDIFF_TYPE__ long int
        using value_type = T;
        using pointer = const T *;
        using reference = const T &;
        using iterator_category = std::random_access_iterator_tag; //  源于struct input_iterator_tag { };
    };

    // 判断“是否至少是双向迭代器"
    template <class Cat>
    struct is_bidirectional_or_better : std::is_base_of<std::bidirectional_iterator_tag, Cat>
    {
    };

    // ---distance----
    template <class It>
    inline typename iterator_traits<It>::difference_type
    distance_impl(It first, It last, std::input_iterator_tag)
    {
        // 计算容器距离
        typename iterator_traits<It>::difference_type n = 0;
        for (; first != last; ++first)
            ++n;
        return n;
    }
    template <class It>
    inline typename iterator_traits<It>::difference_type
    distance_impl(It first, It last, std::random_access_iterator_tag)
    {
        // 计算容器距离
        return last - first;
    }
    template <class It>
    inline typename iterator_traits<It>::difference_type
    distance(It first, It last)
    {
        // 计算容器距离,通过传递不同的iterator类型
        using category = typename iterator_traits<It>::iterator_category;
        return distance_impl(first, last, category{});
    }
    // --------advance------
    template <class It, class Dist>
    inline void advance_impl(It &it, Dist n, std::input_iterator_tag)
    {
        if (n < 0)
        {
            throw std::logic_error("advance:negative step for input/forward iterator");
        }
        // 只支持正步进
        for (; n > 0; --n)
            ++it;
    }
    template <class It, class Dist>
    inline void advance_impl(It &it, Dist n, std::random_access_iterator_tag)
    {
        it += n;
    }
    template <class It, class Dist>
    inline void advance_impl(It &it, Dist n, std::bidirectional_iterator_tag)
    {
        if (n >= 0)
        {
            while (n--)
                ++it;
        }
        else
        {
            while (n++)
                --it;
        }
    }
    template <class It>
    inline void advance(It &it, typename iterator_traits<It>::difference_type n)
    {
        using category = typename iterator_traits<It>::iterator_category;
        advance_impl(it, n, category{});
    }
    // ---------------- advance（编译期常量版） ----------------
    // 当你把 n 作为 integral_constant 传入时，可在编译期阻止负步长用于非双向迭代器。
    template <class It, class Dist, Dist N>
    inline void advance(It &it, std::integral_constant<Dist, N> /*tag*/)
    {
        using category = typename iterator_traits<It>::iterator_category;
        static_assert( (N >= 0) || is_bidirectional_or_better<category>::value,
                      "negative step requires bidirectional iterator");
        advance(it, static_cast<Dist>(N));
    }

    // -------- next/prev（基于 advance）--------
    template <class It>
    inline It next(It it, typename iterator_traits<It>::difference_type n = 1)
    {
        advance(it, n);
        return it;
    }
    template <class It>
    inline It prev(It it, typename iterator_traits<It>::difference_type n = 1)
    {
        advance(it, -n);
        return it;
    }

    template <class It>
    inline typename iterator_traits<It>::difference_type
    range_size(It first, It last)
    {
        return distance(first, last);
    }
}