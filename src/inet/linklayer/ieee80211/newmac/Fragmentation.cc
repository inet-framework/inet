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

#include "Fragmentation.h"

namespace inet {
namespace ieee80211 {

std::vector<Ieee80211DataOrMgmtFrame*> FragmentationNotSupported::fragment(Ieee80211DataOrMgmtFrame *frame)
{
    throw cRuntimeError("Fragmentation not supported");
}

Ieee80211DataOrMgmtFrame *ReassemblyNotSupported::addFragment(Ieee80211DataOrMgmtFrame *frame)
{
    throw cRuntimeError("Fragmentation reassembly not supported");
}

void ReassemblyNotSupported::purge(const MACAddress& address, int tid, int startSeqNumber, int endSeqNumber)
{
}

} // namespace ieee80211
} // namespace inet
