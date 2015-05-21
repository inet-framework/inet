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

#ifndef __INET_IEEE80211MACENUMERATEDMACSTATYPES_H
#define __INET_IEEE80211MACENUMERATEDMACSTATYPES_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/*
 * Enumerated types used within the MAC state machines
 */

enum ChangeType /* type of change due at the next boundary */
{
    ChangeType_dwell, /* dwell (only with FH PHY) */
    ChangeType_mocp /* medium occupancy (only with PCF) */
};

enum Imed /* priority for queuing MMPDUs, relative to MSDUs */
{
    Imed_head, /* place MMPDU at head of transmit queue */
    Imed_norm /* place MMPDU at tail of transmit queue */
};

enum NavSrc /* source of duration in SetNav & ClearNav signals */
{
    NavSrc_rts,  /* RTS frame */
    NavSrc_cfpBss, /* start/end of CFP in own BSS */
    NavSrc_cfendBss,
    NavSrc_cfpOther,  /* start/end of CFP in other BSS */
    NavSrc_cfendOther,
    NavSrc_cswitch, /* channel switch */
    NavSrc_misc, /* durId from other frame types */
    NavSrc_nosrc  /* nonreception events */
};

enum PsMode /* power save mode of a station (PsResponse signal) */
{
    PsMode_sta_active,
    PsMode_power_save,
    PsMode_unknown,
};

enum PsState /* power save state of this station */
{
    PsState_awake,
    PsState_doze
};

enum StateErr /* requests disasoc or deauth (MmIndicate signal) */
{
    StateErr_noerr,
    StateErr_class2,
    StateErr_class3
};

enum StationState /* asoc/auth state of sta (SsResponse signal) */
{
    StationState_not_auth,
    StationState_auth_open,
    StationState_auth_key,
    StationState_asoc,
    StationState_dis_asoc
};


enum TxResult  /* transmission attempt status (PduConfirm signal) */
{
    TxResult_successful,
    TxResult_partial,
    TxResult_retryLimit,
    TxResult_txLifetime,
    TxResult_atimAck,
    TxResult_atimNak
};


/*
 * Enumerated types used in Mac and Mlme service primitives
 */
enum AuthType /* <authentication type> parm in Mlme primitives */
{
    AuthType_open_system,
    AuthType_shared_key
};

enum BssType /* <BSS type> parameter & BSS description element */
{
    BssType_infrastructre,
    BssType_indepedent,
    BssType_any_bss,
};

enum CfPriority /* <priority> parameter of various requests */
{
    CfPriority_contention,
    CfPriority_contentionFree
};

enum MibStatus /* <status> parm of Mlme/Plme Get/Set.confirm */
{
    MibStatus_succes,
    MibStatus_invalid,
    MibStatus_write_only,
    MibStatus_read_only
};

enum MlmeStatus /* <status> parm of Mlme operation confirm */
{
    MlmeStatus_succes,
    MlmeStatus_invalid,
    MlmeStatus_timeout,
    MlmeStatus_refused,
    MlmeStatus_tomany_req,
    MlmeStatus_already_bss
};


enum PwrSave /* <power save mode> parameter of MlmePowerMgt */
{
    PwrSave_sta_active,
    PwrSave_power_save
};

enum Routing /* <routing info> parameter for MAC data service */
{
    Routing_null_rt
};


enum RxStatus /* <reception status> parm of MaUnitdata indication */
{
    RxStatus_rx_success,
    RxStatus_rx_failure
};

enum ScanType /* <scan type> parameter of MlmeScan.request */
{
    ScanType_active_scan,
    ScanType_passive_scan
};

enum ServiceClass /* <service class> parameter for MaUnitdata */
{
    ServiceClass_reordable,
    ServiceClass_strictlyOrdered
};

enum TxStatus /* <transmission status> parm of MaUnitdataStatus */
{
    TxStatus_successful,
    TxStatus_retryLimit,
    TxStatus_txLifetime,
    TxStatus_noBss,
    TxStatus_excessiveDataLength,
    TxStatus_nonNullSourceRouting,
    TxStatus_nsupportedPriority,
    TxStatus_unavailablePriority,
    TxStatus_unsupportedServiceClass,
    TxStatus_unavailableServiceClass,
    TxStatus_unavailableKeyMapping
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACENUMERATEDMACSTATYPES_H
