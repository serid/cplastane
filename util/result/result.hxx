#pragma once

#include <variant>
#include <functional>

template<typename T, typename E>
class Result : std::variant<T, E> {
public:
    auto is_ok() const -> bool {
        return this->index() == 0;
    }

    auto is_err() const -> bool {
        return this->index() != 0;
    }

    template<class U>
    auto map(std::function<U(T)> f) const -> Result<U, E> {
        if (this->is_ok())
            return Result(f(this->value()));
        return *this;
    }

    // Haskell Monad.>>=, Rust Result::and_then()
    template<class U>
    auto bind(std::function<Result<U, E>(T)> k) const -> Result<U, E> {
        if (this->is_ok())
            return k(this->value());
        return *this;
    }

    // Haskell Monad.<|>, Rust Result::or_else()
    auto choice(std::function<Result<T, E>()> k) const -> Result<T, E> {
        if (this->is_ok())
            return *this;
        return k();
    }

    auto value() -> T& {
        if (this->is_ok())
            return std::get<0>(*this);
        throw std::logic_error("called unwrap on an Error value");
    }

    auto error() -> E& {
        if (this->is_err())
            return std::get<1>(*this);
        throw std::logic_error("called unwrap_err on an Ok value");
    }

    auto copy_value() const -> T {
        if (this->is_ok())
            return std::get<0>(*this);
        throw std::logic_error("called unwrap on an Error value");
    }

    auto copy_error() const -> E {
        if (this->is_err())
            return std::get<1>(*this);
        throw std::logic_error("called unwrap_err on an Ok value");
    }

    constexpr Result(T&& t) : std::variant<T, E>(std::forward<T>(t)) {};

    constexpr Result(E&& e) : std::variant<T, E>(std::forward<E>(e)) {};

    operator bool() const {
        return this->is_ok();
    }
};

template<typename T, typename E>
constexpr auto make_ok(T &&value) -> Result<std::decay_t<T>, E> {
    return Result<std::decay_t<T>, E>{std::forward<T>(value)};
}

template<typename T, typename E>
constexpr auto make_err(E &&error) -> Result<T, std::decay_t<E>> {
    return Result<T, std::decay_t<E>>{std::forward<E>(error)};
}
