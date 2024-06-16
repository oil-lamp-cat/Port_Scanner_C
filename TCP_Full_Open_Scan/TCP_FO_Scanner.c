#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h> //close 쓰기 위해

int main(int argc, char* argv[]){ // 인자 받기
    int sockfd; //소켓 초기화
    struct sockaddr_in dest_addr; //소켓 구조체 선언
    int port = 0; // 포트 초기화
    int ret = 0; // 연결 결과 초기화
    for(port = 1; port <= 1024; port ++) // 스캔 포트 수 1 ~ 1024까지
    {
        sockfd = socket(PF_INET, SOCK_STREAM, 0); //소켓 생성
        memset((char*)&dest_addr,0,sizeof(dest_addr)); //메모리 초기화 함수
        dest_addr.sin_family = AF_INET; //프로토콜 주소체계 저장
        dest_addr.sin_port = htons(port); //사용할 포트 저장
        dest_addr.sin_addr.s_addr = inet_addr(argv[1]); //IP할당
        ret = connect(sockfd,(struct sockaddr*)&dest_addr,sizeof(dest_addr)); //연결
        if(ret != -1){ // 결과 -1 이면 포트 생존
            printf("%d Port Open\n", port);
        }
        close(sockfd); //소켓 닫기
    }
    printf("DONE \n");
    return 0;
}