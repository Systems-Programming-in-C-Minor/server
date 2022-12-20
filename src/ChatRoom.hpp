#pragma once

#include <deque>
#include <memory>
#include <set>

class NetworkMessage;
class ChatParticipant;


class ChatRoom {
private:
    std::deque<NetworkMessage> _recent_msgs;
    enum {
        max_recent_msgs = 100
    };
public:
    std::set<std::shared_ptr<ChatParticipant>> participants;

    void join(const std::shared_ptr<ChatParticipant> &participant);

    void leave(const std::shared_ptr<ChatParticipant> &participant);

    void deliver(const NetworkMessage &msg);
};