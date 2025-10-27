#include "./thread.h"

#include <pthread.h>

void thread_current_sleep_until_us(u64 us) {
    struct timespec ts;
    ts.tv_sec  = us / 1000000ULL;             // convert microseconds to seconds
    ts.tv_nsec = (us % 1000000ULL) * 1000ULL; // remainder converted to nanoseconds
    
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    pthread_mutex_lock(&lock);
    pthread_cond_timedwait(&cond, &lock, &ts);
    pthread_mutex_unlock(&lock);
}
