#include "p_local.hpp"


dirtype_t operator++(dirtype_t & dir, int) {
    dirtype_t old = dir;
    dir = (dirtype_t) ((int) dir + 1);
    return old;
};

dirtype_t operator--(dirtype_t & dir, int) {
    dirtype_t old = dir;
    dir = (dirtype_t) ((int) dir - 1);
    return old;
};

dirtype_t operator++(dirtype_t & dir) {
    dirtype_t old = dir;
    dir = (dirtype_t) ((int) dir + 1);
    return old;
};

dirtype_t operator--(dirtype_t & dir) {
    dirtype_t old = dir;
    dir = (dirtype_t) ((int) dir - 1);
    return old;
};