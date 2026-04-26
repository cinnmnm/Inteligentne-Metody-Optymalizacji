#pragma once

enum class MemoryMoveType {
    ADD_NODE,
    REMOVE_NODE,
    EDGE_SWAP
};

struct MemoryMove {
    MemoryMoveType type = MemoryMoveType::ADD_NODE;
    int id_a = -1;
    int id_b = -1;
    int id_c = -1;
    int id_d = -1;
    double delta = 0.0;
};
