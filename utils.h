/**
 * @file utils.h
 * @brief Declares random generation utilities for the simulation.
 */

#pragma once

#include <string>

/**
 * @brief Initializes the random number generator.
 */
void init_random();

/**
 * @brief Generates a random IPv4 address.
 * @return A valid dotted-decimal IPv4 address.
 */
std::string random_ip();

/**
 * @brief Generates a random job type.
 * @return 'S' for streaming or 'P' for processing.
 */
char random_job();

/**
 * @brief Generates a processing duration.
 * @param job Job type associated with the request.
 * @return Duration in clock cycles.
 */
int random_req_time(char job);

/**
 * @brief Generates a random integer from 0 through 99.
 * @return Random percentage value.
 */
int random_percent();

/**
    * @brief Trims leading and trailing whitespace from a string.
    * @param value String to modify.
*/
void trim(std::string& value);