//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Some 8-bit SAP values for IEEE 802.x LLC headers.
//
enum SapCode
{
    SAP_IBM_SNA = 0x04;
    SAP_IP = 0x06;
    SAP_3COM = 0x80;
    SAP_SNAP = 0xAA;
    SAP_BANYAN = 0xBC;
    SAP_NOVELL_IPX = 0xE0;
    SAP_LAN_MANAGER = 0xF4;
    SAP_CLNS = 0xFE;
    SAP_STP = 0x42;
}

class Ieee802SapTagBase extends TagBase
{
    int ssap;        // used with IEEE 802 LLC (see ~EtherFrameWithLlc)
    int dsap;        // used with IEEE 802 LLC (see ~EtherFrameWithLlc)
}

class Ieee802SapReq extends Ieee802SapTagBase
{
}

class Ieee802SapInd extends Ieee802SapTagBase
{
}
