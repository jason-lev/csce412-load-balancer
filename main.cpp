#include <iostream>
#include "utils.h"
#include <queue>


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
    public:
    std::queue<Request> requests;
    void enqueue(Request r) {
        requests.push(r);
    }
    Request dequeue() {
        Request r = requests.back();;
        requests.pop();
        return r;
    }
    bool isEmpty() {
        return requests.empty();
    }
    int size() {
        return requests.size();
    }
};

int main() {
    init_random();
    std::cout << "hello " << std::endl;
    Request r;
    r = r.generateRequest();
    std::cout << "ipIn: " << r.ipIn << std::endl;
    std::cout << "ipOut: " << r.ipOut << std::endl;
    std::cout << "job: " << r.jobType << std::endl;
    std::cout << "time: " << r.time << std::endl;
    return 0;
}