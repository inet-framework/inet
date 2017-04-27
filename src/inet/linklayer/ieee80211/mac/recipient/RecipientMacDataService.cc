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

#include "inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.h"
#include "RecipientMacDataService.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientMacDataService);

void RecipientMacDataService::initialize()
{
    duplicateRemoval = new LegacyDuplicateRemoval();
    basicReassembly = new BasicReassembly();
}

Ieee80211DataOrMgmtFrame* RecipientMacDataService::defragment(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame)
{
    if (auto completeFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(basicReassembly->addFragment(dataOrMgmtFrame)))
        return completeFrame;
    else
        return nullptr;
}

std::vector<Ieee80211Frame*> RecipientMacDataService::dataOrMgmtFrameReceived(Ieee80211DataOrMgmtFrame* frame)
{
    if (duplicateRemoval && duplicateRemoval->isDuplicate(frame)) {
        delete frame;
        return std::vector<Ieee80211Frame*>();
    }
    Ieee80211DataOrMgmtFrame *defragmentedFrame = nullptr;
    if (basicReassembly) { // FIXME: defragmentation
        defragmentedFrame = defragment(frame);
    }
    return defragmentedFrame != nullptr ? std::vector<Ieee80211Frame*>({defragmentedFrame}) : std::vector<Ieee80211Frame*>();
}

std::vector<Ieee80211Frame*> RecipientMacDataService::dataFrameReceived(Ieee80211DataFrame* dataFrame)
{
    return dataOrMgmtFrameReceived(dataFrame);
}

std::vector<Ieee80211Frame*> RecipientMacDataService::managementFrameReceived(Ieee80211ManagementFrame* mgmtFrame)
{
    return dataOrMgmtFrameReceived(mgmtFrame);
}

std::vector<Ieee80211Frame*> RecipientMacDataService::controlFrameReceived(Ieee80211Frame* controlFrame)
{
    return std::vector<Ieee80211Frame*>(); // has nothing to do
}

RecipientMacDataService::~RecipientMacDataService()
{
    delete duplicateRemoval;
    delete basicReassembly;
}

} /* namespace ieee80211 */
} /* namespace inet */
