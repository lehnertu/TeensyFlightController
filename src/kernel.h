/*  

    This is the core on which TAROS is based.
    
    It handles a list of modules which comprise the application system.
    
    The systick interrupt routine is called every millisecond
    and it in turn calls the interrupt() routine of all modules listed.
    This interrupt routine should only perform minimal work and
    typically return within a few microseconds.
    Time critical work can be done right here, as long as the time constraints are kept.
    The total time needed for the completion of the interrupt routine is monitored.
    
    Within the core we mangage a task list.
    This is intended for all work that may take longer than a few microseconds.
    Every module via its interrupt routine can insert tasks into that list.
    A task consists of a pointer to a static method of the module
    that takes no parameters and returns no values. It only acts on
    the internal state of the module (but could send messages for instance).
    This task list is sequentially processed by the main program.
    
*/

#pragma once

#include <cstdint>
#include <list>
#include <functional>
#include <string>

class Module;

// uint32_t systick_millis_count is used as timestamp
// miliseconds since program start (about 50 days capacity)
extern volatile uint32_t FC_systick_millis_count;

// miliseconds since program start (about 50 days capacity)
uint32_t FC_time_now();

// report the time elapsed since the timestamp
// if current FC_systick_millis_count is small than timestamp wrap-around
uint32_t FC_elapsed_millis(uint32_t timestamp);

// ARM_DWT_CYCCNT is a 32-bit unsigned counter for processor cycles (600 MHz)
// we store its value with every systick to procide sub-millisecond timing
extern volatile uint32_t FC_systick_cycle_count;

// we record the maximum number of CPU cycles between 2 interrupts
// (should be about 600000)
extern volatile uint32_t FC_max_isr_spacing;

// we record the total time the systick interrupt routine needs for completion
// this is updated with every interrupt
// the watchdog can use these values
extern volatile uint32_t FC_isr_duration;

// we record the longest time it took to complete an interrupt request
extern volatile uint32_t FC_max_isr_time_to_completion;
extern std::string FC_max_isr_time_module;

// we record the longest time it takes from scheduling a task
// until the task execution is actually started in CPU cycles
extern volatile uint32_t FC_max_task_delay;

// we record the longest time it takes to complete a task (in CPU cycles)
extern volatile uint32_t FC_max_task_runtime;
extern std::string FC_max_task_runtime_module;

// we use our own ISR for the systick interrupt
// it is copied from EventResponder.cpp (previously delay.c)
// and added with our own functionality
extern "C" void FC_systick_isr(void);

// we use a flag to indicate if it is allowed to call module interrupts
extern "C" bool FC_module_interrupts_active;

// Here are the main initializations that are needed to access the processor hardware.
// 1) bend the interrupt vector to our own ISR
void setup_core_system();

/*
    This is a task descriptor
    TODO: this needs extensions: priority, maybe different entry points
*/
typedef std::function<void ()> TaskFunct;
struct Task 
{
    // the module which has started this task
    Module* module;
    // the CPU cycle when the task has bee requested
    uint32_t request_time;
    // a pointer to the procedure to be executed
    TaskFunct funct;
};

// all modules are registered in a list
extern std::list<Module*> module_list;

// all tasks that have been scheduled for execution
extern std::list<Task> task_list;

// this function can be called by the interrupt routine of any modeul
// to request one of the module functions to be scheduled for execution
void schedule_task(Module *mod, TaskFunct f);

// This is the main loop of the kernel.
// After setup the main program calls this function which then runs in foreground forever.
// All system modules are regularly call by the interrupt system and
// may schedule tasks to be run by the kernel.
extern "C" void kernel_loop();

