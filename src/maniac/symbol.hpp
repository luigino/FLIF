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

#pragma once

#include <vector>
#include <assert.h>
#include "util.hpp"
#include "chance.hpp"
#include "../compiler-specific.hpp"

template <typename RAC> class UniformSymbolCoder {
private:
    RAC &rac;

public:
    UniformSymbolCoder(RAC &racIn) : rac(racIn) { }

#ifdef HAS_ENCODER
    void write_int(int min, int max, int val);
#endif
    int read_int(int min, int max) {
        assert(max >= min);
        if (min != 0) {
            max -= min;
        }
        if (max == 0) return min;

        // split in [0..med] [med+1..max]
        int med = max/2;
        bool bit = rac.read_bit();
        if (bit) {
            return read_int(min+med+1, min+max);
        } else {
            return read_int(min, min+med);
        }
    }
};

typedef enum {
    BIT_ZERO,
    BIT_SIGN,
    BIT_EXP,
    BIT_MANT,
//    BIT_EXTRA
} SymbolChanceBitType;

//static const char *SymbolChanceBitName[] = {"zero", "sign", "expo", "mant"};

static const uint16_t EXP_CHANCES[] = {1000, 1200, 1500, 1750, 2000, 2300, 2800, 2400, 2300,
                                       2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048};
static const uint16_t MANT_CHANCES[] = {1900, 1850, 1800, 1750, 1650, 1600, 1600, 2048, 2048,
                                        2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048};
static const uint16_t ZERO_CHANCE = 1000;
static const uint16_t SIGN_CHANCE = 2048;
/*
static const uint16_t EXP_CHANCES[] = {1200, 1600, 1800, 1900, 2050, 2300, 2500, 2300, 2048,
                                       2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048};
static const uint16_t MANT_CHANCES[] = {1750, 1730, 1710, 1670, 1650, 1700, 1800, 1800, 2048,
                                        2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048};
static const uint16_t ZERO_CHANCE = 1600;
static const uint16_t SIGN_CHANCE = 2048;
*/
#ifdef STATS
struct SymbolChanceStats {
    BitChanceStats stats_zero;
    BitChanceStats stats_sign;
    BitChanceStats stats_exp[17];
    BitChanceStats stats_mant[18];

    std::string format() const;
    ~SymbolChanceStats();
};

extern SymbolChanceStats global_symbol_stats;
#endif

template <typename BitChance, int bits> class SymbolChance {
    BitChance bit_zero;
    BitChance bit_sign;
    BitChance bit_exp[bits-1];
    BitChance bit_mant[bits];

public:

    BitChance inline &bitZero()      {
        return bit_zero;
    }

    BitChance inline &bitSign()      {
        return bit_sign;
    }

    // all exp bits 0         -> int(log2(val)) == 0  [ val == 1 ]
    // exp bits up to i are 1 -> int(log2(val)) == i+1
    BitChance inline &bitExp(int i)  {
        assert(i >= 0 && i < bits-1);
        return bit_exp[i];
    }
    BitChance inline &bitMant(int i) {
        assert(i >= 0 && i < bits);
        return bit_mant[i];
    }

    BitChance inline &bit(SymbolChanceBitType typ, int i = 0) {
        switch (typ) {
        default:
        case BIT_ZERO:
            return bitZero();
        case BIT_SIGN:
            return bitSign();
        case BIT_EXP:
            return bitExp(i);
        case BIT_MANT:
            return bitMant(i);
        }
    }
    SymbolChance() { // : bit_exp(bitsin-1), bit_mant(bitsin) {
        bitZero().set_12bit(ZERO_CHANCE);
        bitSign().set_12bit(SIGN_CHANCE);
//        printf("bits: %i\n",bits);
        for (int i=0; i<bits-1; i++) {
            bitExp(i).set_12bit(EXP_CHANCES[i]);
        }
        for (int i=0; i<bits; i++) {
            bitMant(i).set_12bit(MANT_CHANCES[i]);
        }
    }

    int scale() const {
        return bitZero().scale();
    }

#ifdef STATS
    ~SymbolChance() {
        global_symbol_stats.stats_zero += bit_zero.stats();
        global_symbol_stats.stats_sign += bit_sign.stats();
        for (int i = 0; i < bits - 1 && i < 17; i++) {
            global_symbol_stats.stats_exp[i] += bit_exp[i].stats();
        }
        for (int i = 0; i < bits && i < 18; i++) {
            global_symbol_stats.stats_mant[i] += bit_mant[i].stats();
        }
    }
#endif
};

