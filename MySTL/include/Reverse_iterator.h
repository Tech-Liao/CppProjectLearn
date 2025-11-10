#pragma once
#include <iterator>
#include<type_traits>
#include<memory>
#include "Iterator_Traits.h"

namespace my
{
    template <class It>
    class reverse_iterator
    {
    public:
        using iterator_type = It;
        using traits = my::iterator_traits<It>;
        using difference_type = typename traits::difference_type;
        using value_type = typename traits::value_type;
        using reference = typename traits::reference;
        using pointer = typename traits::pointer;
        using iterator_category = typename traits::iterator_category;

        static_assert(std::is_base_of < std::bidirectional_iterator_tag,iterator_category>::value,
                      "reverse_iterator requires at least bidirectional iterator");
        reverse_iterator() : cur_() {}
        explicit reverse_iterator(It it) : cur_(it) {}
        template <class U>
        reverse_iterator(const reverse_iterator<U> &other) : cur_(other.base()) {}
        It base() const { return cur_; }
        reference operator*() const
        {
            It tmp = cur_;
            --tmp;
            return *tmp;
        }
        pointer operator->() const
        {
            It tmp = cur_;
            --tmp;
            return std::addressof(*tmp);
        }
        reverse_iterator &operator++()
        {
            --cur_;
            return *this;
        }
        reverse_iterator operator++(int)
        {
            auto t = *this;
            --cur_;
            return t;
        }
        reverse_iterator &operator--()
        {
            ++cur_;
            return *this;
        }
        reverse_iterator operator--(int)
        {
            auto t = *this;
            ++cur_;
            return t;
        }

        reverse_iterator operator+(difference_type n) const
        {
            return reverse_iterator(cur_ - n);
        }
        reverse_iterator &operator+=(difference_type n)
        {
            cur_ -= n;
            return *this;
        }
        reverse_iterator operator-(difference_type n) const
        {
            return reverse_iterator(cur_ + n);
        }
        reverse_iterator &operator-=(difference_type n)
        {
            cur_ += n;
            return *this;
        }
        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }
        friend difference_type operator-(const reverse_iterator &a, const reverse_iterator &b)
        {
            return b.cur_ - a.cur_;
        }
        friend bool operator==(const reverse_iterator &a, const reverse_iterator &b)
        {
            return a.cur_ == b.cur_;
        }
        friend bool operator!=(const reverse_iterator &a, const reverse_iterator &b)
        {
            return !(a == b);
        }
        friend bool operator<(const reverse_iterator &a, const reverse_iterator &b)
        {
            return b.cur_ < a.cur_;
        }
        friend bool operator>(const reverse_iterator &a, const reverse_iterator &b)
        {
            return b < a;
        }
        friend bool operator<=(const reverse_iterator &a, const reverse_iterator &b)
        {
            return !(b < a);
        }
        friend bool operator>=(const reverse_iterator &a, const reverse_iterator &b)
        {
            return !(a < b);
        }

    private:
        It cur_;
    }; // end class reverse_iterator
    //辅助创建器
    template<class It>
    inline reverse_iterator<It> make_reverse_iterator(It it)
    {
        return reverse_iterator<It>(it);
    }
    template<class It>
    inline reverse_iterator<It> rbegin(It first,It last)
    {
        (void)first;// 明确告诉编译器：我“有意”不使用 first
        return reverse_iterator<It>(last);
    }
    template<class It>
    inline reverse_iterator<It> rend(It first,It last)
    {
        (void) last;    // 明确告诉编译器：我“有意”不使用 last
        return reverse_iterator<It>(first);
    }
}