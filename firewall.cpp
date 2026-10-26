
/**
 * @file firewall.cpp
 * @brief Implements IPv4 address parsing and CIDR-based firewall filtering.
 *
 * Provides functionality for adding blocked IPv4 address ranges and
 * checking whether a given IP address matches any blocked range.
 */

#include "firewall.h"
#include <cstdint>
#include <sstream>

namespace
{
    /**
     * @brief Parses an IPv4 address into a 32-bit unsigned integer.
     *
     * Each octet is validated to ensure it contains between one and
     * three decimal digits and has a value between 0 and 255.
     * The four octets are combined into a single 32-bit value.
     *
     * @param ip IPv4 address string in dotted-decimal notation.
     * @param result Reference to the integer that receives the parsed
     *               address when parsing succeeds.
     *
     * @return True if the address is valid; false otherwise.
     *
     * @note The function is restricted to this source file through
     *       the anonymous namespace.
     */
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
     * @brief Determines whether an IPv4 address matches a CIDR range.
     *
     * Accepts either a single IPv4 address or a network address with
     * a prefix length, such as "192.168.1.0/24". If no prefix is
     * supplied, the range is treated as a single address (/32).
     *
     * The function parses both addresses, constructs a subnet mask
     * from the prefix length, and compares their masked values.
     *
     * @param ip IPv4 address to check.
     * @param range IPv4 address or CIDR range to match against.
     *
     * @return True if the IP address belongs to the specified range;
     *         false if it does not match or the input is invalid.
     *
     * @note This helper is restricted to this source file through
     *       the anonymous namespace.
     */
    bool matchesCIDR(const std::string& ip,
                     const std::string& range)
    {
        std::size_t slash = range.find('/');

        std::string network = range.substr(0, slash);
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

        std::uint32_t ipValue, networkValue;

        if (!parseIPv4(ip, ipValue) ||
            !parseIPv4(network, networkValue))
        {
            return false;
        }

        std::uint32_t mask =
            prefix == 0 ? 0u : (0xFFFFFFFFu << (32 - prefix));

        return (ipValue & mask) == (networkValue & mask);
    }
}

/**
 * @brief Adds an IPv4 address or CIDR range to the firewall block list.
 *
 * The supplied range is stored and checked against incoming IP
 * addresses whenever isBlocked() is called.
 *
 * @param range IPv4 address or CIDR range to block.
 *
 * @see Firewall::isBlocked()
 */
void Firewall::addBlockedRange(const std::string& range)
{
    blockedRanges.push_back(range);
}

/**
 * @brief Checks whether an IPv4 address is blocked by the firewall.
 *
 * Iterates through all configured blocked ranges. If the supplied
 * address matches any range, the function immediately returns true.
 *
 * @param ip IPv4 address to check.
 *
 * @return True if the address matches a blocked range; false otherwise.
 *
 * @see Firewall::addBlockedRange()
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