template <typename SymbolCoder> int reader(SymbolCoder& coder, int bits) {
  int pos=0;
  int value=0;
  int b=1;
  while (pos++ < bits) {
    if (coder.read(BIT_MANT, pos)) value += b;
    b *= 2;
  }
  return value;
}

template <int bits, typename SymbolCoder> int reader(SymbolCoder& coder, int min, int max) ATTRIBUTE_HOT;

template <int bits, typename SymbolCoder> int reader(SymbolCoder& coder, int min, int max) {
    assert(min<=max);
    if (min == max) return min;

    bool sign;
    if (max >= 0 && min <= 0) {
      if (coder.read(BIT_ZERO)) return 0;
      if (min < 0) {
        if (max > 0) {
                sign = coder.read(BIT_SIGN);
                if (sign) min = 1; else max = -1;
        } else {sign = false; max=-1;}
      } else {sign = true; min=1;}
    } else {
        if (min<0) sign = false;
        else sign = true;
    }

    const int amin = (sign? min : -max);
    const int amax = (sign? max : -min);

    const int emax = maniac::util::ilog2(amax);
    int e = maniac::util::ilog2(amin);

    for (; e < emax; e++) {
        // if exponent >e is impossible, we are done
        if ((1 << (e+1)) > amax) break;
        if (coder.read(BIT_EXP,e)) break;
    }

    int have = (1 << e);
    int left = have-1;
    for (int pos = e; pos>0;) {
        int bit = 1;
        left ^= (1 << (--pos));
        int minabs1 = have | (1<<pos);
        int maxabs0 = have | left;
        if (minabs1 > amax) { // 1-bit is impossible
            bit = 0;
        } else if (maxabs0 >= amin) { // 0-bit and 1-bit are both possible
            bit = coder.read(BIT_MANT,pos);
        }
        have |= (bit << pos);
    }
    return (sign ? have : -have);
}

template <typename BitChance, typename RAC> class SimpleBitCoder {
    typedef typename BitChance::Table Table;

private:
    const Table table;
    BitChance ctx;
    RAC &rac;

public:
    SimpleBitCoder(RAC &racIn, int cut = 2, int alpha = 0xFFFFFFFF / 19) : rac(racIn), table(cut,alpha) {}

    void set(uint16_t chance) {
        ctx.set(chance);
    }
#ifdef HAS_ENCODER
    void write(bool bit);
#endif
    bool read() {
        bool bit = rac.read(ctx.get());
        ctx.put(bit, table);
        return bit;
    }
};

template <typename BitChance, typename RAC, int bits> class SimpleSymbolBitCoder {
    typedef typename BitChance::Table Table;

private:
    const Table &table;
    SymbolChance<BitChance,bits> &ctx;
    RAC &rac;

public:
    SimpleSymbolBitCoder(const Table &tableIn, SymbolChance<BitChance,bits> &ctxIn, RAC &racIn) : table(tableIn), ctx(ctxIn), rac(racIn) {}

#ifdef HAS_ENCODER
    void write(bool bit, SymbolChanceBitType typ, int i = 0);
#endif

    bool read(SymbolChanceBitType typ, int i = 0) {
        BitChance& bch = ctx.bit(typ, i);
        bool bit = rac.read_12bit_chance(bch.get_12bit());
        bch.put(bit, table);
        return bit;
    }
};

template <typename BitChance, typename RAC, int bits> class SimpleSymbolCoder {
    typedef typename BitChance::Table Table;

private:
    SymbolChance<BitChance,bits> ctx;
    const Table table;
    RAC &rac;

public:
    SimpleSymbolCoder(RAC& racIn, int cut = 2, int alpha = 0xFFFFFFFF / 19) :  table(cut,alpha), rac(racIn) {
    }

#ifdef HAS_ENCODER
    void write_int(int min, int max, int value);
    void write_int(int nbits, int value);
#endif

    int read_int(int min, int max) {
        SimpleSymbolBitCoder<BitChance, RAC, bits> bitCoder(table, ctx, rac);
        return reader<bits, SimpleSymbolBitCoder<BitChance, RAC, bits>>(bitCoder, min, max);
    }
    int read_int(int nbits) {
        assert (nbits <= bits);
        SimpleSymbolBitCoder<BitChance, RAC, bits> bitCoder(table, ctx, rac);
        return reader(bitCoder, nbits);
    }
};

#ifdef HAS_ENCODER
#include "symbol_enc.hpp"
#endif