/**
 * @file masterloadbalancer.cpp
 * @brief Implements master routing, configuration, and simulation control.
 */

#include "masterloadbalancer.h"
#include "utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Applies firewall filtering and dispatches an incoming request.
 * @param request Request to route.
 * @return Rejection event text, or an empty string if accepted.
 */
std::string MasterLoadBalancer::routeRequest(const Request& request)
{
    if (firewall.isBlocked(request.ipIn))
    {
        ++blocked;

        return "Cycle " + std::to_string(clockCycle) +
               ": firewall blocked request from " + request.ipIn;
    }

    if (request.jobType == 'S')
    {
        streamingBalancer.addRequest(request);
        return {};
    }

    if (request.jobType == 'P')
    {
        processingBalancer.addRequest(request);
        return {};
    }

    ++invalidRequests;

    return "Cycle " + std::to_string(clockCycle) +
           ": rejected request with unknown job type";
}

/**
 * @brief Loads the configuration, runs the simulation, and reports results.
 *
 * Supported configuration keys include:
 * initial_streaming_servers, initial_processing_servers,
 * simulation_cycles, arrival_percent, scale_cooldown,
 * blocked_ranges, and logging_mode.
 */
void MasterLoadBalancer::runSimulation()
{
    int defaultStreamingServers = 3;
    int defaultProcessingServers = 3;
    int defaultCycles = 1000;
    int arrivalPercent = 10;
    int scaleCooldown = 10;

    // These are just for logging purposes, they're used in utils.cpp
    int min_task_time = -1;
    int max_task_time = -1;

    std::vector<std::string> blockedRanges;
    loggingMode = "verbose";
    printingMode = "off";

    // Load configuration; built-in defaults remain if the file is absent.
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
            if (key == "initial_streaming_servers")
                defaultStreamingServers = std::stoi(value);
            else if (key == "initial_processing_servers")
                defaultProcessingServers = std::stoi(value);
            else if (key == "simulation_cycles")
                defaultCycles = std::stoi(value);
            else if (key == "arrival_percent")
                arrivalPercent = std::stoi(value);
            else if (key == "scale_cooldown")
                scaleCooldown = std::stoi(value);
            else if (key == "logging_mode")
            {
                if (value == "verbose" || value == "quiet")
                    loggingMode = value;
                else
                    std::cerr
                        << "Invalid logging_mode; using verbose.\n";
            }
            else if (key == "printing_mode")
            {
                if (value == "off" || value == "on")
                    printingMode = value;
                else
                    std::cerr
                        << "Invalid printing_mode; using off.\n";
            }
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
            else if (key == "min_task_time")
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

    defaultStreamingServers =
        std::max(1, defaultStreamingServers);

    defaultProcessingServers =
        std::max(1, defaultProcessingServers);

    defaultCycles = std::max(1, defaultCycles);
    arrivalPercent = std::clamp(arrivalPercent, 0, 100);
    scaleCooldown = std::max(1, scaleCooldown);

    int streamingServers = defaultStreamingServers;
    int processingServers = defaultProcessingServers;
    int duration = defaultCycles;

    // Initialize isolated child pools.
    streamingBalancer.initializeServers(streamingServers, 'S');
    processingBalancer.initializeServers(processingServers, 'P');

    // Configure the firewall.
    firewall = Firewall{};

    for (const std::string& range : blockedRanges)
        firewall.addBlockedRange(range);

    // Open the event log.
    std::ofstream log("loadbalancer.log");

    if (!log)
        std::cerr << "Warning: Could not open loadbalancer.log\n";

    /**
     * @brief Writes a summary line to the console and log.
     * @param message Summary message.
     *
     * Summary messages are emitted in both verbose and quiet modes.
     */
    auto report = [&](const std::string& message)
    {
        if(printingMode == "on")
            std::cout << message << '\n';

        if (log)
            log << message << '\n';
    };

    /**
     * @brief Writes a detailed event only in verbose mode.
     * @param message Event description.
     */
    auto logEvent = [&](const std::string& message)
    {
        if (loggingMode == "verbose")
        {
            if(printingMode == "on")
                std::cout << message << '\n';

            if (log)
                log << message << '\n';
        }
    };

    generated = 0;
    blocked = 0;
    invalidRequests = 0;
    clockCycle = 0;

    const int initialQueueTarget =
        (streamingServers + processingServers) * 100;

    report("=== Master Load Balancer: Starting Summary ===");
    report("Logging mode: " + loggingMode);
    report("Streaming servers: " + std::to_string(streamingServers));
    report("Processing servers: " + std::to_string(processingServers));
    // report("Simulation cycles: " + std::to_string(duration));
    // report("Arrival probability: " +
    //        std::to_string(arrivalPercent) + "%");
    // report("Scaling cooldown: " + std::to_string(scaleCooldown));
    report("Initial request queue sizes (streaming + processing): " +
           std::to_string(initialQueueTarget));
    report("Task time range: " + std::to_string(min_task_time)
        + "-" + std::to_string(max_task_time));

    // Generate the initial combined queue. Each request is generated once
    // and then sent to its appropriate pool through the master switch.
    Request generator;

    for (int i = 0; i < initialQueueTarget; ++i)
    {
        Request request = generator.generateRequest();
        ++generated;

        const std::string event = routeRequest(request);

        if (!event.empty())
            logEvent(event);
    }

    report("Initial streaming requests accepted: " +
           std::to_string(streamingBalancer.getAccepted()));
    report("Initial processing requests accepted: " +
           std::to_string(processingBalancer.getAccepted()));
    report("Initial requests blocked: " + std::to_string(blocked));

    // Main simulation loop: the master owns the clock.
    while (clockCycle < duration)
    {
        // Randomly generate a new request.
        if (random_percent() < arrivalPercent)
        {
            Request request = generator.generateRequest();
            ++generated;

            const std::string event = routeRequest(request);

            if (!event.empty())
                logEvent(event);
        }

        // Assign work independently in each pool.
        streamingBalancer.distributeRequests();
        processingBalancer.distributeRequests();

        // Advance both pools on the same simulation cycle.
        ++clockCycle;

        streamingBalancer.updateClock(clockCycle);
        processingBalancer.updateClock(clockCycle);

        // Evaluate capacity independently for each pool.
        const std::string streamingEvent =
            streamingBalancer.adjustCapacity(
                clockCycle, scaleCooldown
            );

        if (!streamingEvent.empty())
            logEvent("Streaming pool: " + streamingEvent);

        const std::string processingEvent =
            processingBalancer.adjustCapacity(
                clockCycle, scaleCooldown
            );

        if (!processingEvent.empty())
            logEvent("Processing pool: " + processingEvent);
    }

    // Print final results for the whole system.
    const int accepted =
        streamingBalancer.getAccepted() +
        processingBalancer.getAccepted();

    const int completed =
        streamingBalancer.getCompleted() +
        processingBalancer.getCompleted();

    const std::size_t outstanding =
        streamingBalancer.getOutstandingRequests() +
        processingBalancer.getOutstandingRequests();

    report("=== Master Load Balancer: Ending Summary ===");
    report("Total cycles: " + std::to_string(clockCycle));
    report("Requests generated: " + std::to_string(generated));
    report("Requests accepted: " + std::to_string(accepted));
    report("Requests blocked: " + std::to_string(blocked));
    report("Requests with invalid job types: " +
           std::to_string(invalidRequests));
    report("Requests completed: " + std::to_string(completed));
    report("Requests queued or processing: " +
           std::to_string(outstanding));

    report("--- Streaming Pool ---");
    report("Requests accepted: " +
           std::to_string(streamingBalancer.getAccepted()));
    report("Requests completed: " +
           std::to_string(streamingBalancer.getCompleted()));
    report("Requests queued: " +
           std::to_string(streamingBalancer.getQueueSize()));
    report("Requests processing: " +
           std::to_string(streamingBalancer.getBusyServers()));
    report("Peak queue: " +
           std::to_string(streamingBalancer.getPeakQueue()));
    report("Initial servers: " +
           std::to_string(streamingBalancer.getInitialServers()));
    report("Active servers: " +
           std::to_string(streamingBalancer.getServerCount()));
    report("Servers added: " +
           std::to_string(streamingBalancer.getServersAdded()));
    report("Servers removed: " +
           std::to_string(streamingBalancer.getServersRemoved()));
    // report("Deferred scale-downs: " +
    //        std::to_string(streamingBalancer.getScaleDownDeferred()));

    report("--- Processing Pool ---");
    report("Requests accepted: " +
           std::to_string(processingBalancer.getAccepted()));
    report("Requests completed: " +
           std::to_string(processingBalancer.getCompleted()));
    report("Requests queued: " +
           std::to_string(processingBalancer.getQueueSize()));
    report("Requests processing: " +
           std::to_string(processingBalancer.getBusyServers()));
    report("Peak queue: " +
           std::to_string(processingBalancer.getPeakQueue()));
    report("Initial servers: " +
           std::to_string(processingBalancer.getInitialServers()));
    report("Active servers: " +
           std::to_string(processingBalancer.getServerCount()));
    report("Servers added: " +
           std::to_string(processingBalancer.getServersAdded()));
    report("Servers removed: " +
           std::to_string(processingBalancer.getServersRemoved()));
    // report("Deferred scale-downs: " +
    //        std::to_string(processingBalancer.getScaleDownDeferred()));

    // These values should reconcile, including requests still in flight.
    // report("Accounting check (accepted - completed - outstanding): " +
    //        std::to_string(
    //            accepted - completed - static_cast<int>(outstanding)
    //        ));

    if (log)
        log.close();
}