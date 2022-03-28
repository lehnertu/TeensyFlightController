#include "serial_usb.h"
#include <Arduino.h>

USB_Serial::USB_Serial(
    std::string name,
    uint32_t baud_rate )
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_INITALIZED;
    // open the connection
    Serial.begin(baud_rate);
    // Serial.println("USB_Serial setup done.");
    flag_text_pending = false;
    flag_telemetry_pending = false;
    // send a message to the system_log
    system_log->system_in.receive(
        Message_System(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

bool USB_Serial::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (text_in.count()>0) flag_text_pending = true;
    if (telemetry_in.count()>0) flag_telemetry_pending = true;
    return flag_text_pending | flag_telemetry_pending;
}

void USB_Serial::run()
{

    // TODO probably we should check the buffer availability

    while (flag_text_pending)
    {
        Message_Text msg = text_in.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        flag_text_pending = (text_in.count()>0);
    }
    
    while (flag_telemetry_pending)
    {
        Message_Telemetry msg = telemetry_in.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        flag_telemetry_pending = (telemetry_in.count()>0);
    }
}

