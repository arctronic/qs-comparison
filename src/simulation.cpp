#include <queue>
#include <limits>
#include <vector>

#include "simulation.h"
#include "simulation_log.h"
#include "event.h"
#include "customer.h"
#include "server.h"
#include "exponential_random_number.h"

Simulation::Simulation(int number_of_servers, double inter_arrival_time_mean, double service_time_mean, int number_of_customers) : simulation_log(number_of_servers)
{
    this->number_of_servers = number_of_servers;
    this->inter_arrival_time_mean = inter_arrival_time_mean;
    this->service_time_mean = service_time_mean;
    this->number_of_customers = number_of_customers;

    // Set up the random number generators
    this->inter_arrival_time_generator.SetMean(this->inter_arrival_time_mean);
    this->service_time_generator.SetMean(this->service_time_mean);
}

void Simulation::UpdateTime(double time) { this->clock = time; }

void Simulation::Initialize()
{
    // Set clock time to 0 and server status to Idle
    this->clock = 0;

    // Set all server status as idle
    for (int i = 0; i < this->number_of_servers; i++)
    {
        servers.push_back(Server());
    }

    // Schedule the first arrival event
    Event arrival_event(EventType::ARRIVAL, this->clock + this->inter_arrival_time_generator.GetRandomNumber());
    this->service_queues.resize(this->number_of_servers);
    this->event_queue.push(arrival_event);
}

void Simulation::Run()
{
    // Run simulation till event queue is empty
    while (!event_queue.empty())
    {
        // Fetch current event from the queue and update simulation clock
        Event current_event = this->event_queue.top();
        this->event_queue.pop();
        UpdateTime(current_event.GetInvokeTime());

        // Handle the event
        if (current_event.GetType() == EventType::ARRIVAL)
        {
            HandleArrival();
        }
        else if (current_event.GetType() == EventType::DEPARTURE)
        {
            HandleDeparture(current_event.GetTargetServer());
        }
    }
}

void Simulation::HandleArrival()
{
    // Create a new Customer with its arrival time
    Customer customer(this->clock);

    // Schedule next arrival event if customer limit is not crossed
    if (this->number_of_customers > Customer::GetTotalCustomers())
    {
        Event arrival_event(EventType::ARRIVAL, this->clock + this->inter_arrival_time_generator.GetRandomNumber());
        this->event_queue.push(arrival_event);
    }

    // Find the availble server
    int available_server_index = this->GetAvailableServerIndex();

    // If there are no server availble
    if (available_server_index == -1)
    {
        int queue_no = this->GetSmallestQueue();
        // Send customer to the queue
        this->service_queues[queue_no].push(customer);

        // Log the arrival event
        CreateArrivalLog(customer);
    }
    // If a server is availble, send customer to service
    else
    {
        // Log the arrival event
        CreateArrivalLog(customer);

        // Send the customer to server
        customer.SetServer(available_server_index);
        customer.SetServiceStartTime(this->clock);
        this->servers[available_server_index].SetCurrentCustomer(customer);

        // Set server status to Busy
        this->servers[available_server_index].SetServerStatus(ServerStatus::BUSY);

        // Schedule the departure event (end of service)
        Event departure_event(EventType::DEPARTURE, this->clock + this->service_time_generator.GetRandomNumber(), available_server_index);
        this->event_queue.push(departure_event);

        // Log the service event
        CreateServiceLog(available_server_index);
    }
}

void Simulation::HandleDeparture(int target_server_index)
{
    // Set server status to Idle
    this->servers[target_server_index].SetServerStatus(ServerStatus::IDLE);

    // Log the departure event and empty the server
    CreateDepartureLog(target_server_index);

    // If there are customers in queue, decrease queue length and schedule service time
    for (int i = 0; i < this->service_queues.size(); i++)
    {
        if (!(this->service_queues[i].empty()))
        {
            // Fetch first customer from the queue and send him to server
            Customer customer = service_queues[i].front();
            customer.SetServer(target_server_index);
            customer.SetServiceStartTime(this->clock);
            this->servers[target_server_index].SetCurrentCustomer(customer);

            this->service_queues[i].pop();

            // Set server to busy state
            this->servers[target_server_index].SetServerStatus(ServerStatus::BUSY);

            // Schedule the departure event (end of service)
            Event departure_event(EventType::DEPARTURE, this->clock + this->service_time_generator.GetRandomNumber(), target_server_index);
            this->event_queue.push(departure_event);

            // Log the service event
            CreateServiceLog(target_server_index);
        }
    }
}

void Simulation::CreateArrivalLog(Customer customer)
{
    std::vector<int> sizes;
    for (int i = 0; i < this->service_queues.size(); i++)
    {
        sizes.push_back(this->service_queues[i].size());
    }
    this->simulation_log.CreateEventRecord("Arrival", this->clock, customer.GetSerial(), sizes, -1);
}

void Simulation::CreateServiceLog(int server_index)
{
    std::vector<int> sizes;
    for (int i = 0; i < this->service_queues.size(); i++)
    {
        sizes.push_back(this->service_queues[i].size());
    }

    Customer currently_served_customer = this->servers[server_index].GetCurrentCustomer();

    this->simulation_log.CreateEventRecord("Service", this->clock, currently_served_customer.GetSerial(), sizes, currently_served_customer.GetServer());
}

void Simulation ::CreateDepartureLog(int server_index)
{
    Customer currently_served_customer = this->servers[server_index].GetCurrentCustomer();
    std::vector<int> sizes;
    for (int i = 0; i < this->service_queues.size(); i++)
    {
        sizes.push_back(this->service_queues[i].size());
    }
    this->simulation_log.CreateEventRecord("Departure", this->clock, currently_served_customer.GetSerial(), sizes, currently_served_customer.GetServer());
    currently_served_customer.SetDepartureTime(this->clock);
    this->simulation_log.CreateCustomerRecord(currently_served_customer);
}

SimulationLog *Simulation::GetSimulationLog()
{
    return &(this->simulation_log);
}

int Simulation::GetAvailableServerIndex()
{
    // Check the first available server
    for (int i = 0; i < this->number_of_servers; i++)
        if (servers[i].GetServerStatus() == ServerStatus::IDLE)
            return i;

    // No server is available
    return -1;
}

int Simulation::GetSmallestQueue()
{
    int minIdx = 0;
    int min = 0;
    for (int i = 0; i < this->service_queues.size(); i++)
    {
        if (this->service_queues[i].size() == 0)
        {
            return i;
        }
        if (this->service_queues[i].size() < min)
        {
            min = this->service_queues[i].size();
            minIdx = i;
        }
    }
    return minIdx;
}