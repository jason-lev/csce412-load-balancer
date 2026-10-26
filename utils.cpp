
/**
 * @file utils.cpp
 * @brief Implements utility functions for random request generation.
 *
 * Provides random number initialization and helper functions for
 * generating IP addresses, job types, request processing times,
 * and percentage values for the load balancer simulation.
 */

#include "utils.h"
#include <iostream>

/**
 * @brief Source of nondeterministic random seed values.
 *
 * Used to seed the Mersenne Twister random number generator.
 */
std::random_device rnd_device;

/**
 * @brief Mersenne Twister random number generator.
 *
 * Supplies random values used by the simulation utility functions.
 * Initialize this generator by calling init_random() before using
 * the other random utility functions.
 */
std::mt19937 mersenne_engine;

/**
 * @brief Initializes the random number generator.
 *
 * Seeds the Mersenne Twister engine using a value obtained from
 * std::random_device and prints an initialization message.
 *
 * @note Call this function once before generating random values.
 */
void init_random()
{
    mersenne_engine = std::mt19937(rnd_device());
    std::cout << "Random initialized! Value: "
              << mersenne_engine() << std::endl;
}

/**
 * @brief Generates a randomized dotted-decimal IP-like address.
 *
 * Generates four numeric components and joins them with periods.
 * The first component is regenerated if it equals 10, 172, or 192.
 *
 * @return A string containing four dot-separated numeric components.
 *
 * @warning The generated components range from 0 to 999, so this
 * function does not guarantee a valid IPv4 address. Valid IPv4
 * octets must range from 0 to 255.
 */
std::string random_ip()
{
    int start;

    do
    {
        start = mersenne_engine() % 256;
    }
    while (start == 10 || start == 192 || start == 172);

    std::string ip =
        std::to_string(start) + "." +
        std::to_string(mersenne_engine() % 256) + "." +
        std::to_string(mersenne_engine() % 256) + "." +
        std::to_string(mersenne_engine() % 256);

    return ip;
}

/**
 * @brief Generates a random request job type.
 *
 * Randomly selects one of two job types:
 * - 'P'
 * - 'S'
 *
 * @return 'P' or 'S', selected using the random number generator.
 */
char random_job()
{
    return (mersenne_engine() % 2 == 0) ? 'P' : 'S';
}

/**
 * @brief Generates a random request processing duration.
 *
 * Produces a duration between 50 and 549 clock cycles, inclusive.
 *
 * @param job Job type associated with the request.
 *
 * @return Random processing duration in clock cycles.
 *
 * @note The current implementation does not use the job parameter
 * to influence processing duration.
 */
int random_req_time(char job)
{
    return 50 + (mersenne_engine() % 500);
}

/**
 * @brief Generates a random integer representing a percentage.
 *
 * Produces a value from 0 through 99, inclusive. This can be used
 * to determine whether a probabilistic event should occur.
 *
 * @return Random integer in the range [0, 99].
 */
int random_percent()
{
    return (mersenne_engine() % 100);
}
