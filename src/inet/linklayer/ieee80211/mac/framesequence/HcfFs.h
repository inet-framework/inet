//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_HCFFS_H
#define __INET_HCFFS_H

#include "inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.h"

namespace inet {
namespace ieee80211 {

class INET_API HcfFs : public AlternativesFs {
    public:
        HcfFs();
        virtual ~HcfFs() { }

        virtual int selectHcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context);
        virtual int selectDataOrManagementSequence(AlternativesFs *frameSequence, FrameSequenceContext *context);
        virtual bool isSelfCtsNeeded(OptionalFs *frameSequence, FrameSequenceContext *context);
        virtual bool hasMoreTxOps(RepeatingFs *frameSequence, FrameSequenceContext *context);
        virtual bool hasMoreTxOpsAndMulticast(RepeatingFs *frameSequence, FrameSequenceContext *context);
};

} // namespace ieee80211
} // namespace inet

#endif
