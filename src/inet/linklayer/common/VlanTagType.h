//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_VLANTAGTYPE_H
#define __INET_VLANTAGTYPE_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/DropEligibleTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"

namespace inet {

/**
 * The type of an IEEE 802.1Q VLAN tag: the C-tag (customer VLAN tag, TPID
 * 0x8100) or the S-tag (service VLAN tag, TPID 0x88A8) of IEEE 802.1ad. Each
 * tag type has its own request and indication tags (CVlanReq and SVlanReq,
 * and so on); the functions below find, add and remove the tag of a given
 * type, so that a module that works with one tag type, chosen by a parameter,
 * needs no branches.
 */
enum VlanTagType {
    VLAN_TAG_TYPE_C,
    VLAN_TAG_TYPE_S,
};

/**
 * Returns the tag type of a "vlanTagType" parameter value: "c" or "s".
 */
INET_API VlanTagType parseVlanTagType(const char *vlanTagType);

/**
 * Returns the protocol of the tag type: ieee8021qctag or ieee8021qstag.
 */
INET_API const Protocol *getVlanTagProtocol(VlanTagType vlanTagType);

template<typename B, typename C, typename S>
Ptr<const B> findTypedVlanTag(const Packet *packet, VlanTagType vlanTagType)
{
    if (vlanTagType == VLAN_TAG_TYPE_C)
        return packet->findTag<C>();
    else
        return packet->findTag<S>();
}

template<typename B, typename C, typename S>
Ptr<B> addTypedVlanTagIfAbsent(Packet *packet, VlanTagType vlanTagType)
{
    if (vlanTagType == VLAN_TAG_TYPE_C)
        return packet->addTagIfAbsent<C>();
    else
        return packet->addTagIfAbsent<S>();
}

template<typename B, typename C, typename S>
Ptr<B> removeTypedVlanTagIfPresent(Packet *packet, VlanTagType vlanTagType)
{
    if (vlanTagType == VLAN_TAG_TYPE_C)
        return packet->removeTagIfPresent<C>();
    else
        return packet->removeTagIfPresent<S>();
}

inline Ptr<const VlanTagBase> findVlanReq(const Packet *packet, VlanTagType vlanTagType) { return findTypedVlanTag<VlanTagBase, CVlanReq, SVlanReq>(packet, vlanTagType); }
inline Ptr<const VlanTagBase> findVlanInd(const Packet *packet, VlanTagType vlanTagType) { return findTypedVlanTag<VlanTagBase, CVlanInd, SVlanInd>(packet, vlanTagType); }
inline Ptr<VlanTagBase> addVlanReqIfAbsent(Packet *packet, VlanTagType vlanTagType) { return addTypedVlanTagIfAbsent<VlanTagBase, CVlanReq, SVlanReq>(packet, vlanTagType); }
inline Ptr<VlanTagBase> addVlanIndIfAbsent(Packet *packet, VlanTagType vlanTagType) { return addTypedVlanTagIfAbsent<VlanTagBase, CVlanInd, SVlanInd>(packet, vlanTagType); }
inline Ptr<VlanTagBase> removeVlanReqIfPresent(Packet *packet, VlanTagType vlanTagType) { return removeTypedVlanTagIfPresent<VlanTagBase, CVlanReq, SVlanReq>(packet, vlanTagType); }
inline Ptr<VlanTagBase> removeVlanIndIfPresent(Packet *packet, VlanTagType vlanTagType) { return removeTypedVlanTagIfPresent<VlanTagBase, CVlanInd, SVlanInd>(packet, vlanTagType); }

inline Ptr<const PcpTagBase> findPcpReq(const Packet *packet, VlanTagType vlanTagType) { return findTypedVlanTag<PcpTagBase, CPcpReq, SPcpReq>(packet, vlanTagType); }
inline Ptr<const PcpTagBase> findPcpInd(const Packet *packet, VlanTagType vlanTagType) { return findTypedVlanTag<PcpTagBase, CPcpInd, SPcpInd>(packet, vlanTagType); }
inline Ptr<PcpTagBase> addPcpReqIfAbsent(Packet *packet, VlanTagType vlanTagType) { return addTypedVlanTagIfAbsent<PcpTagBase, CPcpReq, SPcpReq>(packet, vlanTagType); }
inline Ptr<PcpTagBase> addPcpIndIfAbsent(Packet *packet, VlanTagType vlanTagType) { return addTypedVlanTagIfAbsent<PcpTagBase, CPcpInd, SPcpInd>(packet, vlanTagType); }
inline Ptr<PcpTagBase> removePcpReqIfPresent(Packet *packet, VlanTagType vlanTagType) { return removeTypedVlanTagIfPresent<PcpTagBase, CPcpReq, SPcpReq>(packet, vlanTagType); }

inline Ptr<DropEligibleTagBase> addDropEligibleIndIfAbsent(Packet *packet, VlanTagType vlanTagType) { return addTypedVlanTagIfAbsent<DropEligibleTagBase, CDropEligibleInd, SDropEligibleInd>(packet, vlanTagType); }
inline Ptr<DropEligibleTagBase> removeDropEligibleReqIfPresent(Packet *packet, VlanTagType vlanTagType) { return removeTypedVlanTagIfPresent<DropEligibleTagBase, CDropEligibleReq, SDropEligibleReq>(packet, vlanTagType); }

} // namespace inet

#endif

