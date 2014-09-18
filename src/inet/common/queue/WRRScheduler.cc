//
// Copyright (C) 2012 Andras Varga
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

#include "inet/common/queue/WRRScheduler.h"
#include "inet/common/INETUtils.h"

namespace inet {

Define_Module(WRRScheduler);

WRRScheduler::~WRRScheduler()
{
    delete[] weights;
    delete[] buckets;
}

void WRRScheduler::initialize()
{
    SchedulerBase::initialize();

    numInputs = gateSize("in");
    ASSERT(numInputs == (int)inputQueues.size());

    weights = new int[numInputs];
    buckets = new int[numInputs];

    cStringTokenizer tokenizer(par("weights"));
    int i;
    for (i = 0; i < numInputs && tokenizer.hasMoreTokens(); ++i)
        buckets[i] = weights[i] = (int)utils::atoul(tokenizer.nextToken());

    if (i < numInputs)
        throw cRuntimeError("Too few values given in the weights parameter.");
    if (tokenizer.hasMoreTokens())
        throw cRuntimeError("Too many values given in the weights parameter.");
}

bool WRRScheduler::schedulePacket()
{
    bool allQueueIsEmpty = true;
    for (int i = 0; i < numInputs; ++i) {
        if (!inputQueues[i]->isEmpty()) {
            allQueueIsEmpty = false;
            if (buckets[i] > 0) {
                buckets[i]--;
                inputQueues[i]->requestPacket();
                return true;
            }
        }
    }

    if (allQueueIsEmpty)
        return false;

    bool packetRequested = false;
    for (int i = 0; i < numInputs; ++i) {
        buckets[i] = weights[i];
        if (!packetRequested && buckets[i] > 0 && !inputQueues[i]->isEmpty()) {
            buckets[i]--;
            inputQueues[i]->requestPacket();
            packetRequested = true;
        }
    }

    return packetRequested;
}

} // namespace inet

