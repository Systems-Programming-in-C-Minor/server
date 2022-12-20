#include <list>
#include "ChatRoom.hpp"
#include "nlohmann/json.hpp"
#include "NetworkableType.hpp"
#include "NetworkMessage.hpp"
#include "ChatParticipant.hpp"

using nlohmann::json;


void ChatRoom::join(const std::shared_ptr<ChatParticipant> &participant) {
    participants.insert(participant);

    if (participants.size() == 1) {
        auto host_json = json();
        host_json["type"] = Host;
        participant->deliver(NetworkMessage(host_json.dump()));
        participant->is_host = true;
    }

    auto allocation_json = json();
    allocation_json["type"] = Allocation;
    allocation_json["user_id"] = participant->user_id;
    participant->deliver(NetworkMessage(allocation_json.dump()));

//    for (auto msg: _recent_msgs)
//        participant->deliver(msg);

    auto join_json = json();
    join_json["type"] = Join;
    join_json["user_id"] = participant->user_id;
    const auto join_message = NetworkMessage(join_json.dump());
    for (const auto &p: participants) {
        if (p == participant) continue;
        p->deliver(join_message);
    }

    auto users_json = json();
    users_json["type"] = Users;
    auto user_ids = std::list<int>();

    for (const auto &item: participants) {
        user_ids.push_back(item->user_id);
    }

    users_json["user_ids"] = user_ids;

    deliver(NetworkMessage(users_json.dump()));
}

void ChatRoom::leave(const std::shared_ptr<ChatParticipant> &participant) {
    participants.erase(participant);

    if (participant->is_host) {
        for (const auto &p: participants) {
            auto host_json = json();
            host_json["type"] = Host;
            p->deliver(NetworkMessage(host_json.dump()));
            p->is_host = true;
            break;
        }
    }

    auto leave_json = json();
    leave_json["type"] = Leave;
    leave_json["user_id"] = participant->user_id;
    const auto join_message = NetworkMessage(leave_json.dump());
    for (const auto &p: participants)
        p->deliver(join_message);

    auto users_json = json();
    users_json["type"] = Users;
    auto user_ids = std::list<int>();

    for (const auto &item: participants) {
        user_ids.push_back(item->user_id);
    }

    users_json["user_ids"] = user_ids;

    deliver(NetworkMessage(users_json.dump()));
}

void ChatRoom::deliver(const NetworkMessage &msg) {
    _recent_msgs.push_back(msg);
    while (_recent_msgs.size() > max_recent_msgs)
        _recent_msgs.pop_front();

    for (const auto &participant: participants)
        participant->deliver(msg);
}
