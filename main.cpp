
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"
#include "firewall.h"

/**
 * @class Request
 * @brief Represents a single web request.
 *
 * Stores the source and destination IP addresses, job type, and
 * processing time required by the request.
 */
class Request
{
public:
    /** Source IP address. */
    std::string ipIn{};

    /** Destination IP address. */
    std::string ipOut{};

    /** Request job type. */
    char jobType{'A'};

    /** Required processing time in clock cycles. */
    int time{1};

    /**
     * @brief Generates a randomized request.
     * @return A fully initialized Request.
     */
    Request generateRequest()
    {
        Request r;
        r.ipIn = random_ip();
        r.ipOut = random_ip();
        r.jobType = random_job();
        r.time = std::max(1, random_req_time(r.jobType));
        return r;
    }
};

/**
 * @class RequestQueue
 * @brief FIFO queue for pending web requests.
 */
class RequestQueue
{
private:
    /** Requests waiting to be assigned to a server. */
    std::queue<Request> requests;

public:
    /**
     * @brief Adds a request to the back of the queue.
     * @param r Request to enqueue.
     */
    void enqueue(Request r)
    {
        requests.push(r);
    }

    /**
     * @brief Removes and returns the oldest queued request.
     * @throws std::out_of_range If the queue is empty.
     * @return The oldest queued request.
     */
    Request dequeue()
    {
        if (requests.empty())
            throw std::out_of_range(
                "Cannot dequeue an empty request queue"
            );

        Request r = requests.front();
        requests.pop();
        return r;
    }

    /**
     * @brief Checks whether the queue contains no requests.
     * @return True if empty; otherwise false.
     */
    bool isEmpty()
    {
        return requests.empty();
    }

    /**
     * @brief Gets the number of queued requests.
     * @return Current queue size.
     */
    size_t size()
    {
        return requests.size();
    }
};

/**
 * @class WebServer
 * @brief Simulates a server processing one request at a time.
 *
 * A server starts idle. Each call to processRequest() advances the
 * current request by one clock cycle.
 */
class WebServer
{
private:
    /** Identifier for this server. */
    int serverId{0};

    /** True when the server is processing a request. */
    bool busy{false};

    /** Request currently assigned to the server. */
    Request currentRequest{};

    /** Remaining processing cycles for the current request. */
    int remainingTime{0};

public:
    /**
     * @brief Assigns a request if the server is available.
     * @param r Request to process.
     */
    void assignRequest(Request r)
    {
        if (!isAvailable())
            return;

        currentRequest = r;
        remainingTime = std::max(1, r.time);
        busy = true;
    }

    /**
     * @brief Advances processing by one clock cycle.
     *
     * @return True exactly once when a request finishes during this
     *         call; false if no request finishes.
     */
    bool processRequest()
    {
        if (isAvailable())
            return false;

        --remainingTime;

        if (remainingTime <= 0)
        {
            remainingTime = 0;
            busy = false;
            return true;
        }

        return false;
    }

    /**
     * @brief Checks whether the server can accept a request.
     * @return True when idle; otherwise false.
     */
    bool isAvailable()
    {
        return !busy;
    }
};

/**
 * @class LoadBalancer
 * @brief Manages request distribution, servers, scaling, and logging.
 *
 * The simulation generates an initial queue and random arrivals,
 * rejects blocked IP addresses, distributes requests to idle servers,
 * and dynamically scales the server pool based on queue size.
 */
class LoadBalancer
{
private:
    /** Queue of pending requests. */
    RequestQueue queue;

    /** Current simulation clock cycle. */
    int clockCycle{0};

    /** Currently active web servers. */
    std::vector<WebServer> servers;

public:
    /**
     * @brief Adds a request to the queue.
     * @param r Request to add.
     */
    void addRequest(Request r)
    {
        queue.enqueue(r);
    }

    /**
     * @brief Assigns queued requests to available servers.
     *
     * Each available server receives at most one request. Requests
     * are assigned in FIFO order.
     */
    void distributeRequests()
    {
        for (WebServer& server : servers)
        {
            if (queue.isEmpty())
                break;

            if (server.isAvailable())
                server.assignRequest(queue.dequeue());
        }
    }

    /**
     * @brief Advances every server and the simulation clock.
     * @return Number of requests completed during this clock cycle.
     *
     * Completion events are counted directly from processRequest(),
     * so the result does not depend on comparing busy-server counts.
     */
    int updateClock()
    {
        int finished = 0;

        for (WebServer& server : servers)
        {
            if (server.processRequest())
                ++finished;
        }

        ++clockCycle;
        return finished;
    }

