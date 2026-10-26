/**
 * @file masterloadbalancer.h
 * @brief Declares the master load balancer and request switch.
 */

#pragma once

#include "firewall.h"
#include "loadbalancer.h"
#include "request.h"

#include <string>

/**
 * @class MasterLoadBalancer
 * @brief Routes requests to isolated streaming and processing pools.
 *
 * Owns the simulation clock and firewall. Child load balancers maintain
 * separate queues, servers, completion counters, and scaling state.
 */
class MasterLoadBalancer
{
private:
    /** Dedicated pool for streaming requests. */
    LoadBalancer streamingBalancer;

    /** Dedicated pool for processing requests. */
    LoadBalancer processingBalancer;

    /** Firewall applied before routing. */
    Firewall firewall;

    /** Total requests generated during the run. */
    int generated{0};

    /** Total requests rejected by the firewall. */
    int blocked{0};

    /** Requests with unsupported job types. */
    int invalidRequests{0};

    /** Current simulation cycle. */
    int clockCycle{0};

    /** Controls verbose or quiet output. */
    std::string loggingMode{"verbose"};

    /** Controls console output. */
    std::string printingMode{"off"};

    /**
     * @brief Routes a request after checking the firewall.
     * @param request Request to route.
     * @return Event description for a rejected request; empty string
     *         when the request is accepted by a child pool.
     */
    std::string routeRequest(const Request& request);

public:
    /**
     * @brief Loads settings and executes the complete simulation.
     */
    void runSimulation();
};