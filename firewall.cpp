#include "firewall.h"
#include <cstdint>
#include <sstream>

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

void Firewall::addBlockedRange(const std::string& range)
{
    blockedRanges.push_back(range);
}

bool Firewall::isBlocked(const std::string& ip) const
{
    for (const std::string& range : blockedRanges)
    {
        if (matchesCIDR(ip, range))
            return true;
    }

    return false;
}