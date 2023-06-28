
#ifndef UTHREAD_H
#define UTHREAD_H

#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>
#include <vector>

#define DEFAULT_STACK_SIZE (1024 * 128)
#define MAX_UTHREAD_SIZE 1024

// 四种状态：可用，可运行，正在运行，挂起
enum ThreadState { FREE, RUNNABLE, RUNNING, SUSPEND };

typedef void (*Fun)(void *arg);

// 协程结构体
typedef struct uthread_t {
    ucontext_t ctx; // 保存协程上下文
    Fun func;       // 协程执行的用户函数
    void *arg;      // 参数
    enum ThreadState state;
    char stack[DEFAULT_STACK_SIZE]; // 协程栈
} uthread_t;

typedef std::vector<uthread_t> Thread_vector;

// 调度器结构体
typedef struct schedule_t {
    ucontext_t main;
    int running_thread;
    int max_index = MAX_UTHREAD_SIZE;
    // std::vector<uthread_t> threads(max_index);
    uthread_t *threads;

    schedule_t() : running_thread(-1), max_index(0) {
        threads = new uthread_t[MAX_UTHREAD_SIZE];
        for (int i = 0; i < MAX_UTHREAD_SIZE; i++) {
            threads[i].state = FREE;
        }
    }
} schedule_t;

// 创建一个协程
int uthread_create(schedule_t &schedule, Fun func, void *arg);
// 挂起调度器schedule中当前正在执行的协程，切换到主函数
void uthread_yield(schedule_t &schedule);
// 恢复运行调度器中编号为id的协程
void uthread_resume(schedule_t &schedule, int id);
// 判断调度器中所有协程是否都执行完毕
int schedule_finished(const schedule_t &schedule);

#endif