//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211MACFRAMETYPES_H
#define __INET_IEEE80211MACFRAMETYPES_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/*
 * Frame type sorts - p. 2371
 */
enum TypeSubtype
{
    TypeSubtype_asoc_req      = 0b00000000,
    TypeSubtype_asoc_rsp      = 0b00001000,
    TypeSubtype_reasoc_req    = 0b00000100,
    TypeSubtype_reasoc_rsp    = 0b00001100,
    TypeSubtype_probe_req     = 0b00000010,
    TypeSubtype_probe_rsp     = 0b00001010,
    TypeSubtype_beacon        = 0b00000001,
    TypeSubtype_atim          = 0b00001001,
    TypeSubtype_disasoc       = 0b00000101,
    TypeSubtype_auth          = 0b00001101,
    TypeSubtype_deauth        = 0b00000011,
    TypeSubtype_ps_poll       = 0b00100101,
    TypeSubtype_rts           = 0b00101101,
    TypeSubtype_cts           = 0b00100011,
    TypeSubtype_ack           = 0b00101011,
    TypeSubtype_cfend         = 0b00100111,
    TypeSubtype_cfend_ack     = 0b00101111,
    TypeSubtype_data          = 0b00010000,
    TypeSubtype_data_ack      = 0b00011000,
    TypeSubtype_data_poll     = 0b00010100,
    TypeSubtype_data_poll_ack = 0b00011100,
    TypeSubtype_null_frame    = 0b00010010,
    TypeSubtype_cfack         = 0b00011010,
    TypeSubtype_cfpoll        = 0b00010110,
    TypeSubtype_cfpoll_ack    = 0b00011110
};

/*
 * BasicTypes defines the 2-bit frame type groups
 */
enum BasicType
{
    BasicType_control    = 0b00100000,
    BasicType_data       = 0b00010000,
    BasicType_management = 0b00000000,
    BasicType_reserved   = 0b00110000
};

// TODO: move
enum StatusCode
{
    StatusCode_successful,
    StatusCode_unspec_fail,
    StatusCode_unsup_cap,
    StatusCode_reasoc_no_asoc,
    StatusCode_fail_other,
    StatusCode_unsupt_alg,
    StatusCode_auth_seq_fail,
};

enum Capability
{
    Capability_cEss,
    Capability_cIbss,
    Capability_cPollable,
    Capability_cPollReq,
    Capability_cPrivacy,
    Capability_cShortPreamble,
    Capability_cPBCC,
    Capability_cChannelAgility,
    Capability_cShortSlot,
    Capability_cDsssOfdm
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACFRAMETYPES_H
