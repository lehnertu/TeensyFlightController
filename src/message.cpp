#include "message.h"
#include <cstdio>
#include <cstdlib> // for C-style memory handling
#include <cstring> // for std::memcpy
// include <iostream> // for std::cout during debugging

Message::Message(
    std::string sender_module,
    uint16_t    msg_type,
    uint16_t    msg_size,
    void*       msg_data)
{
    // std::cout << "Message standard constructor";
    m_sender_module = sender_module;
    m_type = msg_type;
    m_size = msg_size;
    // std::cout << " size=" << m_size << std::endl;
    if (m_size>0)
    {
        m_data = malloc(m_size);
        std::memcpy(m_data, msg_data, m_size);
    }
    else
        m_data = NULL;
}

Message::Message(const Message& other)
{
    // std::cout << "Message COPY constructor";
    m_sender_module = other.m_sender_module;
    m_type = other.m_type;
    m_size = other.m_size;
    // std::cout << " size=" << m_size << std::endl;
    if (m_size>0)
    {
        m_data = malloc(m_size);
        std::memcpy(m_data, other.m_data, m_size);
    }
    else
        m_data = NULL;
}

Message& Message::operator=(const Message& other)
{
    // protct against invalid self-assignment
    if (this != &other)
    {
        m_sender_module = other.m_sender_module;
        m_type = other.m_type;
        m_size = other.m_size;
        // free the old memory
        if (m_size>0) free(m_data);
        // copy the new data
        if (other.m_size>0)
        {
            m_data = malloc(other.m_size);
            std::memcpy(m_data, other.m_data, other.m_size);
        }
        else
            m_data = NULL;
    }
    return *this;
}
 
Message::Message(
    std::string sender_module,
    std::string text)
{
    // std::cout << "MSG_TYPE_TEXT constructor";
    m_sender_module = sender_module;
    m_type = MSG_TYPE_TEXT;
    m_size = sizeof(MSG_DATA_TEXT) + text.size();
    m_data = malloc(m_size);
    // std::cout << " size=" << m_size << std::endl;
    // pointer to the allocated memory
    MSG_DATA_TEXT *d = (MSG_DATA_TEXT *)m_data;
    d->text = text.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<text.size(); i++)
        *t++ = text[i];
}

Message::Message(
    std::string sender_module,
    uint32_t    time,
    uint8_t     severity_level,
    std::string text)
{
    // std::cout << "MSG_TYPE_SYSTEM constructor";
    m_sender_module = sender_module;
    m_type = MSG_TYPE_SYSTEM;
    m_size = sizeof(MSG_DATA_SYSTEM) + text.size();
    m_data = malloc(m_size);
    // std::cout << " size=" << m_size << std::endl;
    // pointer to the allocated memory
    MSG_DATA_SYSTEM *d = (MSG_DATA_SYSTEM *)m_data;
    d->severity_level = severity_level;
    d->time = time;
    d->text = text.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<text.size(); i++)
        *t++ = text[i];
};

Message::Message(
    std::string sender_module,
    uint32_t    time,
    std::string variable,
    std::string value)
{
    // std::cout << "MSG_TYPE_TELEMETRY constructor";
    m_sender_module = sender_module;
    m_type = MSG_TYPE_TELEMETRY;
    m_size = sizeof(MSG_DATA_SYSTEM) + variable.size() + value.size();
    m_data = malloc(m_size);
    // std::cout << " size=" << m_size << std::endl;
    // pointer to the allocated memory
    MSG_DATA_TELEMETRY *d = (MSG_DATA_TELEMETRY *)m_data;
    d->time = time;
    d->variable = variable.size();
    d->value = value.size();
    // point to the rest of the memory block reserved for the string
    // the pointer is advanced by 1x the size of the object
    d++;
    // copy the content of the string
    char *t = (char *)d;
    for (size_t i=0; i<variable.size(); i++)
        *t++ = variable[i];
    for (size_t i=0; i<value.size(); i++)
        *t++ = value[i];
}

Message::~Message()
{
    // std::cout << "Message destructor ";
    /*
    switch (m_type)
        {
            case MSG_TYPE_ABSTRACT:
                std::cout << "ABSTRACT" << std::endl; break;
            case MSG_TYPE_SYSTEM:
                std::cout << "MSG_TYPE_SYSTEM" << std::endl; break;
            case MSG_TYPE_TEXT:
                std::cout << "MSG_TYPE_TEXT" << std::endl; break;
            case MSG_TYPE_TELEMETRY:
                std::cout << "MSG_TYPE_TELEMETRY" << std::endl; break;
            case MSG_TYPE_GPS_POSITION:
                std::cout << "MSG_TYPE_GPS_POSITION" << std::endl; break;
        };
    */
    if (m_size>0) free(m_data);
}

