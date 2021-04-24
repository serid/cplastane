#pragma once

#include <variant>
#include <functional>

using namespace std;

template<typename T, typename E>
class Result : variant<T, E> {
public:
    auto is_ok() const -> bool {
        return this->index() == 0;
    }

    auto is_err() const -> bool {
        return this->index() != 0;
    }

    template<class U>
    auto map(function<U(T)> f) const -> Result<U, E> {
        if (this->is_ok())
            return Result(f(this->value()));
        return *this;
    }

    // Haskell Monad.>>=, Rust Result::and_then()
    template<class U>
    auto bind(function<Result<U, E>(T)> k) const -> Result<U, E> {
        if (this->is_ok())
            return k(this->value());
        return *this;
    }

    // Haskell Monad.<|>, Rust Result::or_else()
    auto choice(function<Result<T, E>()> k) const -> Result<T, E> {
        if (this->is_ok())
            return *this;
        return k();
    }

    auto value() -> T& {
        if (this->is_ok())
            return get<0>(*this);
        throw logic_error("called unwrap on an Error value");
    }

    auto error() -> E& {
        if (this->is_err())
            return get<1>(*this);
        throw logic_error("called unwrap_err on an Ok value");
    }

    auto copy_value() const -> T {
        if (this->is_ok())
            return get<0>(*this);
        throw logic_error("called unwrap on an Error value");
    }

    auto copy_error() const -> E {
        if (this->is_err())
            return get<1>(*this);
        throw logic_error("called unwrap_err on an Ok value");
    }

    constexpr Result(T&& t) : variant<T, E>(forward<T>(t)) {};

    constexpr Result(E&& e) : variant<T, E>(forward<E>(e)) {};

    operator bool() const {
        return this->is_ok();
    }
};

template<typename T, typename E>
constexpr auto make_ok(T &&value) -> Result<decay_t<T>, E> {
    return Result<decay_t<T>, E>{forward<T>(value)};
}

template<typename T, typename E>
constexpr auto make_err(E &&error) -> Result<T, decay_t<E>> {
    return Result<T, decay_t<E>>{forward<E>(error)};
}
