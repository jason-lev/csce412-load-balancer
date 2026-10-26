/**
 * @file request.cpp
 * @brief Implements request generation.
 */

#include "request.h"
#include "utils.h"

#include <algorithm>

/**
 * @brief Generates randomized IP addresses, job type, and duration.
 * @return A fully initialized request.
 */
Request Request::generateRequest()
{
    Request request;

    request.ipIn = random_ip();
    request.ipOut = random_ip();
    request.jobType = random_job();
    request.time = std::max(1, random_req_time(request.jobType));

    return request;
}