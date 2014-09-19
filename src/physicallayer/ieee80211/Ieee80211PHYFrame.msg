//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

namespace inet::physicallayer;

// The rate1 and rate2 (RATE) indicate the convolutional code rate parameter
// k and n, respectively.
//
// The length (LENGTH) field is a 12-bit integer indicates the number of
// octets in the PSDU that the MAC is currently requesting the PHY to
// transmit.
//
// There are other fields which are not explicitly represented here,
// for e.g. Parity, Reserved, SERVICE, Pad Bits.
//
// The PSDU field must be contained as an encapsulated packet.
//
// This packet is intended to represent the whole PPDU frame in the
// IEEE 802.11 PHY. For detailed description, you may refer to 18.3.2 PLCP
// frame format, Figure 18-1—PPDU frame format in IEEE Std. 802.11-2012.
//
packet Ieee80211PHYFrame
{
    int rate1;
    int rate2;
    int length;
}
