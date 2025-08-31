#include "include/Network.hpp"
#include "include/ScreenManager.hpp"
#include "include/ScreenViewer.hpp"
#include <string>


int main(int argc, char* argv[])
{
    int port;
    int port_recipient;
    std::string ip_recipient_frame = "192.168.31.206";
    std::string ip_recipient_event = "192.168.31.206";
    // std::string ip_recipient_event = "192.168.31.102";
    // std::string ip_recipient = "192.168.31.102";
    // std::string ip_recipient = "192.168.31.187";

    if (argc >= 2) {
        ip_recipient_frame = argv[1];
        ip_recipient_event = (argc >= 3) ? argv[2] : argv[1];
    }

    std::string arg;
    std::cout << "> ";
    std::cin >> arg;

    if (arg == "2") {
        port = 5555;
        port_recipient = 8888;
    }
    else {
        port = 8888;
        port_recipient = 5555;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    try {
        Network client(port);
        client.start(arg, ip_recipient_frame, ip_recipient_event, port_recipient);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return 0;
}