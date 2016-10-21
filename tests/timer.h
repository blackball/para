#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ostream>


class Timer {
    typedef std::chrono::high_resolution_clock high_resolution_clock;
    typedef std::chrono::milliseconds milliseconds;
public:
    explicit Timer(bool run = false)
    {
        if (run)
            Start();
    }
    
    void Start()
    {
        _start = high_resolution_clock::now();
    }
    
    milliseconds End() const
    {
        return std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - _start);
    }
    
    template <typename T, typename Traits>
    friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
    {
        return out << timer.End().count() << "\n";
    }
private:
    high_resolution_clock::time_point _start;
};



#endif /* TIMER_H */
