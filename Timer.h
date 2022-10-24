#ifndef PROJECT_TIMER_H
#define PROJECT_TIMER_H

#include <iostream>
#include <string>
#include <chrono>

class Timer {
public:
    Timer() : last(std::chrono::steady_clock::now()) {}

    double get_time_diff() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = now - last;
        last = std::chrono::steady_clock::now();
        return diff.count();
    }

    void time (const std::string& msg = "Time Taken: ") {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = now - last;
        std::cout << msg << diff.count() << "[s]\n";
        last = std::chrono::steady_clock::now();
    }
private:
    std::chrono::steady_clock::time_point last;
};
#endif //PROJECT_TIMER_H