    /**
     * @brief Loads configuration and executes the simulation.
     *
     * Configuration keys:
     * initial_servers, simulation_cycles, arrival_percent,
     * scale_cooldown, blocked_ranges, logging_mode.
     *
     * Verbose mode records significant events. Quiet mode records
     * only the starting and ending summaries.
     */
    void runSimulation()
    {
        int defaultServers = 5;
        int defaultCycles = 1000;
        int arrivalPercent = 10;
        int scaleCooldown = 10;

        std::vector<std::string> blockedRanges;
        std::string loggingMode = "verbose";

        // Read the optional configuration file.
        std::ifstream config("config.txt");
        std::string line;

        /**
         * @brief Trims whitespace from both ends of a string.
         * @param s String to modify.
         */
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

        while (std::getline(config, line))
        {
            trim(line);

            if (line.empty() || line[0] == '#')
                continue;

            std::size_t equals = line.find('=');

            if (equals == std::string::npos)
                continue;

            std::string key = line.substr(0, equals);
            std::string value = line.substr(equals + 1);

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
                        std::cerr
                            << "Invalid logging_mode; using verbose.\n";
                }
            }
            catch (const std::exception&)
            {
                std::cerr << "Invalid config value for " << key
                          << "; keeping its previous value.\n";
            }
        }

        defaultServers = std::max(1, defaultServers);
        defaultCycles = std::max(1, defaultCycles);
        arrivalPercent = std::clamp(arrivalPercent, 0, 100);
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

        // Reset simulation state.
        queue = RequestQueue{};
        servers.clear();
        servers.resize(initialServerCount);
        clockCycle = 0;

        Firewall firewall;

        for (const std::string& range : blockedRanges)
            firewall.addBlockedRange(range);

        std::ofstream log("loadbalancer.log");

        if (!log)
            std::cerr << "Warning: Could not open loadbalancer.log\n";

        /**
         * @brief Prints and logs a summary line.
         * @param message Text to display and record.
         */
        auto report = [&](const std::string& message)
        {
            std::cout << message << '\n';

            if (log)
                log << message << '\n';
        };

        /**
         * @brief Prints and logs an event only in verbose mode.
         * @param message Event description.
         */
        auto logEvent = [&](const std::string& message)
        {
            if (loggingMode == "verbose")
            {
                std::cout << message << '\n';

                if (log)
                    log << message << '\n';
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
        report("Initial servers: " +
               std::to_string(initialServerCount));
        report("Runtime cycles: " + std::to_string(duration));
        report("Initial queue target: " +
               std::to_string(initialQueueSize));

        Request generator;

        // Populate the initial queue. Firewall rejections are counted
        // as generated but not accepted requests.
        for (int i = 0; i < initialQueueSize; ++i)
        {
            Request r = generator.generateRequest();
            ++generated;

            if (firewall.isBlocked(r.ipIn))
            {
                ++blocked;
                logEvent("Firewall blocked request from " + r.ipIn);
            }
            else
            {
                addRequest(r);
                ++accepted;
            }
        }

        peakQueue = queue.size();

        report("Initial requests accepted: " +
               std::to_string(accepted));
        report("Initial requests blocked: " +
               std::to_string(blocked));

        // Run the simulation.
        while (clockCycle < duration)
        {
            // Generate a new request with the configured probability.
            if (random_percent() < arrivalPercent)
            {
                Request r = generator.generateRequest();
                ++generated;

                if (firewall.isBlocked(r.ipIn))
                {
                    ++blocked;

                    logEvent(
                        "Cycle " + std::to_string(clockCycle) +
                        ": firewall blocked request from " + r.ipIn
                    );
                }
                else
                {
                    addRequest(r);
                    ++accepted;
                }
            }

            // Assign queued work before advancing processing time.
            distributeRequests();

            // Count actual request-completion events.
            completed += updateClock();

            peakQueue = std::max(peakQueue, queue.size());

            // Evaluate scaling only when the cooldown expires.
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

                        logEvent(
                            "Cycle " + std::to_string(clockCycle) +
                            ": removed one server; active servers = " +
                            std::to_string(servers.size())
                        );
                    }
                    else
                    {
                        ++scaleDownDeferred;

                        logEvent(
                            "Cycle " + std::to_string(clockCycle) +
                            ": scale-down deferred; last server is busy"
                        );
                    }

                    nextScaleCycle = clockCycle + scaleCooldown;
                }
                else if (queue.size() > 80 * serverCount)
                {
                    servers.emplace_back();
                    ++serversAdded;

                    logEvent(
                        "Cycle " + std::to_string(clockCycle) +
                        ": added one server; active servers = " +
                        std::to_string(servers.size())
                    );

                    nextScaleCycle = clockCycle + scaleCooldown;
                }
            }
        }

        // Print aggregate results.
        report("=== Simulation Summary ===");
        report("Total clock cycles: " +
               std::to_string(clockCycle));
        report("Requests generated: " +
               std::to_string(generated));
        report("Requests accepted: " +
               std::to_string(accepted));
        report("Requests blocked: " +
               std::to_string(blocked));
        report("Requests completed: " +
               std::to_string(completed));
        report("Requests still queued: " +
               std::to_string(queue.size()));
        report("Peak queue size: " +
               std::to_string(peakQueue));
        report("Final active servers: " +
               std::to_string(servers.size()));
        report("Servers added: " +
               std::to_string(serversAdded));
        report("Servers removed: " +
               std::to_string(serversRemoved));
        report("Scale-down attempts deferred: " +
               std::to_string(scaleDownDeferred));

        if (log)
            log.close();
    }
};

/**
 * @brief Application entry point.
 *
 * Initializes random number generation and runs the load balancer.
 * @return Zero on normal termination.
 */
int main()
{
    init_random();

    LoadBalancer loadBalancer;
    loadBalancer.runSimulation();

    return 0;
}
