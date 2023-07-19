// 一个自定义的生产者-消费者实现模型
// 利用协程语义，脱离操作系统限制

#include "../pt-sem.h"

#define NUM_ITEMS 32
#define BUFSIZE 8

static struct pt_sem mutex, full, empty;

PT_THREAD(producer(struct pt *pt)) {
    static int produced;

    PT_BEGIN(pt);
    for (produced = 0; produced < NUM_ITEMS; ++produced) {
        PT_SEM_WAIT(pt, &full);
        PT_SEM_WAIT(pt, &mutex);
        add_to_buffer(produce_item());
        PT_SEM_SIGNAL(pt, &mutex);
        PT_SEM_SIGNAL(pt, &empty);
    }
    PT_END(pt);
}

PT_THREAD(consumer(struct pt *pt)) {
    static int consumed;

    PT_BEGIN(pt);
    for (consumed = 0; consumed < NUM_ITEMS; ++consumed) {
        PT_SEM_WAIT(pt, &empty);
        PT_SEM_WAIT(pt, &mutex);
        consume_item(get_from_buffer());
        PT_SEM_SIGNAL(pt, &mutex);
        PT_SEM_SIGNAL(pt, &full);
    }
    PT_END(pt);
}

PT_THREAD(driver_thread(struct pt *pt)) {
    static struct pt pt_producer, pt_consumer;

    PT_BEGIN(pt);
    PT_SEM_INIT(&empty, 0);
    PT_SEM_INIT(&full, BUFSIZE);
    PT_SEM_INIT(&mutex, 1);
    PT_INIT(&pt_producer);
    PT_INIT(&pt_consumer);
    PT_WAIT_THREAD(pt, producer(&pt_producer) & consumer(&pt_consumer));
    PT_END(pt);
}
