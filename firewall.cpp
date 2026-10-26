/**
 * @file firewall.cpp
 * @brief Implements IPv4 and CIDR matching for the firewall.
 */

#include "firewall.h"

#include <cstdint>
#include <sstream>
#include <string>

/**
 * @brief Parses an IPv4 dotted-decimal address.
 * @param ip Address string to parse.
 * @param result Receives the 32-bit address value on success.
 * @return True for a valid IPv4 address; otherwise false.
 *
 * This helper is private to this source file.
 */
namespace
{
    bool parseIPv4(const std::string& ip, std::uint32_t& result)
    {
        std::istringstream input(ip);
        std::string part;
        result = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (!std::getline(input, part, '.'))
                return false;

            if (part.empty() || part.size() > 3)
                return false;

            unsigned int octet = 0;

            for (char c : part)
            {
                if (c < '0' || c > '9')
                    return false;

                octet = octet * 10 + (c - '0');

                if (octet > 255)
                    return false;
            }

            result = (result << 8) | octet;
        }

        return !std::getline(input, part, '.');
    }

    /**
     * @brief Checks whether an address belongs to a CIDR range.
     * @param ip Address being checked.
     * @param range Address or CIDR range to compare against.
     * @return True if the address matches the range.
     */
    bool matchesCIDR(const std::string& ip, const std::string& range)
    {
        const std::size_t slash = range.find('/');
        const std::string network = range.substr(0, slash);
        int prefix = 32;

        if (slash != std::string::npos)
        {
            try
            {
                std::size_t used = 0;
                prefix = std::stoi(range.substr(slash + 1), &used);

                if (used != range.size() - slash - 1)
                    return false;
            }
            catch (...)
            {
                return false;
            }
        }

        if (prefix < 0 || prefix > 32)
            return false;

        std::uint32_t ipValue = 0;
        std::uint32_t networkValue = 0;

        if (!parseIPv4(ip, ipValue) ||
            !parseIPv4(network, networkValue))
            return false;

        const std::uint32_t mask =
            prefix == 0 ? 0u : (0xFFFFFFFFu << (32 - prefix));

        return (ipValue & mask) == (networkValue & mask);
    }
}

/**
 * @brief Adds an IPv4 address or CIDR range to the firewall.
 * @param range Range to block.
 */
void Firewall::addBlockedRange(const std::string& range)
{
    blockedRanges.push_back(range);
}

/**
 * @brief Checks an address against every configured blocked range.
 * @param ip Address to check.
 * @return True if any range matches; otherwise false.
 */
bool Firewall::isBlocked(const std::string& ip) const
{
    for (const std::string& range : blockedRanges)
    {
        if (matchesCIDR(ip, range))
            return true;
    }

    return false;
}