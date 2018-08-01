#pragma once

#include <chrono>
#include <type_traits>
#include <fplus/function_traits.hpp>
#include <fplus/container_common.hpp>
#include <fplus/internal/composition.hpp>
#include <fplus/internal/asserts/composition.hpp>

#include <cassert>
#include <exception>
#include <functional>
#include <memory>

namespace fplus
{
    using ExecutionTime = double; // in seconds

    // Holds a value of type T plus a time
    template <typename T>
    class timed
    {
    public:
        timed() : time_(), val_()                                           {}
        timed(const T& val, ExecutionTime t = 0.) : time_(t), val_(val)     {}
        timed(const timed<T>& other) : time_(other.time_), val_(other.val_) {}
        ~timed() = default;

        timed<T>& operator = (const timed<T>& other) {
            val_ = other.val_;
            time_ = other.time_;
            return *this;
        }

        // Execution time in seconds
        ExecutionTime time() const { return time_; } 
        // Inner value
        const T& get() const       { return val_; }
        T& get()                   { return val_; }

    private:
        ExecutionTime time_;
        T val_;
    };


    // API search type: show_maybe : Timed a -> String
    // fwd bind count: 0
    // show_timed((42,1)) -> "42 (1000ms)"
    template <typename T>
    std::string show_timed(const fplus::timed<T>& v)
    {
        std::string result =
            fplus::show(v.get()) + " (" + fplus::show(v.time() * 1000.) + "ms)";
        return result;
    }


    template<typename Fn>
    class timed_function_impl
    {
    public:
        explicit timed_function_impl(Fn fn) : _fn(fn) {};
        template<typename ...Args> auto operator()(Args... args) { return _timed_result(args...); }
    
    private:
        template<typename ...Args>
        auto _timed_result(Args... args)
        {
            fplus::stopwatch timer;
            auto r = _fn(args...);
            auto r_t = fplus::timed<decltype(r)>(r, timer.elapsed());
            return r_t;
        }

        Fn _fn;
    };

    // API search type: make_timed_function : ((a -> b)) -> (a -> Timed b)
    // fwd bind count: 0
    // Transforms a function into a timed / benchmarked version of the same function.
    //
    // Example:
    //
    // using Ints = std::vector<int>;
    // Ints ascending_numbers = fplus::numbers(0, 1000);
    // Ints shuffled_numbers = fplus::shuffle(std::mt19937::default_seed, ascending_numbers);
    // auto sort_func = [](const Ints& values) { return fplus::sort(values); };
    // auto sort_bench = fplus::make_timed_function(sort_func);
    // auto sorted_numbers = sort_bench(shuffled_numbers);
    // // sorted_numbers.get() will return the actual computation output
    // assert(sorted_numbers.get() == ascending_numbers);
    // // sorted_numbers.time() will return the execution time in seconds
    // assert(sorted_numbers.time() < 0.1);
    template<class Fn>
    auto make_timed_function(Fn f)
    {
        return timed_function_impl<decltype(f)>(f);
    }
}