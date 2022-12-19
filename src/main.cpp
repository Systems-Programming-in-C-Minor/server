#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <asio.hpp>
#include "network_message.hpp"

using asio::ip::tcp;

class ChatParticipant {
public:
    virtual ~ChatParticipant() = default;

    virtual void deliver(const NetworkMessage &msg) = 0;
};

class ChatRoom {
private:
    std::set<std::shared_ptr<ChatParticipant>> _participants;
    std::deque<NetworkMessage> _recent_msgs;
    enum {
        max_recent_msgs = 100
    };
public:
    void join(const std::shared_ptr<ChatParticipant> &participant) {
        _participants.insert(participant);
        for (auto msg: _recent_msgs)
            participant->deliver(msg);
    }

    void leave(const std::shared_ptr<ChatParticipant> &participant) {
        _participants.erase(participant);
    }

    void deliver(const NetworkMessage &msg) {
        _recent_msgs.push_back(msg);
        while (_recent_msgs.size() > max_recent_msgs)
            _recent_msgs.pop_front();

        for (const auto &participant: _participants)
            participant->deliver(msg);
    }
};


class ChatSession : public ChatParticipant, public std::enable_shared_from_this<ChatSession> {
private:
    tcp::socket _socket;
    ChatRoom &_room;
    NetworkMessage _read_msg;
    std::deque<NetworkMessage> _write_msgs;

    void do_read_header() {
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

    void do_read_body() {
        auto self(shared_from_this());
        asio::async_read(
                _socket,
                asio::buffer(_read_msg.body(), _read_msg.body_length()),
                [this, self](std::error_code ec, std::size_t /*length*/) {
                    if (!ec) {
                        std::cout << "Message: " << std::string(_read_msg.body(), _read_msg.body_length()) << std::endl;

                        _room.deliver(_read_msg);
                        do_read_header();
                    } else {
                        _room.leave(shared_from_this());
                    }
                }
        );
    }

    void do_write() {
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

public:
    ChatSession(tcp::socket socket, ChatRoom &room) : _socket(std::move(socket)), _room(room) {
    }

    void start() {
        _room.join(shared_from_this());
        do_read_header();
    }

    void deliver(const NetworkMessage &msg) override {
        bool write_in_progress = !_write_msgs.empty();
        _write_msgs.push_back(msg);
        if (!write_in_progress) {
            do_write();
        }
    }
};


class ChatServer {
private:
    tcp::acceptor _acceptor;
    ChatRoom _room;

    void do_accept() {
        _acceptor.async_accept(
                [this](std::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::cout << "Connection established: " << socket.remote_endpoint().address().to_string() << std::endl;
                        std::make_shared<ChatSession>(std::move(socket), _room)->start();
                    }

                    do_accept();
                });
    }

public:
    ChatServer(asio::io_context &io_context, const tcp::endpoint &endpoint) : _acceptor(io_context, endpoint) {
        do_accept();
    }
};


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
