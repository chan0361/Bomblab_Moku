#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    if (!out || !values || cap == 0 || count == 0 || count > MAX_FIELDS) return 1;
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return 0;
}

static uint32_t rotl32(uint32_t value, unsigned amount) {
    return (value << amount) | (value >> (32 - amount));
}

static uint32_t rotr32(uint32_t value, unsigned amount) {
    return (value >> amount) | (value << (32 - amount));
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t step1 = (middle ^ UINT32_C(0xc569)) + 2 * high;
    uint32_t step0 = (low + 8 * high) ^ UINT32_C(0x60b1);
    uint32_t values[] = {(step1 + UINT32_C(0x2178)) ^ step0};
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = (uint32_t)((challenge >> 6) & 31);
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t part = (uint32_t)((challenge >> (i + 15)) & 15);
        values[i] = 3 * values[i - 1] + (part ^ (i + 5));
    }
    return write_answer(out, cap, values, 6);
}

static uint32_t phase_3_case(unsigned index, uint32_t value) {
    switch (index) {
    case 0: return 5 * value + UINT32_C(0x358b3);
    case 1: return (value ^ UINT32_C(0xff97)) + (value >> 1);
    case 2: return 4 * value ^ UINT32_C(0xde72);
    case 3: return (value ^ UINT32_C(0x475b)) + (value >> 1);
    case 4: return 5 * value + UINT32_C(0xb89);
    case 5: return 4 * value ^ UINT32_C(0x4122);
    case 6: return 5 * value + UINT32_C(0x1997e);
    default: return 3 * value + UINT32_C(0x506f);
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t index = (uint32_t)challenge & 7;
    uint32_t value = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t values[] = {index, phase_3_case(index, value)};
    return write_answer(out, cap, values, 2);
}

static uint32_t phase_4_walk(uint32_t target, uint32_t low, uint32_t high,
                             uint32_t salt) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + 3) ^ salt;
    if (target < middle) {
        uint32_t child = phase_4_walk(target, low, middle - 1, salt);
        return ((3 * child) ^ (middle + salt)) ^ UINT32_C(0x0a);
    }
    uint32_t child = phase_4_walk(target, middle + 1, high, salt);
    return ((3 * child) ^ (middle + salt)) ^ UINT32_C(0x13);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t target = (uint32_t)challenge & 127;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t values[] = {phase_4_walk(target, 0, 127, salt)};
    return write_answer(out, cap, values, 1);
}

struct node_data {
    uint32_t tag;
    uint32_t value;
};

static const struct node_data phase_5_nodes[] = {
    {0x017e, 1626}, {0x1d3e, 859}, {0xa311, 638}, {0x9f62, 1091},
    {0x39e3, 2144}, {0x0dd6, 2853}, {0x97da, 1918}, {0xcc32, 77},
    {0x600d, 2698}, {0xb4bb, 1346}, {0xdbbc, 283}, {0x5ed2, 2397}
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    unsigned start = (unsigned)(challenge & 7);
    unsigned steps = (unsigned)((challenge >> 3) & 3);
    uint32_t fold = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    fold = (fold ^ phase_5_nodes[start].tag ^ UINT32_C(0x298a))
           + phase_5_nodes[start].value;
    for (unsigned i = 1; i <= steps; ++i) {
        fold = (fold ^ phase_5_nodes[start + i].tag)
               + phase_5_nodes[start + i].value;
    }
    uint32_t values[] = {fold, phase_5_nodes[start + steps].value};
    return write_answer(out, cap, values, 2);
}

static uint32_t phase_6_op(unsigned index, uint32_t value) {
    switch (index) {
    case 0: return rotr32(value, 7);
    case 1: return (4 * value + UINT32_C(0xd840)) ^ (5 * value);
    case 2: return (value + UINT32_C(0x1355)) ^ (value >> 1);
    case 3: return rotr32(value, 7);
    case 4: return (value ^ (value >> 7)) + UINT32_C(0x2788);
    case 5: return rotl32(value, 3) ^ UINT32_C(0x34c3);
    case 6: return rotl32(value, 3) ^ UINT32_C(0x34c3);
    default: return (value ^ UINT32_C(0xcfb2)) + 8 * value;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t input = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    unsigned first = (unsigned)(challenge & 7);
    unsigned second = (unsigned)((challenge >> 3) & 7);
    uint32_t intermediate = phase_6_op(first, input) ^ UINT32_C(0xde0d);
    uint32_t values[] = {phase_6_op(second, intermediate) + UINT32_C(0x9199)};
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    if (!out || cap == 0) return 1;
    uint32_t first = (uint32_t)context & 3;
    uint32_t challenge_mix = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_mix = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t mixed = challenge_mix ^ context_mix;
    uint32_t secret_mix = ((mixed >> 1) ^ UINT32_C(0x884b))
                          + (mixed ^ UINT32_C(0x472f));
    uint32_t second = secret_mix ^ (first * UINT32_C(0x472f))
                      ^ (uint32_t)(context >> 16);
    uint32_t third = second ^ (context_mix + first + UINT32_C(0x884b)
                               + (challenge_mix >> 1));
    int size = snprintf(out, cap,
                        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                        first, second, third);
    return size < 0 || (size_t)size >= cap;
}
