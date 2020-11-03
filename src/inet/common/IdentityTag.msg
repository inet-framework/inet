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
    static int64_t nextIdentityStart;
    static int64_t getNextIdentityStart(b length) { auto result = nextIdentityStart; nextIdentityStart += b(length).get(); return result; }

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
int64_t IdentityTag::nextIdentityStart = 0;
}}
