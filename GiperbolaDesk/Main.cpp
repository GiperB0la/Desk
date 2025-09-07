#include "include/Desk.hpp"


//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//{
//    std::string ip = "127.0.0.1";
//    unsigned int port = 8888;
//
//    int argc = 0;
//    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
//
//    if (argvW && argc >= 3) {
//        char buffer[256];
//
//        WideCharToMultiByte(CP_UTF8, 0, argvW[1], -1, buffer, sizeof(buffer), nullptr, nullptr);
//        ip = buffer;
//
//        WideCharToMultiByte(CP_UTF8, 0, argvW[2], -1, buffer, sizeof(buffer), nullptr, nullptr);
//        port = std::stoi(buffer);
//    }
//
//    if (argvW) {
//        LocalFree(argvW);
//    }
//
//    Desk desk(ip, port);
//    desk.run();
//
//    return 0;
//}

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