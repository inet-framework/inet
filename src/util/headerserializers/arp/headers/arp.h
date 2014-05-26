//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef ARP_H_
#define ARP_H_

#ifdef _MSC_VER
#define __PACKED__
#else
#define __PACKED__  __attribute__((packed))
#endif

struct arphdr {
        u_short ar_hrd;                  // Hardware type (16 bits)
        u_short ar_pro;                  // Protocol type (16 bits)
        char ar_hln;                     // Byte length of each hardware address (n) (8 bits)
        char ar_pln;                     // Byte length of each protocol address (m) (8 bits)
        u_short ar_op;                   // Operation code (16 bits)
        u_char ar_sha[ETHER_ADDR_LEN];   // source hardware address (n bytes)
        u_int32_t ar_spa;                // source protocol address (m bytes)
        u_char ar_tha[ETHER_ADDR_LEN];   // target hardware address (n bytes)
        u_int32_t ar_tpa;                // target protocol address (m bytes)
} __PACKED__;



#endif /* ARP_H_ */
