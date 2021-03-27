#pragma once

#include <optional>
#include <functional>

using namespace std;

template<typename T>
class Option : public optional<T> {
public:
    template<class U>
    auto map(function<U(T)> f) -> Option<U> {
        if (this->has_value())
            return Option(f(**this));
        return Option<U>();
    }

    // Haskell Monad.>>=, Rust Option::and_then()
    template<class U>
    auto bind(function<Option<U>(T)> k) -> Option<U> {
        if (this->has_value())
            return k(**this);
        return Option<U>();
    }

    // Haskell Monad.<|>, Rust Option::or_else()
    auto choice(function<Option<T>()> k) -> Option<T> {
        if (this->has_value())
            return *this;
        return k();
    }

    /*
    // Try to mirror optional's constructors

    // (1)
    constexpr Option() noexcept: optional<T>() {}

    // (2)
    // TODO: corresponding constructor from optional mentions something about triviality
    template<enable_if_t<is_copy_constructible_v<T>> * = nullptr>
    constexpr Option(const Option &other) : optional<T>(other) {};

    constexpr Option(const Option &other) = delete;

    // (3)
    template<enable_if_t<is_move_constructible_v<T>> * = nullptr>
    constexpr Option(Option &&other) : optional<T>(other) {};

    // (4)
    template<typename U>
    static inline constexpr bool doge =
            is_constructible_v<T, const U &> &&
            !is_constructible_v<T, optional<U> &> &&
            !is_constructible_v<T, const optional<U> &> &&
            !is_constructible_v<T, optional<U> &&> &&
            !is_constructible_v<T, const optional<U> &&> &&
            !is_convertible_v<optional<U> &, T> &&
            !is_convertible_v<const optional<U> &, T> &&
            !is_convertible_v<optional<U> &&, T> &&
            !is_convertible_v<const optional<U> &&, T>;

    template<typename U>
    static inline constexpr bool is_explicit_4 = is_convertible_v<const U &, T> == false;

    template<typename U, enable_if_t<doge<U> && !is_explicit_4<U>> * = nullptr>
    Option(const Option<U> &other);

    template<typename U, enable_if_t<doge<U> && is_explicit_4<U>> * = nullptr>
    explicit Option(const Option<U> &other);

    // (8)
    template<typename U>
    static inline constexpr bool doge2 = is_constructible_v<T, U &&>;

    template<typename U>
    static inline constexpr bool is_explicit_8 = is_convertible_v<U &&, T> == false;

    template<typename U = value_type, enable_if_t<doge2<U> && !is_explicit_8<U>> * = nullptr>
    constexpr Option(U &&value) : optional<T>(forward<U>(value)) {};

    template<typename U = value_type, enable_if_t<doge2<U> && is_explicit_8<U>> * = nullptr>
    explicit constexpr Option(U &&value) : optional<T>(forward<U>(value)) {};

    // End Constructor (8)
    */

    constexpr Option() noexcept: optional<T>() {}

    template<typename U = T>
    explicit constexpr Option(U &&value) : optional<T>(forward<U>(value)) {};
};

template<typename T>
constexpr auto make_option(T &&v) -> Option<decay_t<T>> {
    return Option<decay_t<T>>{forward<T>(v)};
}