std::string Message::print_content()
{
    // std::cout << "Message::print_content() size=" << m_size << std::endl;
    // TODO: handle all other message types
    std::string ret("");
    switch (m_type)
        {
            case MSG_TYPE_ABSTRACT:
                {
                    break;
                };
            case MSG_TYPE_SYSTEM:
                {
                    // std::cout << "MSG_TYPE_SYSTEM  header=" << sizeof(MSG_DATA_SYSTEM);
                    MSG_DATA_SYSTEM *ptr = (MSG_DATA_SYSTEM *)m_data;
                    char buffer[12];
                    // time
                    int n = snprintf(buffer, 11, "%10.3f", (double)(ptr->time)*0.001);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // severity level
                    n = snprintf(buffer, 5, "%4d", ptr->severity_level);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // this is the number of characters in the text
                    int count = ptr->text;
                    // std::cout << " characters=" << count << std::endl;
                    // advance the pointer beyond the data structure (where the text is)
                    ptr++;
                    char *t = (char *)ptr;
                    // message text
                    for (int i=0; i<count; i++)
                        ret += *t++;
                    break;
                };
            case MSG_TYPE_TEXT:
                {
                    // std::cout << "MSG_TYPE_TEXT  header=" << sizeof(MSG_TYPE_TEXT);
                    // this message contains just one string
                    char* ptr = (char *)m_data;
                    // the pointer initially points to the length byte
                    int count = *ptr++;
                    // std::cout << " characters=" << count << std::endl;
                    // now append all characters
                    for (int i=0; i<count; i++)
                        ret += *ptr++;
                    break;
                };
            case MSG_TYPE_TELEMETRY:
                {
                    // std::cout << "MSG_TYPE_TELEMETRY  header=" << sizeof(MSG_DATA_TELEMETRY);
                    MSG_DATA_TELEMETRY *ptr = (MSG_DATA_TELEMETRY *)m_data;
                    char buffer[12];
                    // time
                    int n = snprintf(buffer, 11, "%10.3f", (double)(ptr->time)*0.001);
                    ret += std::string(buffer,n);
                    // separator
                    ret += std::string(" : ");
                    // size of the text fields
                    int var_count = ptr->variable;
                    int val_count = ptr->value;
                    // advance the pointer beyond the data structure (where the text is)
                    ptr++;
                    char *t = (char *)ptr;
                    // variable name
                    for (int i=0; i<var_count; i++)
                        ret += *t++;
                    for (int i=var_count; i<8; i++)
                        ret += " ";
                    // separator
                    ret += std::string(" : ");
                    // value
                    for (int i=0; i<val_count; i++)
                        ret += *t++;
                    break;
                };
            case MSG_TYPE_GPS_POSITION:
                {
                    // std::cout << "MSG_TYPE_GPS_POSITION  header=" << sizeof(MSG_DATA_GPS_POSITION);
                    MSG_DATA_GPS_POSITION *ptr = (MSG_DATA_GPS_POSITION *)m_data;
                    char buffer[16];
                    // latitude
                    int n = snprintf(buffer, 15, "%10.6f", ptr->latitude);
                    ret += "lat=";
                    ret += std::string(buffer,n);
                    // longitude
                    n = snprintf(buffer, 15, "%11.6f", ptr->longitude);
                    ret += ", long=";
                    ret += std::string(buffer,n);
                    // altitude
                    n = snprintf(buffer, 15, "%7.2f", ptr->altitude);
                    ret += ", alti=";
                    ret += std::string(buffer,n);
                    break;
                };
            default:
                {
                    break;
                };
        };
    return ret;
}

std::string Message::printout()
{
    std::string text = m_sender_module;
    // pad with spaces to 8 characters
    size_t len = text.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        text += space;
    };
    text = text.substr(0, 8);
    // separator
    text += std::string(" : ");
    // message text
    text += print_content();
    return text;
}

Message Message::as_text()
{
    return Message(m_sender_module, print_content());
}

/*

//-----------------------------------------------------------------------------


Message_Text::Message_Text(
    std::string sender_module,
    std::string text) :
    // call the base class contructor
    Message(sender_module)
{
    m_text = text;
}

std::string Message_Text::print_content()
{
    return m_text;
}

//-----------------------------------------------------------------------------

Message_System::Message_System(
    std::string sender_module,
    uint32_t time,
    uint8_t severity_level,
    std::string text) :
    // call the base class contructor
    Message(sender_module)
{
    m_severity_level = severity_level;
    m_time = time;
    m_text = text;
}

std::string Message_System::print_content()
{
    char buffer[12];
    // time
    int n = snprintf(buffer, 11, "%10.3f", (double)m_time*0.001);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // severity level
    n = snprintf(buffer, 5, "%4d", m_severity_level);
    buffer[n] = '\0';
    text += std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // message text
    text += m_text;
    return text;
}

//-----------------------------------------------------------------------------

Message_GPS_position::Message_GPS_position(
    std::string sender_id,
    double  latitude,
    double  longitude,
    float   altitude) :
    // call the base class contructor
    Message(sender_id)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_altitude = altitude;
}

std::string Message_GPS_position::print_content()
{
    char buffer[16];
    // latitude
    int n = snprintf(buffer, 15, "%10.6f", m_latitude);
    buffer[n] = '\0';
    std::string text = "lat=";
    text += std::string(buffer,n);
    // longitude
    n = snprintf(buffer, 15, "%11.6f", m_longitude);
    buffer[n] = '\0';
    text += ", long=";
    text += std::string(buffer,n);
    // altitude
    n = snprintf(buffer, 15, "%7.2f", m_altitude);
    buffer[n] = '\0';
    text += ", alti=";
    text += std::string(buffer,n);
    return text;
}

//-----------------------------------------------------------------------------

Message_Telemetry::Message_Telemetry(
            std::string sender_id,
            uint32_t time,
            std::string variable,
            std::string value) :
    // call the base class contructor
    Message(sender_id)
{
    m_time = time;
    m_variable = variable;
    m_value = value;
}

std::string Message_Telemetry::print_content()
{
    char buffer[12];
    // time
    int n = snprintf(buffer, 11, "%10.3f", (double)m_time*0.001);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // vaiable name
    std::string varname = m_variable;
    // pad with spaces to 8 characters
    size_t len = varname.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        varname += space;
    };
    text += varname.substr(0, 8);
    // separator
    text += std::string(" : ");
    // value
    text += m_value;
    return text;
}

*/
