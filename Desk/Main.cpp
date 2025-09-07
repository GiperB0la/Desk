#include "include/Desk.hpp"


int main(int argc, char* argv[])
{
    std::string ip = "127.0.0.1";
    unsigned int port = 8888;

    if (argc >= 3) {
        ip = argv[1];
        port = std::stoi(argv[2]);
    }

    Desk desk(ip, port);
    desk.run();

    return 0;
}