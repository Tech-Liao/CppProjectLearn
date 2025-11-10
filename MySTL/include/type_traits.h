#pragma once
// 最小可用的 type_traits 实现（C++11 基线）
// 仅依赖语言本身；未使用 <type_traits>
namespace my {
// --------- 基础常量与布尔类型 ---------
template <class T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template <bool B>
using bool_constant = integral_constant<bool, B>;
// -----基础工具----------
template <bool Cond, class T = void>
struct enable_if {};
template <class T>
struct enable_if<true, T> {
    using type = T;
};

template <bool Cond, class T, class F>
struct conditional {
    using type = T;
};
template <class T, class F>
struct conditional<false, T, F> {
    using type = F;
};

template <class T, class U>
struct is_same : false_type {};
template <class T>
struct is_same<T, T> : true_type {};

// ---------引用/修饰符移除与添加--------
template <class T>
struct remove_const {
    using type = T;
};
template <class T>
struct remove_const<const T> {
    using type = T;
};

template <class T>
struct remove_volatile {
    using type = T;
};
template <class T>
struct remove_volatile<volatile T> {
    using type = T;
};

template <class T>
struct remove_cv {
    using type = typename remove_volatile<typename remove_const<T>::type>::type;
};

template <class T>
struct remove_reference {
    using type = T;
};
template <class T>
struct remove_reference<T &> {
    using type = T;
};
template <class T>
struct remove_reference<T &&> {
    using type = T;
};

template <class T>
struct is_lvalue_reference : my::false_type {};
template <class T>
struct is_lvalue_reference<T &> : my::true_type {};

template <class T>
struct add_lvalue_reference {
    using type = T &&;
};
template <class T>
struct add_lvalue_reference<T &> {
    using type = T &;
};
template <class T>
struct add_lvalue_reference<T &&> {
    using type = T &;
};

template <class T>
struct add_rvalue_reference {
    using type = T &&;
};
template <class T>
struct add_rvalue_reference<T &> {
    using type = T &;
};
template <class T>
struct add_rvalue_reference<T &&> {
    using type = T &&;
};

template <class T>
struct add_pointer {
    using type = typename remove_reference<T>::type *;
};

//	--------数组/函数判断(为decay/make_pair作准备)
template <class T>
struct is_array : false_type {};
template <class T>
struct is_array<T[]> : true_type {};
template <class T, unsigned long N>
struct is_array<T[N]> : true_type {};

template <class T>
struct remove_extent {
    using type = T;
};
template <class T>
struct remove_extent<T[]> {
    using type = T;
};
template <class T, unsigned long N>
struct remove_extent<T[N]> {
    using type = T;
};

template <class T>
struct remove_all_extents {
    using type = T;
};
template <class T>
struct remove_all_extents<T[]> {
    using type = typename remove_all_extents<T>::type;
};
template <class T, unsigned long N>
struct remove_all_extents<T[N]> {
    using type = typename remove_all_extents<T>::type;
};

template <class>
struct is_function : false_type {};
template <class R, class... Args>
struct is_function<R(Args...)> : true_type {};
template <class R, class... Args>
struct is_function<R(Args..., ...)> : true_type {};

// 带 cv/ref 的函数类型
template <class R, class... Args>
struct is_function<R(Args...) const> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) volatile> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) const volatile> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) &> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) &&> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) const &> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) const &&> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) volatile &> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) volatile &&> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) const volatile &> : true_type {};
template <class R, class... Args>
struct is_function<R(Args...) const volatile &&> : true_type {};

// --------- decay：去引用 + 去 cv + 数组转指针 + 函数转指针 ---------
template <class T>
struct decay {
   private:
    using U = typename remove_reference<T>::type;

   public:
    // 三类名:cond = is_array<U>::value,
    // T=ypename add_pointer<typename remove_extent<U>::type>::type
    /*
     F = typename conditional<
        is_function<U>::value,
        typename add_pointer<U>::type,
        typename remove_cv<U>::type>::type>::type
    */
    using type = typename conditional<
        is_array<U>::value,
        typename add_pointer<typename remove_extent<U>::type>::type,
        typename conditional<is_function<U>::value,
                             typename add_pointer<U>::type,
                             typename remove_cv<U>::type>::type>::type;
};

// --------- is_integral（用于 SFINAE 约束示例） ---------

template <class T>
struct is_integral_base : false_type {};
template <>
struct is_integral_base<bool> : true_type {};
template <>
struct is_integral_base<char> : true_type {};
template <>
struct is_integral_base<signed char> : true_type {};
template <>
struct is_integral_base<unsigned char> : true_type {};
#ifdef __cpp_char8_t
template <>
struct is_integral_base<char8_t> : true_type {};
#endif
template <>
struct is_integral_base<char16_t> : true_type {};
template <>
struct is_integral_base<char32_t> : true_type {};
template <>
struct is_integral_base<wchar_t> : true_type {};
template <>
struct is_integral_base<short> : true_type {};
template <>
struct is_integral_base<unsigned short> : true_type {};
template <>
struct is_integral_base<int> : true_type {};
template <>
struct is_integral_base<unsigned int> : true_type {};
template <>
struct is_integral_base<long> : true_type {};
template <>
struct is_integral_base<unsigned long> : true_type {};
template <>
struct is_integral_base<long long> : true_type {};
template <>
struct is_integral_base<unsigned long long> : true_type {};

template <class T>
struct is_integral : is_integral_base<typename remove_cv<T>::type> {};

// --------- 逻辑组合（C++17 的 and/or/not） ---------
template <class...>
struct conjunction : true_type {};
template <class B1>
struct conjunction<B1> : B1 {};
template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : conditional<B1::value, conjunction<Bn...>, B1>::type {};

template <class...>
struct disjunction : false_type {};
template <class B1>
struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...>
    : conditional<B1::value, conjunction<Bn...>, B1>::type {};
template <class B>
struct negation : bool_constant<!B::value> {};
}  // namespace my