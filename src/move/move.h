#ifndef MOVE_H
#define MOVE_H

#include "../types.h"
#include "../board/board.h"

#define MOVE_NONE 0
#define MOVE_NULL 1

#include "movegen.h"

#define move_from(m) ((m) & 63)
#define move_to(m) (((m) >> 6) & 63)
#define move_promotion(m) (((m) >> 12) & 7)
#define make_move(from, to) (((from) | ((to) << 6)))
#define make_promotion_move(from, to, promo) (((from) | ((to) << 6) | ((promo) << 12)))
#define is_valid_move(m) ((m) != MOVE_NONE)

#endif
