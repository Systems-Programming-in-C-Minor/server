#pragma once


#include <memory>
#include <deque>
#include "ChatParticipant.hpp"
#include "NetworkMessage.hpp"
#include "asio/ip/tcp.hpp"


class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
private:
    asio::ip::tcp::socket _socket;
    ChatRoom &_room;
    NetworkMessage _read_msg;
    std::deque<NetworkMessage> _write_msgs;

    void do_read_header();

    void do_read_body();

    void do_write();

public:
    ChatSession(asio::ip::tcp::socket socket, ChatRoom &room);

    void start();

    void deliver(const NetworkMessage &msg) override;
};
