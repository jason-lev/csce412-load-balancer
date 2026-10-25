#include <iostream>
#include "utils.h"
#include "firewall.h"
#include <queue>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


class Request
{
    public:
    std::string ipIn;
    std::string ipOut;
    char jobType;
    int time;

    Request generateRequest() {
        Request r;
        r.ipIn  = random_ip();
        r.ipOut = random_ip();
        jobType = random_job();
        time = random_req_time(jobType);
        return r;
    }
};

class RequestQueue
{
    private:
        std::queue<Request> requests;
    public:
    void enqueue(Request r) {
        requests.push(r);
    }
    Request dequeue() {
        if (requests.empty())
            throw std::out_of_range("Cannot dequeue an empty request queue");
        Request r = requests.front();
        requests.pop();
        return r;
    }
    bool isEmpty() {
        return requests.empty();
    }
    size_t size() {
        return requests.size();
    }
};

class WebServer
{
    private:
        int serverId;
        bool busy;
        Request currentRequest;
        int remainingTime;

    public:
        void assignRequest(Request r) {
            if (!isAvailable())
                return;

            currentRequest = r;
            remainingTime = r.time > 0 ? r.time : 1;
            busy = true;
        }
        void processRequest() {
            if (isAvailable())
                return;

            --remainingTime;

            if (remainingTime <= 0)
            {
                remainingTime = 0;
                busy = false;
            }
        }
        bool isAvailable() {
            return !busy;
        }
};

