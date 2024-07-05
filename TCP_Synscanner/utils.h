#include <random>
#include <netdb.h>
#include <thread>
#include <future>
#include <sys/time.h>
#include <mutex>
#include <map>
#include <list>
#include <string.h>

//일단 원래 필요했던 것들 전부 포함
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <cstring>
#include <errno.h>
#include <sys/wait.h>

using std::thread;
extern std::map<std::string, std::list<int>> report;
extern std::mutex mtx;


const int MAX_PORT = 65535;
const int DYNAMIC_PORT = 49152;

/* "Pseudo tcp header" used for checksum calculation */
struct tcp_pheader {
    unsigned int source_address; // 4 byte/s
    unsigned int dest_address;   // 4 byte/s
    unsigned char reserved;      // 1 byte/s
    unsigned char protocol;      // 1 byte/s
    unsigned short tcp_length;   // 2 byte/s
    struct tcphdr tcph;
};

int random_number(int min, int max);
void resault_report();
void set_tcph_checksum(struct tcphdr *tcph, long source_addr, long dest_addr);
void create_tcph(struct tcphdr *tcph, short flags);
void create_iph(struct ip *iph, long source_addr, long dest_addr);
void set_tcph_port(struct tcphdr *tcph, short port);
void packet_sniffer(int recv_sock, struct in_addr dest_addr);
void print_time(struct timeval *timeval);
bool syn_ack_response(char* recv_packet, struct in_addr dest_addr);
std::string getIPv4Address();