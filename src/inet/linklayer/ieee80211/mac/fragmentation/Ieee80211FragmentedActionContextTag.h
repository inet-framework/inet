//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211FRAGMENTEDACTIONCONTEXTTAG_H
#define __INET_IEEE80211FRAGMENTEDACTIONCONTEXTTAG_H

#include "inet/common/TagBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Sender-local action context for an MPDU fragment whose packet content
 * contains only the common management header and its action-body slice.
 */
class INET_API Ieee80211FragmentedActionContextTag : public TagBase
{
  protected:
    Ptr<const Ieee80211ActionFrame> actionFrame;

  public:
    Ieee80211FragmentedActionContextTag() {}
    Ieee80211FragmentedActionContextTag(const Ieee80211FragmentedActionContextTag& other) : TagBase(other), actionFrame(other.actionFrame) {}

    virtual Ieee80211FragmentedActionContextTag *dup() const override { return new Ieee80211FragmentedActionContextTag(*this); }

    const Ptr<const Ieee80211ActionFrame>& getActionFrame() const { return actionFrame; }
    void setActionFrame(const Ptr<const Ieee80211ActionFrame>& actionFrame) { this->actionFrame = actionFrame; }
};

template<typename T>
const Ptr<const T> findFragmentedActionContext(const Packet *packet)
{
    if (auto contextTag = packet->findTag<Ieee80211FragmentedActionContextTag>())
        return dynamicPtrCast<const T>(contextTag->getActionFrame());
    else
        return dynamicPtrCast<const T>(packet->peekAtFront<Ieee80211MacHeader>());
}

} // namespace ieee80211
} // namespace inet

#endif
