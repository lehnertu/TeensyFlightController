#include <cstdlib>
#include <cstdint>
#include <list>

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h

#include "global.h"
#include "display.h"
#include "module.h"
#include "message.h"
#include "system.h"

/*
    This is a task descriptor
    TODO: this needs extensions: priority, maybe different entry points
*/
struct Task 
{
    // the module which has started this task
    Module* module;
    // the system time at which the task has been started (FC_systick_millis_count)
    uint32_t schedule_time;
};

extern "C" int main(void)
{

    // all modules are registered in a list
    std::list<Module*> module_list;
    
    // all tasks that have been scheduled for execution
    std::list<Task> task_list;

    // the logger has to be added to the list of modules so it will be scheduled for execution
    module_list.push_back(&system_log);
    system_log.system_in.receive(
        MESSAGE_SYSTEM {
            .sender_module = "SYSTEM",
            .severity_level = MSG_LEVEL_MILESTONE,
            .text="Teensy Flight Controller - Version 1.0" } );
    
    // now create and wire all modules and add them to the list
    // all extended initializations are not yet done but
    // have to be handled my the modules as tasks
    FC_build_system(&module_list);

    // Here the core hardware is initialized.
    // The millisecond systick interrupt is bent to out own ISR.
    setup_core_system();
    
    system_log.system_in.receive(
        MESSAGE_SYSTEM {
            .sender_module = "SYSTEM",
            .severity_level = MSG_LEVEL_MILESTONE,
            .text="entering event loop." } );

    // we keep track of the task completion time
    FC_max_time_to_completion = 0;

	// infinite system loop
	while (1)
	{
	
	    // systick interrupt has occured - run scheduler
	    // but wait for the tasklist to be completed
	    if (FC_systick_flag>0 and task_list.empty())
	    {
	        // a value greater than one indicates that one systick has been skipped
	        // which should be reported as a major instability incident
	        if (FC_systick_flag>1)
	        {
                system_log.system_in.receive(
                    MESSAGE_SYSTEM {
                        .sender_module = "SYSTEM",
                        .severity_level = MSG_LEVEL_CRITICAL,
                        .text="systick overrun" } );
	        };
	        // reset the flag
	        FC_systick_flag=0;
	        
	        // SCHEDULER:
	        // The scheduler runs through the complete list of modules.
	        // All modules which have work to do get a task scheduled
	        // at appropriate position (priority) in the (ordered) task list.
	        std::list<Module*>::iterator it;
	        for (it = module_list.begin(); it != module_list.end(); it++)
	        {
	            Module* mod = *it;
	            if (mod->have_work())
	            {
	                Task task = {
	                    .module = mod,
	                    .schedule_time = FC_systick_millis_count
	                    };
	                task_list.push_back(task);
	                // Serial.print("scheduling Module at ");
	                // Serial.println((uint64_t)task.module);
	            }
	        };
	        
	    };
	    
	    // TASKMANAGER:
	    // All scheduled tasks get executed based on priority (list position).
        if (!task_list.empty())
        {
            // get the first entry in the task list
            Task task = task_list.front();
            // remove it from the list
            task_list.pop_front();
            // Serial.print("calling Module at ");
            // Serial.println((uint64_t)task.module);
            // execute the task
            task.module->run();
            // if the tasklist is empty now, we store the time is took to complete
            if (task_list.empty())
            {
                // TODO : prevent overruns
                FC_time_to_completion = ARM_DWT_CYCCNT - FC_systick_cycle_count;
                // is is reset when an output is created (either log or display)
                if (FC_time_to_completion > FC_max_time_to_completion)
                    FC_max_time_to_completion = FC_time_to_completion;
            };
        }

	} // infinite system loop

}

