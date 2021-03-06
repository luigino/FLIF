/*
 FLIF - Free Lossless Image Format
 Copyright (C) 2010-2015  Jon Sneyers & Pieter Wuille, LGPL v3+

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdint.h>

#include "chance.hpp"
#include "bit.hpp"

void build_table(uint16_t *zero_state, uint16_t *one_state, size_t size, uint32_t factor, unsigned int max_p)
{
    const int64_t one = 1LL << 32;
    int64_t p;
    unsigned int last_p8, p8;
    unsigned int i;

    memset(zero_state,0,sizeof(uint16_t) * size);
    memset(one_state,0,sizeof(uint16_t) * size);

    last_p8 = 0;
    p = one / 2;
    for (i = 0; i < size / 2; i++) {
        p8 = (size * p + one / 2) >> 32; //FIXME try without the one
        if (p8 <= last_p8) p8 = last_p8 + 1;
        if (last_p8 && last_p8 < size && p8 <= max_p) one_state[last_p8] = p8;

        p += ((one - p) * factor + one / 2) >> 32;
        last_p8 = p8;
    }

    for (i = size - max_p; i <= max_p; i++) {
        if (one_state[i]) continue;

        p = (i * one + size / 2) / size;
        p += ((one - p) * factor + one / 2) >> 32;
        p8 = (size * p + one / 2) >> 32; //FIXME try without the one
        if (p8 <= i) p8 = i + 1;
        if (p8 > max_p) p8 = max_p;
        one_state[i] = p8;
    }

    for (i = 1; i < size; i++)
        zero_state[i] = size - one_state[size - i];
}

/** Computes an approximation of log(4096 / x) / log(2) * base */
static uint32_t log4kf(int x, uint32_t base) {
    int bits = 8 * sizeof(int) - __builtin_clz(x);
    uint64_t y = ((uint64_t)x) << (32 - bits);
    uint32_t res = base * (13 - bits);
    uint32_t add = base;
    while ((add > 1) && ((y & 0x7FFFFFFF) != 0)) {
        y = (((uint64_t)y) * y + 0x40000000) >> 31;
        add >>= 1;
        if ((y >> 32) != 0) {
            res -= add;
            y >>= 1;
        }
    }
    return res;
}

Log4kTable::Log4kTable() {
    data[0] = 0;
    for (int i = 1; i <= 4096; i++) {
        data[i] = (log4kf(i, (65535UL << 16) / 12) + (1 << 15)) >> 16;
    }
    scale = 65535 / 12;
}

const Log4kTable log4k;