class LoadBalancer
{
    private:
        RequestQueue queue;
        int clockCycle;
        std::vector<WebServer> servers;
    public:
        void addRequest(Request r) {
            queue.enqueue(r);
        }
        void distributeRequests() {
            for (WebServer& server : servers)
            {
                if (queue.isEmpty())
                    break;

                if (server.isAvailable())
                    server.assignRequest(queue.dequeue());
            }
        }
        void updateClock() {
            for (WebServer& server : servers)
                server.processRequest();

            ++clockCycle;
        }
        void runSimulation() {
            int defaultServers = 5;
            int defaultCycles = 1000;
            int arrivalPercent = 10;
            int scaleCooldown = 10;

            std::vector<std::string> blockedRanges;
            std::string loggingMode = "verbose";

            std::ifstream config("config.txt");
            std::string line;

            while (std::getline(config, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                std::size_t equals = line.find('=');

                if (equals == std::string::npos)
                    continue;

                std::string key = line.substr(0, equals);
                std::string value = line.substr(equals + 1);

                auto trim = [](std::string& s)
                {
                    const std::string whitespace = " \t\r\n";
                    std::size_t first = s.find_first_not_of(whitespace);

                    if (first == std::string::npos)
                    {
                        s.clear();
                        return;
                    }

                    std::size_t last = s.find_last_not_of(whitespace);
                    s = s.substr(first, last - first + 1);
                };

                trim(key);
                trim(value);

                try
                {
                    if (key == "initial_servers")
                        defaultServers = std::stoi(value);
                    else if (key == "simulation_cycles")
                        defaultCycles = std::stoi(value);
                    else if (key == "arrival_percent")
                        arrivalPercent = std::stoi(value);
                    else if (key == "scale_cooldown")
                        scaleCooldown = std::stoi(value);
                    else if (key == "blocked_ranges")
                    {
                        std::stringstream ranges(value);
                        std::string range;

                        while (std::getline(ranges, range, ','))
                        {
                            trim(range);

                            if (!range.empty())
                                blockedRanges.push_back(range);
                        }
                    }
                    else if (key == "logging_mode")
                    {
                        if (value == "verbose" || value == "quiet")
                            loggingMode = value;
                        else
                            std::cerr << "Invalid logging_mode; using verbose.\n";
                    }
                }
                catch (...)
                {
                    std::cerr << "Invalid config value for " << key
                            << "; using the previous default.\n";
                }
            }

            defaultServers = std::max(1, defaultServers);
            defaultCycles = std::max(1, defaultCycles);
            arrivalPercent = std::max(0, std::min(100, arrivalPercent));
            scaleCooldown = std::max(1, scaleCooldown);

            int initialServerCount = defaultServers;
            int duration = defaultCycles;

            std::cout << "Number of web servers (default "
                    << defaultServers << "): ";

            if (!(std::cin >> initialServerCount))
            {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                initialServerCount = defaultServers;
            }

            std::cout << "Simulation duration in clock cycles (default "
                    << defaultCycles << "): ";

            if (!(std::cin >> duration))
            {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                duration = defaultCycles;
            }

            initialServerCount = std::max(1, initialServerCount);
            duration = std::max(1, duration);

            servers.clear();
            servers.resize(initialServerCount);

            Firewall firewall;

            for (const std::string& range : blockedRanges)
                firewall.addBlockedRange(range);

            std::ofstream log("loadbalancer.log");

            if (!log)
                std::cerr << "Warning: Could not open loadbalancer.log\n";

            auto report = [&](const std::string& message)
            {
                std::cout << message << '\n';

                if (log)
                    log << message << '\n';
            };

            auto logEvent = [&](const std::string& message)
            {
                if (loggingMode == "verbose")
                {
                    if (log)
                        log << message << '\n';

                    std::cout << message << '\n';
                }
            };

            int generated = 0;
            int blocked = 0;
            int completed = 0;
            int accepted = 0;
            size_t peakQueue = 0;
            int nextScaleCycle = 0;
            int initialQueueSize = initialServerCount * 100;

            int serversAdded = 0;
            int serversRemoved = 0;
            int scaleDownDeferred = 0;

            report("=== Load Balancer Simulation ===");
            report("Logging mode: " + loggingMode);
            report("Initial servers: " + std::to_string(initialServerCount));
            report("Runtime cycles: " + std::to_string(duration));
            report("Initial queue target: " +
                std::to_string(initialQueueSize));

            // Generate the initial queue.
            Request generator;

            for (int i = 0; i < initialQueueSize; ++i)
            {
                Request r = generator.generateRequest();
                ++generated;

                if (firewall.isBlocked(r.ipIn))
                {
                    ++blocked;
                    logEvent("Firewall blocked request from " + r.ipIn);
                    continue;
                }

                addRequest(r);
                ++accepted;
            }

            peakQueue = queue.size();

            report("Initial requests accepted: " +
                std::to_string(accepted));
            report("Initial requests blocked: " +
                std::to_string(blocked));

            clockCycle = 0;

            while (clockCycle < duration)
            {
                // Randomly generate new requests.
                if (random_percent() < arrivalPercent)
                {
                    Request r = generator.generateRequest();
                    ++generated;

                    if (firewall.isBlocked(r.ipIn))
                    {
                        ++blocked;
                        logEvent("Cycle " + std::to_string(clockCycle) +
                            ": firewall blocked request from " + r.ipIn);
                    }
                    else
                    {
                        addRequest(r);
                        ++accepted;
                    }
                }

                // Assign waiting requests to available servers.
                distributeRequests();

                // Count busy servers immediately before processing.
                int busyBeforeTick = 0;

                for (WebServer& server : servers)
                {
                    if (!server.isAvailable())
                        ++busyBeforeTick;
                }

                // Advance processing by one clock cycle.
                updateClock();

                // Count busy servers after processing.
                int busyAfterTick = 0;

                for (WebServer& server : servers)
                {
                    if (!server.isAvailable())
                        ++busyAfterTick;
                }

                // A server that was busy and becomes idle completed a request.
                completed += std::max(
                    0, busyBeforeTick - busyAfterTick
                );

                peakQueue = std::max(peakQueue, queue.size());

                // Dynamically adjust the server count.
                if (clockCycle >= nextScaleCycle)
                {
                    size_t serverCount = servers.size();

                    if (serverCount > 1 &&
                        queue.size() < 50 * serverCount)
                    {
                        if (servers.back().isAvailable())
                        {
                            servers.pop_back();
                            ++serversRemoved;

                            logEvent("Cycle " + std::to_string(clockCycle) +
                                    ": removed one server; active servers = " +
                                    std::to_string(servers.size()));
                        }
                        else
                        {
                            ++scaleDownDeferred;

                            logEvent("Cycle " + std::to_string(clockCycle) +
                                    ": scale-down deferred; last server is busy");
                        }

                        nextScaleCycle = clockCycle + scaleCooldown;
                    }
                    else if (queue.size() > 80 * serverCount)
                    {
                        servers.emplace_back();
                        ++serversAdded;

                        logEvent("Cycle " + std::to_string(clockCycle) +
                                ": added one server; active servers = " +
                                std::to_string(servers.size()));

                        nextScaleCycle = clockCycle + scaleCooldown;
                    }
                }
            }

            // Final simulation summary.
            report("=== Simulation Summary ===");
            report("Total clock cycles: " + std::to_string(clockCycle));
            report("Requests generated: " + std::to_string(generated));
            report("Requests accepted: " + std::to_string(accepted));
            report("Requests blocked: " + std::to_string(blocked));
            report("Requests completed: " + std::to_string(completed));
            report("Requests still queued: " + std::to_string(queue.size()));
            report("Peak queue size: " + std::to_string(peakQueue));
            report("Final active servers: " +
                std::to_string(servers.size()));

            report("Servers added: " + std::to_string(serversAdded));
            report("Servers removed: " + std::to_string(serversRemoved));
            report("Scale-down attempts deferred: " +
                std::to_string(scaleDownDeferred));

            if (log)
                log.close();
        }
};

int main() {
    init_random();

    LoadBalancer loadBalancer;
    loadBalancer.runSimulation();

    return 0;
}