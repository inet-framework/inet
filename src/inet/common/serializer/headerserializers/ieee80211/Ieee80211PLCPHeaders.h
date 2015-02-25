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

#ifndef __INET_IEEE80211PLCPHEADERS_H
#define __INET_IEEE80211PLCPHEADERS_H

namespace inet {
namespace serializer {

#define OFDM_PLCP_HEADER_LENGTH 5

enum PLCPHeaderType
{
    OFDM = 0,
    DSSS = 1,
    Infrared = 2,
    FHSS = 3,
    HRDSSS = 4,
    ERP = 5,
    HT = 6
};

/* 18.3.2 PLCP frame format */
struct Ieee80211OFDMPLCPHeader
{
    unsigned int rate : 4;
    unsigned int reserved : 1;
    unsigned int length : 12;
    unsigned int parity : 1;
    unsigned int tail : 6;
    unsigned int service : 16;
};


} /* namespace serializer */
} /* namespace inet */

#endif /* __INET_IEEE80211PLCPHEADERS_H */
