#include <iostream>
#include "ChatSession.hpp"
#include "ChatRoom.hpp"
#include "asio/read.hpp"
#include "asio/write.hpp"

using asio::ip::tcp;


void ChatSession::do_read_header() {
    auto self(shared_from_this());
    asio::async_read(
            _socket,
            asio::buffer(_read_msg.data(), NetworkMessage::header_length),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (!ec && _read_msg.decode_header()) {
                    do_read_body();
                } else {
                    _room.leave(shared_from_this());
                }
            }
    );
}

void ChatSession::do_read_body() {
    auto self(shared_from_this());
    asio::async_read(
            _socket,
            asio::buffer(_read_msg.body(), _read_msg.body_length()),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    _room.deliver(_read_msg);
                    do_read_header();
                } else {
                    _room.leave(shared_from_this());
                }
            }
    );
}

void ChatSession::do_write() {
    auto self(shared_from_this());
    asio::async_write(
            _socket,
            asio::buffer(_write_msgs.front().data(), _write_msgs.front().length()),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    _write_msgs.pop_front();
                    if (!_write_msgs.empty()) {
                        do_write();
                    }
                } else {
                    _room.leave(shared_from_this());
                }
            }
    );
}

void ChatSession::start() {
    _room.join(shared_from_this());
    do_read_header();
}

void ChatSession::deliver(const NetworkMessage &msg) {
    bool write_in_progress = !_write_msgs.empty();
    _write_msgs.push_back(msg);
    if (!write_in_progress) {
        do_write();
    }
}

ChatSession::ChatSession(tcp::socket socket, ChatRoom &room) : _socket(std::move(socket)),
                                                               _room(room),
                                                               ChatParticipant(room) {}
