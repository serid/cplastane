#pragma once

#include <optional>
#include <functional>

template<typename T>
class Option : public std::optional<T> {
    typedef T value_type;

public:
    auto map(std::function<T(T)> f) -> Option<T> {
        if (this->has_value())
            return Option(f(**this));
        return Option<T>();
    }

    // Haskell Monad.>>=, Rust Option::and_then()
    template<class U>
    auto bind(std::function<Option<U>(T)> k) -> Option<U> {
        if (this->has_value())
            return k(**this);
        return Option<U>();
    }

    // Haskell Monad.<|>, Rust Option::or_else()
    auto choice(std::function<Option<T>()> k) -> Option<T> {
        if (this->has_value())
            return *this;
        return k();
    }

    /*
    // Try to mirror std::optional's constructors

    // (1)
    constexpr Option() noexcept: std::optional<T>() {}

    // (2)
    // TODO: corresponding constructor from std::optional mentions something about triviality
    template<std::enable_if_t<std::is_copy_constructible_v<T>> * = nullptr>
    constexpr Option(const Option &other) : std::optional<T>(other) {};

    constexpr Option(const Option &other) = delete;

    // (3)
    template<std::enable_if_t<std::is_move_constructible_v<T>> * = nullptr>
    constexpr Option(Option &&other) : std::optional<T>(other) {};

    // (4)
    template<typename U>
    static inline constexpr bool doge =
            std::is_constructible_v<T, const U &> &&
            !std::is_constructible_v<T, std::optional<U> &> &&
            !std::is_constructible_v<T, const std::optional<U> &> &&
            !std::is_constructible_v<T, std::optional<U> &&> &&
            !std::is_constructible_v<T, const std::optional<U> &&> &&
            !std::is_convertible_v<std::optional<U> &, T> &&
            !std::is_convertible_v<const std::optional<U> &, T> &&
            !std::is_convertible_v<std::optional<U> &&, T> &&
            !std::is_convertible_v<const std::optional<U> &&, T>;

    template<typename U>
    static inline constexpr bool is_explicit_4 = std::is_convertible_v<const U &, T> == false;

    template<typename U, std::enable_if_t<doge<U> && !is_explicit_4<U>> * = nullptr>
    Option(const Option<U> &other);

    template<typename U, std::enable_if_t<doge<U> && is_explicit_4<U>> * = nullptr>
    explicit Option(const Option<U> &other);

    // (8)
    template<typename U>
    static inline constexpr bool doge2 = std::is_constructible_v<T, U &&>;

    template<typename U>
    static inline constexpr bool is_explicit_8 = std::is_convertible_v<U &&, T> == false;

    template<typename U = value_type, std::enable_if_t<doge2<U> && !is_explicit_8<U>> * = nullptr>
    constexpr Option(U &&value) : std::optional<T>(std::forward<U>(value)) {};

    template<typename U = value_type, std::enable_if_t<doge2<U> && is_explicit_8<U>> * = nullptr>
    explicit constexpr Option(U &&value) : std::optional<T>(std::forward<U>(value)) {};

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