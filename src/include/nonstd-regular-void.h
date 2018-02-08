
#include <type_traits>

// If x might have void type, then whenever you want to write f(x),
// you should write invoke_with_result_of(f, [&]{return x;}) instead.
// The convenience macro for this is REGULAR_INVOKE(f, x).

namespace nonstd {

template<class F, class T, class R = decltype(std::declval<T&&>()()), std::enable_if_t<!std::is_void<R>::value, bool> = true>
auto invoke_with_result_of(F&& f, T&& t)
{
    return std::forward<F>(f) ( std::forward<T>(t)() );
}

template<class F, class T, class R = decltype(std::declval<T&&>()()), std::enable_if_t<std::is_void<R>::value, bool> = true>
auto invoke_with_result_of(F&& f, T&& t)
{
    std::forward<T>(t)();
    return std::forward<F>(f)();
}

template<class F, class T, std::enable_if_t<!std::is_void<T>::value, bool> = true>
auto unevaluated_invoke_impl() -> decltype( std::declval<F&&>() ( std::declval<T&&>() ) );

template<class F, class T, std::enable_if_t<std::is_void<T>::value, bool> = true>
auto unevaluated_invoke_impl() -> decltype( std::declval<F&&>()() );

} // namespace nonstd

#define REGULAR_INVOKE(f, x) nonstd::invoke_with_result_of( \
        [&](auto&&... xs_) { return f(std::forward<decltype(xs_)>(xs_)...); }, \
        [&]() -> decltype(auto) {return x;} \
    )

#define UNEVALUATED_INVOKE(f, x) nonstd::unevaluated_invoke_impl<decltype(f), decltype(x)>()
