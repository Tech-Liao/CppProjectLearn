#pragma once
#include <iterator>
#include "Iterator_Traits.h"

namespace my
{
    template <class It>
    class Reverse_Iterator
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
        Reverse_Iterator() : cur_() {}
        explicit Reverse_Iterator(It it) : cur_(it) {}
        template <class U>
        Reverse_Iterator(const Reverse_Iterator<U> &other) : cur_(other.base()) {}
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
        Reverse_Iterator &operator++()
        {
            --cur_;
            return *this;
        }
        Reverse_Iterator operator++(int)
        {
            auto t = *this;
            --cur_;
            return t;
        }
        Reverse_Iterator &operator--()
        {
            ++cur_;
            return *this;
        }
        Reverse_Iterator operator--(int)
        {
            auto t = *this;
            ++cur_;
            return t;
        }

        Reverse_Iterator operator+(difference_type n) const
        {
            return Reverse_Iterator(cur_ - n);
        }
        Reverse_Iterator &operator+(difference_type n)
        {
            cur_ -= n;
            return *this;
        }
        Reverse_Iterator operator-(difference_type n) const
        {
            return Reverse_Iterator(cur_ + n);
        }
        Reverse_Iterator &operator-(difference_type n)
        {
            cur_ += n;
            return *this;
        }
        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }
        friend difference_type operator-(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return b.cur_ - a.cur_;
        }
        friend bool operator==(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return a.cur_ == b.cur_;
        }
        friend bool operator!=(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return !(a == b);
        }
        friend bool operator<(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return b.cur_ < a.cur_;
        }
        friend bool operator>(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return b < a;
        }
        friend bool operator<=(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return !(b < a);
        }
        friend bool operator>=(const Reverse_Iterator &a, const Reverse_Iterator &b)
        {
            return !(a < b);
        }

    private:
        It cur_;
    }; // end class Reverse_Iterator
}