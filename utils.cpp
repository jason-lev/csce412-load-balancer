#include "utils.h"
#include <iostream>

std::random_device rnd_device;
std::mt19937 mersenne_engine;

void init_random() {
    mersenne_engine = std::mt19937(rnd_device());
    std::cout << "Random initialized! Value: " << mersenne_engine() << std::endl;
}

std::string random_ip() {
    int start;
    do {
        start = mersenne_engine() % 1000;
    } while(start == 10 || start == 192 || start == 172);
    std::string ip = std::to_string(start) + "." + std::to_string(mersenne_engine() % 1000) + "." 
        + std::to_string(mersenne_engine() % 1000) + "." + std::to_string(mersenne_engine() % 1000);
    return ip;
}

char random_job() {
    return (mersenne_engine() % 2 == 0) ? 'P' : 'S';
}

int random_req_time(char job) {
    return 50 + (mersenne_engine() % 500);
}

int random_percent() {
    return (mersenne_engine() % 100);
}