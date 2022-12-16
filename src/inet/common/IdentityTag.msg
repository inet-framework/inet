//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

//
// This tag provides a bit level identity for binary data. It must attached to
// a packet or chunk as a region tag. The identity of a bit in the region can be
// calculated by adding the offset of said bit to the identityStart field of the
// tag.
//
// The tag is split and merged automatically when the data is split or merged.
// This allows one to have an effective globally unique identity for each bit
// in the simulation. This tag allows following the bits in the network through
// the whole lifetime of the simulation.
//
class IdentityTag extends TagBase
{
    uint64_t identityStart = -1;
}

cplusplus(IdentityTag) {{
  public:
    static uint64_t getNextIdentityStart(b length);

    virtual const Ptr<TagBase> changeRegion(b offsetDelta, b lengthDelta) const override {
        if (offsetDelta == b(0))
            return const_cast<IdentityTag *>(this)->shared_from_this();
        else {
            const auto& result = staticPtrCast<IdentityTag>(dupShared());
            result->identityStart += b(offsetDelta).get();
            return result;
        }
    }
}}

cplusplus(cc) {{
uint64_t IdentityTag::getNextIdentityStart(b length)
{
    static int handle = cSimulationOrSharedDataManager::registerSharedCounterName("inet::IdentityTag::nextIdentityStart");
    uint64_t& nextIdentityStart = getSimulationOrSharedDataManager()->getSharedCounter(handle);
    auto result = nextIdentityStart;
    nextIdentityStart += b(length).get();
    return result;
}
}}
