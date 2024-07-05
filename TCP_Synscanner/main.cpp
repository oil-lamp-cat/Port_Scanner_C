#include "SynScanner.h"
#include "utils.h"

std::map<std::string, std::list<int>> report;


void usage(const char* program_name)
{
    std::cerr << "Usage: " << program_name << "[-p port_range] Target" << std::endl;
}

int main(int argc, char ** argv)
{
    //char* host = argv[1];
    int start_port = 1;
    int end_port = 65535;
    int opt;
    int mode=0;

    while ((opt = getopt(argc, argv, "p:hs")) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                char* dash = std::strchr(optarg, '-');
                if(dash)
                {
                    *dash = '\0';
                    start_port = std::atoi(optarg);
                    end_port = std::atoi(dash + 1);
                }
                else
                {
                    std::cerr << "불가능한 포트 범위";
                    exit(1);
                }
                break;
            }
            case 's':
            {
                mode=1;
                break;
            }
            case '?':
            case 'h':
            {
                usage(argv[0]);
                exit(1);
            }
            default:
            {
                mode=0;
            }
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
        exit(1);
    }

    std::string host = argv[optind];

    switch (mode)
    {
        case 0:
        {
            std::cout << "full open scan을 시작합니다." << std::endl;
            sleep(1);
            break;
        }
        case 1:
        {
            std::cout << "syn scan을 시작합니다." << std::endl;
            sleep(1);
            syn_scan(host, start_port, end_port);
            break;
        }
        default:
        {
            std::cout << "mode 선택 오류 발생" << std::endl;
            break;
        }
    }

    resault_report();
}