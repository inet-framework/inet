/*
 * Copyright (C) 2006-2009 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Axel Neumann, Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include "BatmanMain.h"


/* clear the bits */
void Batman::bit_init(std::vector<TYPE_OF_WORD> &seq_bits) {
    int i;

    for (i = 0; i < (int)num_words; i++)
        seq_bits[i] = 0;
}

/* returns true if corresponding bit in given seq_bits indicates so and curr_seqno is within range of last_seqno */
uint8_t Batman::get_bit_status(std::vector<TYPE_OF_WORD> &seq_bits, uint16_t last_seqno, uint16_t curr_seqno) {
    int16_t diff, word_offset, word_num;

    diff = last_seqno- curr_seqno;
    if (diff < 0 || diff >= local_win_size) {
        return 0;
    } else {
        word_offset = (last_seqno - curr_seqno) % WORD_BIT_SIZE;    /* which position in the selected word */
        word_num = (last_seqno - curr_seqno) / WORD_BIT_SIZE;    /* which word */

        if (seq_bits[word_num] & 1<<word_offset)   /* get position status */
            return 1;
        else
            return 0;
    }
}

/* turn corresponding bit on, so we can remember that we got the packet */
void Batman::bit_mark(std::vector<TYPE_OF_WORD> &seq_bits, int32_t n) {
    int32_t word_offset, word_num;
    if (n<0 || n >= local_win_size) {            /* if too old, just drop it */
/*         printf("got old packet, dropping\n");*/
        return;
    }

/*     printf("mark bit %d\n", n); */

    word_offset = n%WORD_BIT_SIZE;    /* which position in the selected word */
    word_num = n/WORD_BIT_SIZE;    /* which word */

    seq_bits[word_num] |= 1<<word_offset;    /* turn the position on */
}

/* shift the packet array p by n places. */
void Batman::bit_shift(std::vector<TYPE_OF_WORD> &seq_bits, int32_t n) {
    int32_t word_offset, word_num;
    int32_t i;

/*    bit_print(seq_bits);*/
    if (n<=0)
        return;

    word_offset = n%WORD_BIT_SIZE;    /* shift how much inside each word */
    word_num = n/WORD_BIT_SIZE;    /* shift over how much (full) words */

    for (i=num_words-1; i>word_num; i--) {
        /* going from old to new, so we can't overwrite the data we copy from. *
          * left is high, right is low: FEDC BA98 7654 3210
         *                                      ^^ ^^
         *                             vvvv
         * ^^^^ = from, vvvvv =to, we'd have word_num==1 and
         * word_offset==WORD_BIT_SIZE/2 ????? in this example. (=24 bits)
         *
         * our desired output would be: 9876 5432 1000 0000
         * */

        seq_bits[i] =
                (seq_bits[i - word_num] << word_offset) +
                        /* take the lower port from the left half, shift it left to its final position */
                (seq_bits[i - word_num - 1] >>  (WORD_BIT_SIZE-word_offset));
                    /* and the upper part of the right half and shift it left to it's position */
        /* for our example that would be: word[0] = 9800 + 0076 = 9876 */
    }
    /* now for our last word, i==word_num, we only have the it's "left" half. that's the 1000 word in
     * our example.*/

    seq_bits[i] = (seq_bits[i - word_num] << word_offset);

    /* pad the rest with 0, if there is anything */
    i--;

    for (; i>=0; i--) {
        seq_bits[i] = 0;
    }
/*    bit_print(seq_bits); */
}


/* receive and process one packet, returns 1 if received seq_num is considered new, 0 if old  */
char Batman::bit_get_packet(std::vector<TYPE_OF_WORD> &seq_bits, int16_t seq_num_diff, int8_t set_mark)
{
    int i;

    /* we already got a sequence number higher than this one, so we just mark it. this should wrap around the integer just fine */
    if ((seq_num_diff < 0) && (seq_num_diff >= -local_win_size)) {
        if (set_mark)
            bit_mark(seq_bits, -seq_num_diff);

        return 0;
    }

    if ((seq_num_diff > local_win_size) || (seq_num_diff < -local_win_size)) {        /* it seems we missed a lot of packets or the other host restarted */

        if (seq_num_diff > local_win_size)
            debug_output(4) << "It seems we missed a lot of packets (" << seq_num_diff-1 << ") !\n";

        if (-seq_num_diff > local_win_size)
            debug_output(4) << "Other host probably restarted !\n";

        for (i=0; i<num_words; i++)
            seq_bits[i] = 0;

        if (set_mark)
            seq_bits[0] = 1;  /* we only have the latest packet */
    } else {
        bit_shift(seq_bits, seq_num_diff);

        if (set_mark)
            bit_mark(seq_bits, 0);
    }

    return 1;
}

/* count the hamming weight, how many good packets did we receive? just count the 1's ... */
int Batman::bit_packet_count(std::vector<TYPE_OF_WORD> &seq_bits)
{
    int i, hamming = 0;
    TYPE_OF_WORD word;

    for (i=0; i<num_words; i++) {
        word = seq_bits[i];

        while (word) {
            word &= word-1;   /* see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
            hamming++;
        }
    }

    return(hamming);
}

uint8_t Batman::bit_count(int32_t to_count)
{
    uint8_t hamming = 0;

    while (to_count) {
        to_count &= to_count-1;   /* see http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan */
        hamming++;
    }

    return(hamming);
}

