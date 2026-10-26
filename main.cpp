/**
 * @file main.cpp
 * @brief Application entry point for the load balancer simulation.
 */

#include "masterloadbalancer.h"
#include "utils.h"

/**
 * @brief Initializes random generation and launches the master balancer.
 * @return Zero on normal termination.
 */
int main()
{
    init_random();

    MasterLoadBalancer master;
    master.runSimulation();

    return 0;
}