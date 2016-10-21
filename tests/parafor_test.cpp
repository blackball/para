#include <parafor.h>
#include "timer.h"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>

// g++ --std=c++11 -Wall -O3 -fopenmp -I../include

static bool
PrimeTest(long val) {
    usleep(val);
    bool flag = true;
    for (int i = 2; i < val; ++i) {
        if (val % i == 0) {
            flag = false;
        }
    }
    return flag;
}

struct PrimeOp {
    const std::vector<long> &_vals;
    size_t _sum;
    
    PrimeOp(const std::vector<long> &vals):_vals(vals), _sum(0) {
    }
    
    const int GetNumItems() const {
        return _vals.size();
    }
    
    inline void operator()(int index, int tid) {
        if (PrimeTest(_vals[index]))
            ++_sum;
    }

    inline void Post() {
        // you need ordered, we could sort.
    }
};


#pragma omp declare reduction (merge : std::vector<long> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))
void PrimeTaskOmp(std::vector<long>&in, std::vector<long>&out) {
    size_t len = in.size();
    size_t sum = 0;
#pragma omp parallel for schedule(dynamic, 8) reduction(+: sum) num_threads(4) 
    for (size_t i = 0; i < len; i ++) {
        if (PrimeTest(in[i])) ++sum;
    }
}

static void
Compare() {
#define N 1000
    std::vector<long> vals, dst;
    for (long i = 0; i < N; ++i)
        vals.push_back(i);
    
    Timer timer;

    timer.Start();
    PrimeTaskOmp(vals, dst);
    timer.End();
    std::cout << timer;
    std::cout << dst.size() << std::endl;
    dst.clear();

    ParaFor<PrimeOp, 4> pf(vals);
    
    timer.Start();
    // PrimeTaskPF(vals, dst);
    pf.Join();    
    timer.End();
    std::cout << timer;
    std::cout << dst.size() << std::endl;

    timer.Start();
    // PrimeTaskPF(vals, dst);
    pf.Join();    
    timer.End();
    std::cout << timer;
    std::cout << dst.size() << std::endl;
}

int
main(int argc, char *argv[]) {
    Compare();
    return 0;
}
