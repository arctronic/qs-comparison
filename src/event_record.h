#ifndef EVENT_RECORD_H
#define EVENT_RECORD_H

#include "server.h"
#include <string>
#include<vector>
class EventRecord
{
private:
    std::string event_type;
    double current_simulation_time;
    int customer_serial;
    std::vector<int> current_queue_size;
    int server_index;

public:
    EventRecord();

    EventRecord(std::string event_type, double current_simulation_time, int customer_serial, std::vector<int> current_queue_size, int server_index);

    friend class SimulationLog;
};

#endif