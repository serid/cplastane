#pragma once

#include <optional>
#include <functional>

#include "../result/result.hxx"

template<typename T>
class Option : public std::optional<T> {
public:
    template<class U>
    auto map(std::function<U(T)> f) -> Option<U> {
        if (this->has_value())
            return Option(f(this->value()));
        return Option<U>();
    }

    // Haskell Monad.>>=, Rust Option::and_then()
    template<class U>
    auto bind(std::function<Option<U>(T)> k) -> Option<U> {
        if (this->has_value())
            return k(this->value());
        return Option<U>();
    }

    // Haskell Monad.<|>, Rust Option::or_else()
    auto choice(std::function<Option<T>()> k) -> Option<T> {
        if (this->has_value())
            return *this;
        return k();
    }

    // Rust Option::unwrap_or()
    template<typename E>
    auto unwrap_or(E &&error) && -> Result<T, E> {
        if (this->has_value())
            return make_ok<T, E>(std::move(this->value()));
        else
            return make_err<T, E>(std::forward<E>(error));
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

    constexpr Option() noexcept: std::optional<T>() {}

    template<typename U = T>
    explicit constexpr Option(U &&value) : std::optional<T>(std::forward<U>(value)) {};
};

template<typename T>
constexpr auto make_option(T &&v) -> Option<std::decay_t<T>> {
    return Option<std::decay_t<T>>{std::forward<T>(v)};
}