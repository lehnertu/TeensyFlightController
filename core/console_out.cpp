#include "console_out.h"
#include <iostream>

Console_out::Console_out(std::string name)
{
    // copy the name
    id = name;
    flag_text_pending = false;
    flag_telemetry_pending = false;
    // send a message to the system_log
    system_log.system_in.receive(
        Message_System(id, MSG_LEVEL_STATE_CHANGE, "setup done.") );
}

bool Console_out::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (text_in.count()>0) flag_text_pending = true;
    if (telemetry_in.count()>0) flag_telemetry_pending = true;
    return flag_text_pending | flag_telemetry_pending;
}

void Console_out::run()
{

    while (flag_text_pending)
    {
        Message_Text msg = text_in.fetch();
        std::string buffer = msg.serialize();
        // write out
        std::cout << buffer << std::endl;
        flag_text_pending = (text_in.count()>0);
    }
    
    while (flag_telemetry_pending)
    {
        Message_Telemetry msg = telemetry_in.fetch();
        std::string buffer = msg.serialize();
        // write out
        std::cout << buffer << std::endl;
        flag_telemetry_pending = (telemetry_in.count()>0);
    }
}

