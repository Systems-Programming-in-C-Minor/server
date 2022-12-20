#pragma once

class ChatRoom;
class NetworkMessage;

class ChatParticipant {
public:
    explicit ChatParticipant(const ChatRoom& room);

    virtual ~ChatParticipant() = default;

    int user_id;
    bool is_host = false;

    virtual void deliver(const NetworkMessage &msg) = 0;
};
