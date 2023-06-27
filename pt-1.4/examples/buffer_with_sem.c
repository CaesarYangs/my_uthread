/*The bounded buffer with protothread semaphores
The following example shows how to implement the bounded buffer problem using
the protothreads semaphore library. The example uses three protothreads: one
producer() protothread that produces items, one consumer() protothread that
consumes items, and one driver_thread() that schedules the producer and consumer
protothreads.

Note that there is no need for a mutex to guard the add_to_buffer() and
get_from_buffer() functions because of the implicit locking semantics of
protothreads - a protothread will never be preempted and will never block except
in an explicit PT_WAIT statement.
*/

#include "../pt-sem.h"

#define NUM_ITEMS 32
#define BUFSIZE 8

static struct pt_sem full, empty;

static PT_THREAD(producer(struct pt *pt)) {
    static int produced;

    PT_BEGIN(pt);

    for (produced = 0; produced < NUM_ITEMS; ++produced) {
        PT_SEM_WAIT(pt, &full);

        add_to_buffer(produce_item());

        PT_SEM_SIGNAL(pt, &empty);
    }

    PT_END(pt);
}

static PT_THREAD(consumer(struct pt *pt)) {
    static int consumed;

    PT_BEGIN(pt);

    for (consumed = 0; consumed < NUM_ITEMS; ++consumed) {
        PT_SEM_WAIT(pt, &empty);

        consume_item(get_from_buffer());

        PT_SEM_SIGNAL(pt, &full);
    }

    PT_END(pt);
}

static PT_THREAD(driver_thread(struct pt *pt)) {
    static struct pt pt_producer, pt_consumer;

    PT_BEGIN(pt);

    PT_SEM_INIT(&empty, 0);
    PT_SEM_INIT(&full, BUFSIZE);

    PT_INIT(&pt_producer);
    PT_INIT(&pt_consumer);

    PT_WAIT_THREAD(pt, producer(&pt_producer) & consumer(&pt_consumer));

    PT_END(pt);
}