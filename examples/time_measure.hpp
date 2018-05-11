#ifndef _TIME_MEASURE_H_
#define _TIME_MEASURE_H_

#include <chrono>
#include <iostream>

class time_measure
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;
    using nanoseconds = std::chrono::nanoseconds;
    using seconds = std::chrono::seconds;

    time_measure()
        : start_{ clock::now() }
    {}

    void finish(std::size_t iterations) {
        auto end{ clock::now() };
        auto duration{ end - start_ };
        auto ns{ std::chrono::duration_cast<nanoseconds>(duration).count() };
        double seconds{ ns / 1000000000.0 };
        double milliseconds{ seconds * 1000.0 };
        double iter_per_sec{ iterations / seconds };
        std::cout << "nanoseconds: " << ns << "\n";
        std::cout << "milliseconds: " << milliseconds << "\n";
        std::cout << "seconds: " << seconds << "\n";
        std::cout << "iterations: " << iterations << "\n";
        std::cout << "per iteration: " << ns / iterations << " ns\n";
        std::cout << "iterations per second: " <<
            static_cast<int>(iter_per_sec) << "\n";
    }

private:
    time_point start_;
};

#endif
