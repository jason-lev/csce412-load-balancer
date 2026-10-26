/**
 * @file utils.cpp
 * @brief Implements random generation utilities.
 */

#include "utils.h"

#include <iostream>
#include <random>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    /** Random device used to seed the generator. */
    std::random_device randomDevice;

    /** Mersenne Twister engine used by all utility functions. */
    std::mt19937 randomEngine;

    /**
     * @brief Generates a uniformly distributed integer.
     * @param minimum Inclusive lower bound.
     * @param maximum Inclusive upper bound.
     * @return Random integer in the requested range.
     */
    int randomInt(int minimum, int maximum)
    {
        return std::uniform_int_distribution<int>(
            minimum, maximum
        )(randomEngine);
    }
}

/** Local copy of the min task time. **/
int min_task_time = 50;

/** Local copy of the max task time. **/
int max_task_time = 550;

/**
 * @brief Seeds the random number generator and saves local copies of the min/max task time..
 */

void init_random()
{
    randomEngine.seed(randomDevice());
    // std::cout << "Random generator initialized.\n";

    std::ifstream config("config.txt");
    std::string line;

    while (std::getline(config, line))
    {
        trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        const std::size_t equals = line.find('=');

        if (equals == std::string::npos)
            continue;

        std::string key = line.substr(0, equals);
        std::string value = line.substr(equals + 1);

        trim(key);
        trim(value);

        try
        {
            if (key == "min_task_time")
                min_task_time = std::stoi(value);
            else if (key == "max_task_time")
                max_task_time = std::stoi(value);
        }
        catch (const std::exception&)
        {
            std::cerr << "Invalid value for configuration key "
                      << key << "; keeping its default.\n";
        }
    }
}

/**
 * @brief Generates a valid random IPv4 address.
 * @return IPv4 address with four octets in the range 0-255.
 */
std::string random_ip()
{
    return std::to_string(randomInt(1, 223)) + "." +
           std::to_string(randomInt(0, 255)) + "." +
           std::to_string(randomInt(0, 255)) + "." +
           std::to_string(randomInt(1, 254));
}

/**
 * @brief Randomly chooses a streaming or processing job.
 * @return 'S' or 'P'.
 */
char random_job()
{
    return randomInt(0, 1) == 0 ? 'S' : 'P';
}

/**
 * @brief Generates a random request duration.
 * @param job Type of job being generated.
 * @return Duration between 50 and 549 clock cycles.
 *
 * @note This implementation uses the same duration range for both
 * job types. Adjust this function if streaming and processing
 * requests should have different durations.
 */
int random_req_time(char job)
{
    (void)job;
    return randomInt(min_task_time, max_task_time);
}

/**
 * @brief Generates a random percentage value.
 * @return Integer from 0 through 99.
 */
int random_percent()
{
    return randomInt(0, 99);
}

/**
    * @brief Trims leading and trailing whitespace from a string.
    * @param value String to modify.
*/
void trim(std::string& value)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t first =
        value.find_first_not_of(whitespace);

    if (first == std::string::npos)
    {
        value.clear();
        return;
    }

    const std::size_t last =
        value.find_last_not_of(whitespace);

    value = value.substr(first, last - first + 1);
};