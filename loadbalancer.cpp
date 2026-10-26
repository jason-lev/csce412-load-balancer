
/**
 * @file loadbalancer.cpp
 * @brief Implements request distribution and independent server scaling.
 */

#include "loadbalancer.h"

#include <algorithm>
#include <stdexcept>

/**
 * @brief Initializes a dedicated server pool.
 * @param count Initial server count.
 * @param type Job type accepted by the pool ('S' or 'P').
 */
void LoadBalancer::initializeServers(int count, char type)
{
    if (type != 'S' && type != 'P')
        throw std::invalid_argument(
            "LoadBalancer type must be 'S' or 'P'"
        );

    jobType = type;

    queue = RequestQueue{};
    servers.clear();

    clockCycle = 0;
    nextServerId = 1;
    accepted = 0;
    completed = 0;
    peakQueue = 0;
    serversAdded = 0;
    serversRemoved = 0;
    scaleDownDeferred = 0;
    nextScaleCycle = 0;

    initialServers = std::max(1, count);

    for (int i = 0; i < initialServers; ++i)
        servers.emplace_back(nextServerId++);
}

/**
 * @brief Enqueues a request for this pool.
 * @param request Request to enqueue.
 */
void LoadBalancer::addRequest(const Request& request)
{
    if (request.jobType != jobType)
        throw std::invalid_argument(
            "Request routed to the wrong load balancer"
        );

    queue.enqueue(request);
    ++accepted;

    peakQueue = std::max(peakQueue, queue.size());
}

/**
 * @brief Assigns pending requests to available servers.
 *
 * Each server receives at most one request. Requests are removed from
 * the queue in FIFO order.
 */
void LoadBalancer::distributeRequests()
{
    for (WebServer& server : servers)
    {
        if (queue.isEmpty())
            break;

        if (server.isAvailable())
            server.assignRequest(queue.dequeue());
    }
}

/**
 * @brief Advances each server by one simulation cycle.
 * @param cycle Current simulation cycle.
 * @return Number of requests that completed during this cycle.
 */
int LoadBalancer::updateClock(int cycle)
{
    clockCycle = cycle;
    int finished = 0;

    for (WebServer& server : servers)
    {
        if (server.processRequest())
            ++finished;
    }

    completed += finished;
    return finished;
}

/**
 * @brief Scales the pool according to its pending queue size.
 * @param cycle Current simulation cycle.
 * @param cooldown Minimum cycles between scaling actions.
 * @return Description of the scaling action, or an empty string.
 */
std::string LoadBalancer::adjustCapacity(int cycle, int cooldown)
{
    if (cycle < nextScaleCycle || servers.empty())
        return {};

    const std::size_t count = servers.size();
    const int safeCooldown = std::max(1, cooldown);

    // Scale down if the queue has fewer than 50 requests per server.
    if (count > 1 && queue.size() < 50 * count)
    {
        // Only remove an idle server. Never discard an active request.
        if (servers.back().isAvailable())
        {
            const int removedId = servers.back().getServerId();
            servers.pop_back();

            ++serversRemoved;
            nextScaleCycle = cycle + safeCooldown;

            return "Cycle " + std::to_string(cycle) +
                   ": removed server " + std::to_string(removedId) +
                   "; active servers = " +
                   std::to_string(servers.size());
        }

        // Try again after the cooldown rather than removing a busy server.
        ++scaleDownDeferred;
        nextScaleCycle = cycle + safeCooldown;

        // return "Cycle " + std::to_string(cycle) +
            //    ": scale-down deferred; last server is busy";
        return {};
    }

    // Scale up if the queue has more than 80 requests per server.
    if (queue.size() > 80 * count)
    {
        const int addedId = nextServerId++;
        servers.emplace_back(addedId);

        ++serversAdded;
        nextScaleCycle = cycle + safeCooldown;

        return "Cycle " + std::to_string(cycle) +
               ": added server " + std::to_string(addedId) +
               "; active servers = " +
               std::to_string(servers.size());
    }

    return {};
}

/** @brief Returns the job type handled by this pool. */
char LoadBalancer::getJobType() const
{
    return jobType;
}

/** @brief Returns the accepted request count. */
int LoadBalancer::getAccepted() const
{
    return accepted;
}

/** @brief Returns the completed request count. */
int LoadBalancer::getCompleted() const
{
    return completed;
}

/** @brief Returns the number of queued requests. */
std::size_t LoadBalancer::getQueueSize() const
{
    return queue.size();
}

/**
 * @brief Counts servers that are currently busy.
 * @return Number of busy servers.
 */
int LoadBalancer::getBusyServers() const
{
    int count = 0;

    for (const WebServer& server : servers)
    {
        if (!server.isAvailable())
            ++count;
    }

    return count;
}

/**
 * @brief Counts queued and in-flight requests.
 * @return Total number of outstanding requests.
 */
std::size_t LoadBalancer::getOutstandingRequests() const
{
    return queue.size() +
           static_cast<std::size_t>(getBusyServers());
}

/** @brief Returns the peak observed queue size. */
std::size_t LoadBalancer::getPeakQueue() const
{
    return peakQueue;
}

/** @brief Returns the active server count. */
std::size_t LoadBalancer::getServerCount() const
{
    return servers.size();
}

/** @brief Returns the initial server count. */
int LoadBalancer::getInitialServers() const
{
    return initialServers;
}

/** @brief Returns the number of servers added. */
int LoadBalancer::getServersAdded() const
{
    return serversAdded;
}

/** @brief Returns the number of servers removed. */
int LoadBalancer::getServersRemoved() const
{
    return serversRemoved;
}

/** @brief Returns the number of deferred scale-down attempts. */
int LoadBalancer::getScaleDownDeferred() const
{
    return scaleDownDeferred;
}
