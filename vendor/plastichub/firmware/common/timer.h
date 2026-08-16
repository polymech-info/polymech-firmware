#ifndef TIMER_H
#define TIMER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "macros.h"

#ifndef TIMER_MAX_TASKS
#define TIMER_MAX_TASKS 0x10
#endif

template <
    size_t max_tasks = TIMER_MAX_TASKS,   /* max allocated tasks */
    unsigned long (*time_func)() = millis /* time function for timer */
    >
class Timer
{
public:
    typedef bool (*handler_t)(void *opaque);  /* task handler func signature */
    /* Calls handler with opaque as argument in delay units of time */
    bool
    in(unsigned long delay, handler_t h, void *opaque = NULL)
    {
        return add_task(time_func(), delay, h, opaque);
    }

    /* Calls handler with opaque as argument at time */
    bool
    at(unsigned long time, handler_t h, void *opaque = NULL)
    {
        const unsigned long now = time_func();
        return add_task(now, time - now, h, opaque);
    }

    /* Calls handler with opaque as argument every interval units of time */
    bool
    every(unsigned long interval, handler_t h, void *opaque = NULL)
    {
        return add_task(time_func(), interval, h, opaque, interval);
    }

    
    /* Ticks the timer forward - call this function in loop() */
    void
    tick()
    {
        tick(time_func());
    }

    /* Ticks the timer forward - call this function in loop() */
    inline void
    tick(unsigned long t)
    {
        for (size_t i = 0; i < max_tasks; ++i)
        {
            struct task *const task = &tasks[i];
            const unsigned long duration = t - task->start;

            if (task->handler && duration >= task->expires)
            {
                task->repeat = task->handler(task->opaque) && task->repeat;

                if (task->repeat)
                    task->start = t;
                else
                    remove(task);
            }else{
            }
        }
    }

private:
    struct task
    {
        handler_t handler; /* task handler callback func */
        void *opaque;      /* argument given to the callback handler */
        unsigned long start,
            expires, /* when the task expires */
            repeat;  /* repeat task */
    } tasks[max_tasks];

    inline void
    remove(struct task *task)
    {
        task->handler = NULL;
        task->opaque = NULL;
        task->start = 0;
        task->expires = 0;
        task->repeat = 0;
    }

    inline struct task *
    next_task_slot()
    {
        for (size_t i = 0; i < max_tasks; ++i)
        {
            struct task *const slot = &tasks[i];
            if (slot->handler == NULL)
                return slot;
        }

        return NULL;
    }

    inline struct task *
    add_task(unsigned long start, unsigned long expires,
             handler_t h, void *opaque, bool repeat = 0)
    {
        struct task *const slot = next_task_slot();

        if (!slot){
            return NULL;
        }

        slot->handler = h;
        slot->opaque = opaque;
        slot->start = start;
        slot->expires = expires;
        slot->repeat = repeat;

        return slot;
    }
};

/* create a timer with the default settings */
inline Timer<>
timer_create_default()
{
    return Timer<>();
}

#endif
