#include <list>
#include <algorithm>
#include "ChatParticipant.hpp"
#include "ChatRoom.hpp"

ChatParticipant::ChatParticipant(const ChatRoom &room) {
    user_id = 0;

    auto user_ids = std::list<int>();

    for (const auto &item: room.participants) {
        user_ids.push_back(item->user_id);
    }

    for (int i = 0;; i++) {
        if (std::find(user_ids.begin(), user_ids.end(), i) == user_ids.end()) {
            user_id = i;
            break;
        }
    }
}