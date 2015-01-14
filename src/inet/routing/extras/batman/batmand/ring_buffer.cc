/*
 * Copyright (C) 2007-2009 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner
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


#include "inet/routing/extras/batman/BatmanMain.h"

namespace inet {

namespace inetmanet {

void Batman::ring_buffer_set(std::vector<uint8_t> &tq_recv, uint8_t &tq_index, uint8_t value)
{
    tq_recv[tq_index] = value;
    tq_index = (tq_index + 1) % global_win_size;
}

uint8_t Batman::ring_buffer_avg(std::vector<uint8_t> &tq_recv)
{
    uint16_t count = 0;
    uint32_t sum = 0;

    for (auto & elem : tq_recv) {
        if (elem != 0) {
            count++;
            sum += elem;
        }
    }

    if (count == 0)
        return 0;

    return (uint8_t)(sum / count);
}

} // namespace inetmanet

} // namespace inet


