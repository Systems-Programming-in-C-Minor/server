#pragma once

#include "ChatRoom.hpp"
#include "asio/ip/tcp.hpp"


class ChatServer {
private:
    asio::ip::tcp::acceptor _acceptor;
    ChatRoom _room;

    void do_accept();

public:
    ChatServer(asio::io_context &io_context, const asio::ip::tcp::endpoint &endpoint);
};