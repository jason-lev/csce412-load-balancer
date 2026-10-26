/**
 * @file webserver.cpp
 * @brief Implements web server request processing.
 */

#include "webserver.h"

#include <algorithm>

/**
 * @brief Constructs a server in the idle state.
 * @param id Identifier for this server.
 */
WebServer::WebServer(int id)
    : serverId(id),
      busy(false),
      currentRequest{},
      remainingTime(0)
{
}

/**
 * @brief Assigns a request if the server is available.
 * @param request Request to process.
 * @return True if assigned successfully; otherwise false.
 */
bool WebServer::assignRequest(const Request& request)
{
    if (!isAvailable())
        return false;

    currentRequest = request;
    remainingTime = std::max(1, request.time);
    busy = true;

    return true;
}

/**
 * @brief Processes one clock cycle of the active request.
 *
 * A completed request makes the server idle and produces exactly
 * one completion event.
 *
 * @return True if the request completed; otherwise false.
 */
bool WebServer::processRequest()
{
    if (isAvailable())
        return false;

    --remainingTime;

    if (remainingTime <= 0)
    {
        remainingTime = 0;
        busy = false;
        return true;
    }

    return false;
}

/**
 * @brief Checks whether the server can accept a request.
 * @return True if idle; otherwise false.
 */
bool WebServer::isAvailable() const
{
    return !busy;
}

/**
 * @brief Returns the server identifier.
 * @return Server ID.
 */
int WebServer::getServerId() const
{
    return serverId;
}