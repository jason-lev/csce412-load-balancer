
/**
 * @file loadbalancer.h
 * @brief Declares a load balancer dedicated to one request type.
 */

#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include "request.h"
#include "requestqueue.h"
#include "webserver.h"

#include <cstddef>
#include <string>
#include <vector>

/**
 * @class LoadBalancer
 * @brief Manages a request queue and dedicated web server pool.
 *
 * Each instance accepts only one job type:
 * 'S' for streaming or 'P' for processing.
 * Server scaling is performed independently for each instance.
 */
class LoadBalancer
{
private:
    /** Queue of requests awaiting assignment. */
    RequestQueue queue;

    /** Servers belonging exclusively to this pool. */
    std::vector<WebServer> servers;

    /** Job type accepted by this pool. */
    char jobType{'P'};

    /** Most recent simulation cycle. */
    int clockCycle{0};

    /** ID assigned to the next server created. */
    int nextServerId{1};

    /** Total requests accepted by this pool. */
    int accepted{0};

    /** Total requests completed by this pool. */
    int completed{0};

    /** Largest queue size observed. */
    std::size_t peakQueue{0};

    /** Initial number of servers. */
    int initialServers{0};

    /** Number of servers added during scaling. */
    int serversAdded{0};

    /** Number of servers removed during scaling. */
    int serversRemoved{0};

    /** Number of scale-down attempts deferred because a server was busy. */
    int scaleDownDeferred{0};

    /** Earliest cycle at which another scaling action may occur. */
    int nextScaleCycle{0};

public:
    /**
     * @brief Initializes the pool with a job type and server count.
     * @param count Initial number of servers; minimum is one.
     * @param type Accepted job type: 'S' or 'P'.
     * @throws std::invalid_argument If type is neither 'S' nor 'P'.
     */
    void initializeServers(int count, char type);

    /**
     * @brief Adds a request to this pool's queue.
     * @param request Request to enqueue.
     * @throws std::invalid_argument If the request type does not match.
     */
    void addRequest(const Request& request);

    /**
     * @brief Assigns queued requests to available servers.
     */
    void distributeRequests();

    /**
     * @brief Advances all servers by one cycle.
     * @param cycle Current simulation cycle.
     * @return Number of requests completed during this cycle.
     */
    int updateClock(int cycle);

    /**
     * @brief Adjusts server capacity according to queue size.
     *
     * Adds one server if the queue exceeds 80 requests per server.
     * Removes one idle server if the queue falls below 50 requests
     * per server, provided at least one server remains.
     *
     * @param cycle Current simulation cycle.
     * @param cooldown Minimum cycles between scaling actions.
     * @return Event description, or an empty string if no action occurred.
     */
    std::string adjustCapacity(int cycle, int cooldown);

    /** @brief Gets the job type handled by this pool. */
    char getJobType() const;

    /** @brief Gets the number of accepted requests. */
    int getAccepted() const;

    /** @brief Gets the number of completed requests. */
    int getCompleted() const;

    /** @brief Gets the number of requests waiting in the queue. */
    std::size_t getQueueSize() const;

    /** @brief Gets the number of servers currently processing requests. */
    int getBusyServers() const;

    /**
     * @brief Gets the number of outstanding requests.
     * @return Queued requests plus requests being processed.
     */
    std::size_t getOutstandingRequests() const;

    /** @brief Gets the maximum observed queue size. */
    std::size_t getPeakQueue() const;

    /** @brief Gets the current number of active servers. */
    std::size_t getServerCount() const;

    /** @brief Gets the initial server count. */
    int getInitialServers() const;

    /** @brief Gets the number of servers added. */
    int getServersAdded() const;

    /** @brief Gets the number of servers removed. */
    int getServersRemoved() const;

    /** @brief Gets the number of deferred scale-down attempts. */
    int getScaleDownDeferred() const;
};

#endif
