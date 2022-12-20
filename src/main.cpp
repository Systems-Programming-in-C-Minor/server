#include <cstdlib>
#include <iostream>
#include <list>
#include "NetworkMessage.hpp"
#include "ChatServer.hpp"

using asio::ip::tcp;


int main(int argc, char *argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <port> [<port> ...]\n";
            return 1;
        }

        asio::io_context io_context;

        std::list<ChatServer> servers;
        for (int i = 1; i < argc; ++i) {
            const auto endpoint = tcp::endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_context, endpoint);
        }

        io_context.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
