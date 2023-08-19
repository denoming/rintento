#pragma once

#include <chrono>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace jar {

namespace details {

class Base {
public:
    Base()
        : _notified{false}
    {
    }

    virtual ~Base() = default;

    void
    wait()
    {
        std::unique_lock lock{_guard};
        _condition.wait(lock, [this]() { return _notified; });
    }

    template<typename Rep, typename Period = std::ratio<1>>
    bool
    wait(std::chrono::duration<Rep, Period> timeout)
    {
        std::unique_lock lock{_guard};
        auto result = _condition.wait_for(lock, timeout, [this]() { return _notified; });
        _notified = false;
        return result;
    }

    void
    reset()
    {
        std::lock_guard lock{_guard};
        _notified = false;
    }

protected:
    void
    notify()
    {
        std::unique_lock lock{_guard};
        _notified = true;
        lock.unlock();
        _condition.notify_one();
    }

private:
    bool _notified;
    mutable std::mutex _guard;
    std::condition_variable _condition;
};

} // namespace details

template<typename F>
class Waiter;

template<typename R, typename... Args>
class Waiter<R(Args...)> : public details::Base {
public:
    auto
    enroll()
    {
        return [this](Args... args) -> void {
            this->call(args...);
            if (this->check(args...)) {
                Base::notify();
            }
        };
    }

    template<typename F>
        requires std::invocable<F, Args...>
    auto
    enroll(F&& fn)
    {
        this->onCall(std::forward<F>(fn));
        return [this](Args... args) -> void {
            this->call(args...);
            if (this->check(args...)) {
                Base::notify();
            }
        };
    }

    template<typename F>
        requires std::invocable<F, Args...>
    void
    onCall(F&& fn)
    {
        _callable = fn;
    }

    template<typename F>
        requires std::predicate<F, Args...>
    void
    onCheck(F&& fn)
    {
        _predicate = fn;
    }

private:
    void
    call(Args... args)
    {
        if (_callable) {
            std::invoke(_callable, args...);
        }
    }

    bool
    check(Args... args)
    {
        if (_predicate) {
            return std::invoke(_predicate, args...);
        }
        return true;
    }

private:
    std::function<void(Args...)> _callable;
    std::function<bool(Args...)> _predicate;
};

template<typename F>
class Waiter : public details::Base {
public:
    auto
    enroll(F&& fn)
    {
        Base::reset();
        return [this, fn = std::forward<F>(fn)]() -> void {
            if (fn()) {
                Base::notify();
            }
        };
    }
};

} // namespace jar
