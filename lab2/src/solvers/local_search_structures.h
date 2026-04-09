#pragma once

enum class MoveType { INTER_SWAP, INTRA_NODE_SWAP, INTRA_EDGE_SWAP };

struct Move {
    MoveType type;
    int index1 = -1;
    int index2 = -1;
    double delta = 0.0;
};
