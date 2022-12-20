#pragma once

enum NetworkableType { // DO NOT CHANGE ORDER, the client has to have the same order to work
    Host,
    Join,
    Leave,
    Allocation,
    Users,
    Update,
};
