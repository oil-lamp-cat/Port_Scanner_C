#include <iostream>
#include <vector>
#include <future>
#include <cstring> //c언어 스트링 함수 쓰려고
#include <unistd.h>

// 추가적인 헤더파일
#include <sys/time.h> //시간
#include <sys/socket.h> //소켓
#include <arpa/inet.h>



#define MAX_PORTS 1000  // 최대 포트 번호 범위

// TCP 연결을 시도하여 포트가 열려 있는지 확인하는 함수 (비동기 방식)
int scan_port_async(const std::string &host, int port) {
    struct sockaddr_in addr;
    int sockfd, result;

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        return -1;
    }

    // 주소 구조체 초기화
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    addr.sin_port = htons(port);

    // 연결 시도
    result = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));

    // 소켓 닫기
    close(sockfd);

    // 연결 결과 반환
    if (result == 0) {
        return port;  // 포트 열림
    } else if (errno == ECONNREFUSED) {
        return 0;  // 포트 닫힘
    } else {
        perror("connect");
        return -1; // 에러 발생
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

int main(int argc, char *argv[]) {
    if (argc < 2) { //만약 옵션이 들어오지 않았다면
        std::cerr << "Usage: " << argv[0] << " <host>" << std::endl;
        return 1;
    }

    std::string host = argv[1]; //ip 할당
    struct timeval start_time, end_time, elapsed_time;

    // 프로그램 시작 시간 기록
    gettimeofday(&start_time, nullptr);
    std::cout << "Program start time: ";
    print_time(&start_time);
    std::cout << std::endl;

    std::vector<std::future<int>> futures;
    std::vector<int> open_ports;

    // 비동기적으로 포트 스캔 실행
    for (int port = 1; port <= MAX_PORTS; ++port) {
        futures.push_back(std::async(std::launch::async, scan_port_async, host, port));
    }

    // 모든 비동기 작업 완료 및 결과 처리
    int open_ports_count = 0, closed_ports = 0, error_ports = 0;
    for (auto &future : futures) {
        int result = future.get(); // 비동기 작업 결과 가져오기

        if (result > 0) {
            open_ports.push_back(result);
            open_ports_count;
        } else if (result == 0) {
            ++closed_ports;
        } else {
            ++error_ports;
        }
    }

    // 프로그램 종료 시간 기록
    gettimeofday(&end_time, nullptr);
    std::cout << "Program end time: ";
    print_time(&end_time);
    std::cout << std::endl;

    // 총 걸린 시간 계산
    timersub(&end_time, &start_time, &elapsed_time);
    std::cout << "Total elapsed time: " << elapsed_time.tv_sec << " seconds " << elapsed_time.tv_usec << " microseconds" << std::endl;

    // 결과 출력
    if (!open_ports.empty()) {
        std::cout << "Open ports:";
        for (auto port : open_ports) {
            std::cout << " " << port;
        }
        std::cout << std::endl;
    } else {
        std::cout << "No open ports found." << std::endl;
    }

    std::cout << "Open ports: " << open_ports_count << std::endl;
    std::cout << "Closed ports: " << closed_ports << std::endl;
    std::cout << "Errors: " << error_ports << std::endl;

    return 0;
}
