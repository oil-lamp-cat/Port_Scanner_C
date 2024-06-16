//기본 함수
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> //스레드

// 추가적인 함수들
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>


#define MAX_PORTS 100  // 최대 포트 번호 범위
#define NUM_THREADS 100  // 사용할 스레드 수 (임의로 설정)

struct ScanThreadArgs {
    const char *host;
    int start_port;
    int end_port;
};

// TCP 연결을 시도하여 포트가 열려 있는지 확인하는 함수 (스레드용)
void *scan_ports(void *args) {
    struct ScanThreadArgs *scan_args = (struct ScanThreadArgs *) args;

    for (int port = scan_args->start_port; port <= scan_args->end_port; port++) {
        struct sockaddr_in addr;
        int sockfd, result;

        // 소켓 생성
        if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            perror("socket"); //오류잡이
            continue;
        }

        // 주소 구조체 초기화
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(scan_args->host);
        addr.sin_port = htons(port);

        // 연결 시도
        result = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));

        // 연결 결과 확인
        if (result == 0) {
            printf("Port %d open\n", port);
        }

        // 소켓 닫기
        close(sockfd);
    }

    pthread_exit(NULL);
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
    printf("%s.%06ld", time_str, timeval->tv_usec);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <host>\n", argv[0]);
        return 1;
    }

    char *host = argv[1];
    struct timeval start_time, end_time, elapsed_time;

    // 프로그램 시작 시간 기록
    gettimeofday(&start_time, NULL);
    printf("Program start time: ");
    print_time(&start_time);
    printf("\n");

    // 스레드 배열 및 인자 초기화
    pthread_t threads[NUM_THREADS];
    struct ScanThreadArgs thread_args[NUM_THREADS];
    int ports_per_thread = MAX_PORTS / NUM_THREADS;

    // 스레드 생성 및 실행
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].host = host;
        thread_args[i].start_port = i * ports_per_thread + 1;
        thread_args[i].end_port = (i + 1) * ports_per_thread;

        if (pthread_create(&threads[i], NULL, scan_ports, (void *) &thread_args[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }

    // 모든 스레드 종료 대기
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 프로그램 종료 시간 기록
    gettimeofday(&end_time, NULL);
    printf("Program end time: ");
    print_time(&end_time);
    printf("\n");

    // 총 걸린 시간 계산
    timersub(&end_time, &start_time, &elapsed_time);
    printf("Total elapsed time: %ld seconds %ld microseconds\n", elapsed_time.tv_sec, elapsed_time.tv_usec);

    return 0;
}
