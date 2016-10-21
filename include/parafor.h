#ifndef PARAFOR_H
#define PARAFOR_H

#include <pthread.h> // TODO: windows 
#include <utility> // forward
#include <limits.h> // INT_MAX

template<typename OP, int TN>
struct ParaFor {
    OP _op; // we may need to reset _op
    template <typename... Args>
    ParaFor(Args&&... args) : _op(std::forward<Args>(args)...) {
    }
    
    void Join() {
        if (TN > 1) {
            for (int i = 0; i < TN; ++i) {
                _workers[i].currindex = i;
                _workers[i].tid = i;
                _workers[i].pf = this;
            }

            for (int i = 0; i < TN; ++i) {
                pthread_create(&_threads[i], 0, &(ParaFor<OP, TN>::_Callback), &_workers[i]);
            }

            for (int i = 0; i < TN; ++i) {
                pthread_join(_threads[i], NULL);
            }
        }
        else if (TN == 1) {
            const int numitems = _op.GetNumItems();
            for (int i = 0; i < numitems; ++i)
                _op(i, 0);
        }
        _op.Post();
    }

private:
    struct _Worker {
        int currindex, tid;
        ParaFor<OP, TN> *pf;
    };
    
    pthread_t _threads[TN];
    _Worker _workers[TN];

private:
    ParaFor(ParaFor<OP, TN> const&);
    ParaFor<OP, TN>& operator=(ParaFor<OP, TN> const&);

    static inline void* _Callback(void *data) {
        ((_Worker *)data)->pf->_Work((_Worker *)data);
        return NULL;
    }

    void _Work(_Worker *worker) {
        const int tid = worker->tid, numitems = _op.GetNumItems();
        int *pcurrindex = &(worker->currindex);
        
        for (int itemindex = __sync_fetch_and_add(pcurrindex, TN); itemindex < numitems; ) {
            _op(itemindex, tid);
            itemindex = __sync_fetch_and_add(pcurrindex, TN);
        }

        for (int itemindex = _Steal(); itemindex >= 0; itemindex = _Steal()) {
            _op(itemindex, tid);
        }
        
        pthread_exit(0);
    }

    int _Steal() {
        int tid = 0, mintid = 0, nextitemindex = 0, minitemindex = INT_MAX;
        for (; tid < TN; ++tid) {
            if (minitemindex > _workers[tid].currindex) {
                minitemindex = _workers[tid].currindex;
                mintid = tid;
            }
        }
        
        nextitemindex = __sync_fetch_and_add(&(_workers[mintid].currindex), TN);
        if (nextitemindex >= _op.GetNumItems()) {
            return -1;
        }
        return nextitemindex;
    }
};

#endif /* PARAFOR_H */
