#include "utils.h"

std::mutex mtx;

unsigned short checksum(unsigned short *addr, int len) {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w ;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

void set_tcph_checksum(struct tcphdr *tcph, long source_addr, long dest_addr) {
    tcph->th_sum = 0;
    struct tcp_pheader tcp_ph;
    tcp_ph.source_address = source_addr;
    tcp_ph.dest_address = dest_addr;
    tcp_ph.reserved = 0;
    tcp_ph.protocol = IPPROTO_TCP;
    tcp_ph.tcp_length = htons( sizeof(struct tcphdr) );
    memcpy(&tcp_ph.tcph, tcph, sizeof (struct tcphdr));
    tcph->th_sum = checksum( (unsigned short*) &tcp_ph , sizeof (struct tcp_pheader));
}

void create_tcph(struct tcphdr *tcph, short flags) {
    int rand_port = random_number(DYNAMIC_PORT, MAX_PORT);
    tcph->th_sport = htons(rand_port); // has to be > than the dynamically assigned range
    tcph->th_win = htons(1460);
    tcph->th_dport = htons(1);
    tcph->th_seq = htonl(0); 
    tcph->th_ack = htonl(0);
    tcph->th_off = sizeof(struct tcphdr) / 4; // number of 32-bit words in tcp header(where tcph begins)
    tcph->th_flags = flags;    
    tcph->th_win = htons(65535);              // maximum allowed window size 
    tcph->th_sum = 0;
    tcph->th_urp = 0;
}

void set_tcph_port(struct tcphdr *tcph, short port) {
    tcph->th_dport = htons(port);
}

void create_iph(struct ip *iph, long source_addr, long dest_addr) {
    iph->ip_hl = 5;
    iph->ip_v = 4;
    iph->ip_tos = 0;
    iph->ip_len = htons(sizeof(struct ip) + sizeof (struct tcphdr));
    iph->ip_id = htons(0);    
    iph->ip_off = 0;
    iph->ip_ttl = 255;
    iph->ip_p = IPPROTO_TCP;
    iph->ip_src.s_addr = source_addr;
    iph->ip_dst.s_addr = dest_addr;
    iph->ip_sum = 0; // kernel calculates checksum
}

int random_number(int min, int max){
	std::random_device seeder;

	std::mt19937 rng(seeder());
	std::uniform_int_distribution<int> gen(min, max);
	int r = gen(rng);
	return r;
}

void resault_report()
{
    std::cout << "Scan results:" << std::endl;
    mtx.lock();
    for (const auto& pair : report)
    {
        std::cout << "Host: " << pair.first << std::endl;
        for (int port : pair.second)
        {
            std::cout << "Port: " << port << " is open." << std::endl;
        }
    }
    mtx.unlock();
}

void packet_sniffer(int recv_sock, struct in_addr dest_addr)
{
    char* host = inet_ntoa(dest_addr);
    std::string host_str(host);
    while(true)
    {
        char recv_packet[4096] = {0};
        int bytes_received = recv(recv_sock ,recv_packet, sizeof(recv_packet), 0);

        if (bytes_received < 0) {
            std::cerr << "Error receiving packet" << std::endl;
            continue;
        }

        bool port_is_open = syn_ack_response(recv_packet, dest_addr);
        if (port_is_open) {
            struct tcphdr *tcph=(struct tcphdr*)(recv_packet + sizeof(struct ip));
            int port = ntohs(tcph->th_sport);

            mtx.lock();
            report[host].push_back(port);
            mtx.unlock();
        }
    }
}

// 초 단위 시간을 시, 분, 초로 변환하여 출력하는 함수
void print_time(struct timeval *timeval) {
    struct tm *local_time;
    char time_str[30];

    // 초 단위 시간을 struct tm 구조체로 변환
    local_time = localtime(&timeval->tv_sec);

    // 시간을 시:분:초 형식의 문자열로 변환
    strftime(time_str, sizeof(time_str), "%T", local_time);

    // 변환된 문자열과 마이크로초 출력
    std::cout << time_str << "." << timeval->tv_usec;
}

bool syn_ack_response(char* recv_packet, struct in_addr dest_addr) {
    struct ip *iph = (struct ip*)recv_packet;
    char iph_protocol = iph->ip_p;
    long source_addr = iph->ip_src.s_addr;
    int iph_size = iph->ip_hl*4;


    if(iph_protocol == IPPROTO_TCP && memcmp(&source_addr, &dest_addr, sizeof(struct in_addr)) == 0) {
        struct tcphdr *tcph=(struct tcphdr*)(recv_packet + iph_size);
        if(tcph->th_flags == (TH_SYN|TH_ACK))
            return true;
    }
    return false;
}

std::string getIPv4Address() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        exit(1);
    }

    struct addrinfo hints, *info, *p;
    int gai_result;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Force IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
        exit(1);
    }

    std::vector<std::string> ipAddresses;
    for (p = info; p != nullptr; p = p->ai_next) {
        void *addr;
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);

        // Convert the IP to a string and store it:
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        ipAddresses.push_back(ipstr);
    }

    freeaddrinfo(info); // free the linked list

    if (ipAddresses.size() == 1) {
        return ipAddresses[0]; // Return the single IP address
    } else if (ipAddresses.empty()) {
        return "No IPv4 addresses found.";
    } else {
        std::cout << "Available IPv4 addresses:\n";
        for (int i = 0; i < ipAddresses.size(); ++i) {
            std::cout << i + 1 << ": " << ipAddresses[i] << "\n";
        }

        std::cout << "Select an IP address (1-" << ipAddresses.size() << "): ";
        int choice;
        std::cin >> choice;

        if (choice < 1 || choice > static_cast<int>(ipAddresses.size())) {
            return "Invalid selection.";
        }

        return ipAddresses[choice - 1]; // Return the selected IP address
    }
}