/**
 * @file requestqueue.cpp
 * @brief Implements FIFO request queue operations.
 */

#include "requestqueue.h"

/**
 * @brief Enqueues a copy of a request.
 * @param request Request to add.
 */
void RequestQueue::enqueue(const Request& request)
{
    requests.push(request);
}

/**
 * @brief Removes and returns the oldest request.
 * @throws std::out_of_range If there are no queued requests.
 * @return The removed request.
 */
Request RequestQueue::dequeue()
{
    if (requests.empty())
        throw std::out_of_range("Cannot dequeue an empty request queue");

    Request request = requests.front();
    requests.pop();

    return request;
}

/**
 * @brief Checks whether the queue contains no requests.
 * @return True if empty; otherwise false.
 */
bool RequestQueue::isEmpty() const
{
    return requests.empty();
}

/**
 * @brief Returns the current queue length.
 * @return Number of queued requests.
 */
std::size_t RequestQueue::size() const
{
    return requests.size();
}