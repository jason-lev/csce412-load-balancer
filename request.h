#pragma once

/**
 * @file request.h
 * @brief Declares the Request class.
 */

#include <string>

/**
 * @class Request
 * @brief Represents a single incoming web request.
 *
 * Job type 'S' represents streaming, and 'P' represents processing.
 */
class Request
{
public:
    /** Source IPv4 address. */
    std::string ipIn{};

    /** Destination IPv4 address. */
    std::string ipOut{};

    /** Request type: 'S' for streaming or 'P' for processing. */
    char jobType{'P'};

    /** Processing duration in clock cycles. */
    int time{1};

    /**
     * @brief Generates a randomized request.
     * @return A newly generated request.
     */
    Request generateRequest();
};