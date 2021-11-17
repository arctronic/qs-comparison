#include <iostream>
#include <ctime>
#include "simulation.h"
#include "exponential_random_number.h"

int main()
{
    // Simulation parameters
    int number_of_customers, number_of_servers;
    double inter_arrival_time_mean, service_time_mean;

    // Input
    std::cout << "Number of Servers: ";
    std::cin >> number_of_servers;

    std::cout << "Inter Arrival Time Mean: ";
    std::cin >> inter_arrival_time_mean;

    std::cout << "Service Time Mean: ";
    std::cin >> service_time_mean;

    std::cout << "Total Number of Customers: ";
    std::cin >> number_of_customers;

    // Create and run the simulation
    Simulation simulation(number_of_servers, inter_arrival_time_mean, service_time_mean, number_of_customers);
    simulation.Initialize();
    simulation.Run();

    // Create conclusive data from simulation
    simulation.GetSimulationLog()->CreateSimulationSummary();
    simulation.GetSimulationLog()->CreateStatisticalData();

    // Output data to separate files

    // Set decimal precision to 2 places
    std::cout.precision(2);

    // Event records
    freopen("event.csv", "w", stdout);
    std::cout << simulation.GetSimulationLog()->GetEventRecords();
    fclose(stdout);

    // Customer records
    freopen("customer.csv", "w", stdout);
    std::cout << simulation.GetSimulationLog()->GetCustomerRecords();
    fclose(stdout);

    // Statistics records
    
    freopen("statistics.csv", "w", stdout);
    std::cout << simulation.GetSimulationLog()->GetSimulationStatistics();
    fclose(stdout);

    return 0;
}