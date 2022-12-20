#include <iostream>
#include "ChatServer.hpp"
#include "ChatSession.hpp"

using asio::ip::tcp;


void ChatServer::do_accept() {
    _acceptor.async_accept(
            [this](std::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "Connection established: " << socket.remote_endpoint().address().to_string()
                              << std::endl;
                    std::make_shared<ChatSession>(std::move(socket), _room)->start();
                }

                do_accept();
            });
}

ChatServer::ChatServer(asio::io_context &io_context, const tcp::endpoint &endpoint) : _acceptor(io_context, endpoint) {
    do_accept();
}
