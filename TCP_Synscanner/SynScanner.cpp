#include "utils.h"
#include "SynScanner.h"

void send_packet(int send_sock, const unsigned char* packet, size_t packet_size, const struct sockaddr* address) {
    if (sendto(send_sock, packet, packet_size, 0, address, sizeof(struct sockaddr)) < 0) {
        cerr << "ERROR sending packet" << endl;
    }
}


void syn_scan(std::string desthost, int start_port, int end_port)
{   
    unsigned char packet[40];

    struct timeval start_time, end_time, elapsed_time;
    struct iphdr *iphdr;
    struct tcphdr *tcphdr;
    struct sockaddr_in address;

    std::vector<std::future<int>> futures;
    std::string ipv4 = getIPv4Address();

    //ip from
    long source_address = inet_addr(ipv4.c_str());
    //ip to
    long dest_address = inet_addr(desthost.c_str());
    short flags = TH_SYN;

    //socket 생성
    int send_sock = socket( AF_INET, SOCK_RAW, IPPROTO_RAW );
    if (send_sock < 0)
        cerr << "ERROR opening socket" << endl;
    int recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (recv_sock < 0)
        cerr << "ERROR opening socket" << endl;

    struct ip *iph = (struct ip *) packet;
    struct tcphdr *tcph = (struct tcphdr *) (packet + 20);
    
    create_iph(iph, source_address, dest_address);
    create_tcph(tcph, flags);

    gettimeofday(&start_time, nullptr);

    thread sniffer(packet_sniffer, recv_sock, (struct in_addr ){inet_addr(desthost.c_str())});
    for (int port=start_port; port <= end_port; port++)
    {
        set_tcph_port(tcph, port);
        set_tcph_checksum(tcph, source_address, dest_address);

        address.sin_family = AF_INET;

        auto future = std::async(std::launch::async, send_packet, send_sock, packet, sizeof(packet), (struct sockaddr *)&address);
    }   
    sniffer.detach();
    close(send_sock);
    close(recv_sock);

    gettimeofday(&end_time, nullptr);

    timersub(&end_time, &start_time, &elapsed_time);
    std::cout << "Total elapsed time: " << elapsed_time.tv_sec << " seconds " << elapsed_time.tv_usec << " microseconds" << std::endl;
}