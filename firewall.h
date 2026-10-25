#pragma once

#include <string>
#include <vector>

class Firewall
{
private:
    std::vector<std::string> blockedRanges;

public:
    void addBlockedRange(const std::string& range);
    bool isBlocked(const std::string& ip) const;
};