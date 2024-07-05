#ifndef SYNSCANNER_H
#define SYNSCANNER_H

#include <iostream>
#include <vector>
#include <future>
#include <chrono>
#include <thread>

using namespace std;

void syn_scan(std::string desthost, int start_port, int end_port);

#endif