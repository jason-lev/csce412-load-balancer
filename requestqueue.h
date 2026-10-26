/**
 * @file requestqueue.h
 * @brief Declares a FIFO queue for web requests.
 */

#pragma once

#include "request.h"
#include <queue>
#include <cstddef>
#include <stdexcept>

/**
 * @class RequestQueue
 * @brief Stores requests waiting to be assigned to servers.
 */
class RequestQueue
{
private:
    /** FIFO container holding pending requests. */
    std::queue<Request> requests;

public:
    /**
     * @brief Adds a request to the back of the queue.
     * @param request Request to enqueue.
     */
    void enqueue(const Request& request);

    /**
     * @brief Removes and returns the oldest queued request.
     * @throws std::out_of_range If the queue is empty.
     * @return The oldest request.
     */
    Request dequeue();

    /**
     * @brief Checks whether the queue is empty.
     * @return True if empty; otherwise false.
     */
    bool isEmpty() const;

    /**
     * @brief Gets the number of pending requests.
     * @return Number of requests in the queue.
     */
    std::size_t size() const;
};