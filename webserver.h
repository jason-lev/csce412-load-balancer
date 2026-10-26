/**
 * @file webserver.h
 * @brief Declares the simulated web server.
 */

#pragma once

#include "request.h"

/**
 * @class WebServer
 * @brief Processes one request at a time.
 *
 * A server begins idle. Each processing call advances the active
 * request by one cycle.
 */
class WebServer
{
private:
    /** Unique identifier within its load balancer. */
    int serverId{0};

    /** True when a request is being processed. */
    bool busy{false};

    /** Request currently assigned to the server. */
    Request currentRequest{};

    /** Remaining processing cycles. */
    int remainingTime{0};

public:
    /**
     * @brief Constructs an idle server.
     * @param id Server identifier.
     */
    explicit WebServer(int id = 0);

    /**
     * @brief Assigns a request to an idle server.
     * @param request Request to process.
     * @return True if assigned; false if the server is busy.
     */
    bool assignRequest(const Request& request);

    /**
     * @brief Advances request processing by one clock cycle.
     * @return True exactly when a request completes during this call.
     */
    bool processRequest();

    /**
     * @brief Checks whether the server is idle.
     * @return True if available; otherwise false.
     */
    bool isAvailable() const;

    /**
     * @brief Gets this server's identifier.
     * @return Server ID.
     */
    int getServerId() const;
};