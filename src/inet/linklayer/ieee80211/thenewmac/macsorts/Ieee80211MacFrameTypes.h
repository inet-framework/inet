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
    TypeSubtype_asoc_req,
    TypeSubtype_asoc_rsp,
    TypeSubtype_reasoc_req,
    TypeSubtype_reasoc_rsp,
    TypeSubtype_probe_req,
    TypeSubtype_probe_rsp,
    TypeSubtype_beacon,
    TypeSubtype_atim,
    TypeSubtype_disasoc,
    TypeSubtype_auth,
    TypeSubtype_deauth,
    TypeSubtype_ps_poll,
    TypeSubtype_rts,
    TypeSubtype_cts,
    TypeSubtype_ack,
    TypeSubtype_cfend,
    TypeSubtype_cfend_ack,
    TypeSubtype_data,
    TypeSubtype_data_ack,
    TypeSubtype_data_poll,
    TypeSubtype_data_poll_ack,
    TypeSubtype_null_frame,
    TypeSubtype_cfack,
    TypeSubtype_cfpoll,
    TypeSubtype_cfpoll_ack
};

/*
 * BasicTypes defines the 2-bit frame type groups
 */
enum BasicType
{
    BasicType_control,
    BasicType_data,
    BasicType_management,
    BasicType_reserved
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
