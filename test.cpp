#include "uthread.cpp"
#include <stdio.h>

void func2(void *arg) {
    puts("22-1-1");
    puts("22-1-2");
    uthread_yield(*(schedule_t *)arg);
    puts("22-2-1");
    puts("22-2-2");
}

void func3(void *arg) {
    puts("3333-1-2");
    puts("3333-1-2");
    uthread_yield(*(schedule_t *)arg);
    puts("3333-2-1");
    puts("3333-2-2");
}

void schedule_test() {
    schedule_t s;

    int id1 = uthread_create(s, func2, &s);
    int id2 = uthread_create(s, func3, &s);

    while (!schedule_finished(s)) {
        uthread_resume(s, id1);
        uthread_resume(s, id2);
    }
    puts("main over");
}

int main() {
    schedule_test();

    return 0;
}
