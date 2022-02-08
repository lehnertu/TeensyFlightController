#include <cstdio>

#include "global.h"
#include "dummy_gps.h"

DummyGPS::DummyGPS(
    std::string name,
    float rate,
    float tm_rate )
{
    // copy the name
    id = name;
    gps_rate = rate;
    telemetry_rate = tm_rate;
    startup_time = FC_systick_millis_count;
    flag_state_change = true;
    flag_update_pending = false;
    last_update = FC_systick_millis_count;
    flag_telemetry_pending = false;
    last_telemetry = FC_systick_millis_count;
    status_lock = false;
    // home position
    lat = 51.04943;
    lon = 13.89053;
    alt = 285.0;
    vx = 0.0;
    vy = 0.0;
    vz = 3.0;
    // we cannot send a status message that we are ready to run
    // because the port is not yet wired to any receiver
}

bool DummyGPS::have_work()
{
    float elapsed = FC_systick_millis_count - last_update;
    flag_update_pending = (elapsed*gps_rate >= 1000.0);
    
    elapsed = FC_systick_millis_count - last_telemetry;
    flag_telemetry_pending = (elapsed*telemetry_rate >= 1000.0);

    return flag_state_change | flag_update_pending | flag_telemetry_pending;
}

#define DEGREE_PER_METER 9e-6
// int32_t random(void);
// delivers positive numbers running 0...2147483647
#define MAX_RANDOM 2147483648.0

void DummyGPS::run()
{
    
    if (flag_update_pending)
    {
        // time in seconds
        float elapsed = 0.001*(FC_systick_millis_count - last_update);
        // velocity damping
        vx -= 0.2 * vx * elapsed;
        vy -= 0.2 * vy * elapsed;
        vz -= 0.5 * vz * elapsed;
        // random velocity change
        vx += 0.5 * elapsed * random()/MAX_RANDOM;
        vy += 0.5 * elapsed * random()/MAX_RANDOM;
        vz += 1.0 * elapsed * random()/MAX_RANDOM;
        // position change
        lat += vy * elapsed * DEGREE_PER_METER;
        lon += vx * elapsed * DEGREE_PER_METER;
        alt += vz * elapsed;

        // TODO: send message
        
        last_update = FC_systick_millis_count;
        flag_update_pending = false;
    };

    if (flag_telemetry_pending)
    {
        char buffer[16];
        int n = snprintf(buffer, 15, "%.6f", lat);
        buffer[n] = '\0';
        tm_out.transmit(
            MESSAGE_TELEMETRY {
                .sender_module = id,
                .variable = "LAT",
                .value = std::string(buffer,n) }
        );

        n = snprintf(buffer, 15, "%.6f", lon);
        buffer[n] = '\0';
        tm_out.transmit(
            MESSAGE_TELEMETRY {
                .sender_module = id,
                .variable = "LON",
                .value = std::string(buffer,n) }
        );

        n = snprintf(buffer, 15, "%.2f", alt);
        buffer[n] = '\0';
        tm_out.transmit(
            MESSAGE_TELEMETRY {
                .sender_module = id,
                .variable = "ALT",
                .value = std::string(buffer,n) }
        );

        last_telemetry = FC_systick_millis_count;
        flag_telemetry_pending = false;
    };

    if (!status_lock)
    {
        if (FC_systick_millis_count - startup_time > 5000)
        {
            status_lock = true;
            flag_state_change = true;
        };
    };
    
    if (flag_state_change)
    {
        if (status_lock)
        {
            system_log.system_in.receive(
                MESSAGE_SYSTEM {
                    .sender_module = id,
                    .severity_level = MSG_LEVEL_STATE_CHANGE,
                    .text="acquired lock." } );
        } else {
            system_log.system_in.receive(
                MESSAGE_SYSTEM {
                    .sender_module = id,
                    .severity_level = MSG_LEVEL_STATE_CHANGE,
                    .text="up and running." } );
        };
        flag_state_change = false;
    };
    
}

