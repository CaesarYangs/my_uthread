#include "uthread.h"

// implementation of the resume func
void uthread_resume(schedule_t &schedule, int id) {
    if (id < 0 || id >= schedule.max_index) {
        return;
    }

    uthread_t *t = &(schedule.threads[id]);

    if (t->state == RUNNABLE) {
        getcontext(&(t->ctx));
        t->ctx.uc_stack.ss_sp = t->stack;
        t->ctx.uc_stack.ss_size = DEFAULT_STACK_SIZE;
        t->ctx.uc_stack.ss_flags = 0;
        t->ctx.uc_link = &(schedule.main);
        t->state = RUNNING;
        schedule.running_thread = id;
        makecontext(&(t->ctx), (void (*)(void))(uthread_body), 1, &schedule);
    }

    if (t->state == SUSPEND) {
        swapcontext(&(schedule.main), &(t->ctx));
    }
}

// implementation of the yield func
void uthread_yield(schedule_t &schedule) {
    if (schedule.running_thread != -1) {
        uthread_t *t = &(schedule.threads[schedule.running_thread]);
        t->state = SUSPEND;
        schedule.running_thread = -1;

        swapcontext(&(t->ctx), &(schedule.main));
    }
}

void uthread_body(schedule_t *ps) {
    int id = ps->running_thread;

    if (id != -1) {
        uthread_t *t = &(ps->threads[id]);
        t->func(t->arg);
        t->state = FREE;
        ps->running_thread = -1;
    }
}