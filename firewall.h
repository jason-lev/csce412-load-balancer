#pragma once

/**
 * @file firewall.h
 * @brief Declares the IPv4 range-blocking firewall.
 */

#include <string>
#include <vector>

/**
 * @class Firewall
 * @brief Filters IPv4 addresses using individual addresses and CIDR ranges.
 *
 * Stores blocked address ranges and provides a method for determining
 * whether an IP address matches any of those ranges.
 */
class Firewall
{
private:
    /** List of blocked IPv4 addresses and CIDR ranges. */
    std::vector<std::string> blockedRanges;

public:
    /**
     * @brief Adds an IP address or CIDR range to the block list.
     * @param range IPv4 address or CIDR range, such as "192.168.1.0/24".
     */
    void addBlockedRange(const std::string& range);

    /**
     * @brief Checks whether an IP address is blocked.
     * @param ip IPv4 address to check.
     * @return True if the address matches a blocked range; false otherwise.
     */
    bool isBlocked(const std::string& ip) const;
};