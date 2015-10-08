//
// Copyright (C) 2015 Andras Varga
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef __INET_FRAGMENTATION_H
#define __INET_FRAGMENTATION_H

#include "IFragmentation.h"

namespace inet {
namespace ieee80211 {

class INET_API FragmentationNotSupported : public IFragmenter
{
    public:
        virtual std::vector<Ieee80211DataOrMgmtFrame*> fragment(Ieee80211DataOrMgmtFrame *frame);
};

class INET_API ReassemblyNotSupported : public IReassembly
{
    public:
        virtual Ieee80211DataOrMgmtFrame *addFragment(Ieee80211DataOrMgmtFrame *frame);
        virtual void purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber);
};

} // namespace ieee80211
} // namespace inet

#endif
