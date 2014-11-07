//
// Copyright (C) 2014 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
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


#include "DRRVLANQueue3.h"

Define_Module(DRRVLANQueue3);

DRRVLANQueue3::DRRVLANQueue3()
{
}

DRRVLANQueue3::~DRRVLANQueue3()
{
#ifndef NDEBUG
    for (int i = 0; i < numFlows; i++)
    {
        delete conformedCounterVector[i];
        delete nonconformedCounterVector[i];
    }
#endif
}

void DRRVLANQueue3::initialize()
{
    DRRVLANQueue::initialize();

     // DRR scheduler
     conformedCounters.assign(numFlows, 0);
     nonconformedCounters.assign(numFlows, 0);

     // statistics
     numConformedBitsSent.assign(numFlows, 0.0);
     numNonconformedBitsSent.assign(numFlows, 0.0);
     numConformedPktsSent.assign(numFlows, 0);
     numNonconformedPktsSent.assign(numFlows, 0);

     // debugging
#ifndef NDEBUG
     for (int i=0; i < numFlows; i++)
     {
         cOutVector *vector;
         std::stringstream vname;

         vname << "conformed bytes for flow[" << i << "]";
         vector = new cOutVector((vname.str()).c_str());
         conformedCounterVector.push_back(vector);

         vname.str(std::string());  // clear string stream
         vname << "non-conformed bytes for flow[" << i << "]";
         vector = new cOutVector((vname.str()).c_str());
         nonconformedCounterVector.push_back(vector);
     }
#endif
}

void DRRVLANQueue3::handleMessage(cMessage *msg)
{
    if (warmupFinished == false)
    {   // start statistics gathering once the warm-up period has passed.
        if (simTime() >= simulation.getWarmupPeriod()) {
            warmupFinished = true;
            for (int i = 0; i < numFlows; i++)
            {
                numPktsReceived[i] = voq[i]->getLength();   // take into account the packets already in VOQs
            }
        }
    }

    if (!msg->isSelfMessage())
    {   // a packet arrives
        int flowIndex = classifier->classifyPacket(msg);
        int pktLength = PK(msg)->getBitLength();
// DEBUG
        ASSERT(pktLength > 0);
// DEBUG
        int pktByteLength = PK(msg)->getByteLength();
        int color = tbm[flowIndex]->meterPacket(msg);   // result of metering; 0 for conformed and 1 for non-conformed packet
//        cQueue *queue = voq[flowIndex];
//        bool queueEmpty = queue->isEmpty(); // status of queue before enqueueing the packet
        if (warmupFinished == true)
        {
            numPktsReceived[flowIndex]++;
            if (color == 0)
            {   // packet is conformed
                numPktsConformed[flowIndex]++;
            }
        }

        if (packetRequested > 0)
        {
            packetRequested--;
#ifndef NDEBUG
            pktReqVector.record(packetRequested);
#endif
            if (warmupFinished == true)
            {
                numBitsSent[flowIndex] += pktLength;
                numPktsSent[flowIndex]++;
                if (color == 0) {
                    numConformedBitsSent[flowIndex] += pktLength;
                    numConformedPktsSent[flowIndex]++;
                } else {
                    numNonconformedBitsSent[flowIndex] += pktLength;
                    numNonconformedPktsSent[flowIndex]++;
                }
            }
            sendOut(msg);
        }
        else
        {
            bool dropped = enqueue(msg);
            if (dropped)
            {
                if (warmupFinished == true)
                {
                    numPktsDropped[flowIndex]++;
                }
                if (color == 0)
                {   // exchange of the values of counters to emulate priority queueing
                    if (nonconformedCounters[flowIndex] >= pktByteLength)
                    {
                        conformedCounters[flowIndex] += pktByteLength;
                        nonconformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
                        conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
                        nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                        // update active list for conformed packets
                        IntList::iterator iter = std::find(conformedList.begin(), conformedList.end(), flowIndex);
                        if (iter == conformedList.end())
                        {   // add flow index to active list
                            conformedList.push_back(flowIndex);
                        }

                        // update active list for non-conformed packets
                        if (nonconformedCounters[flowIndex] == 0)
                        {
                            nonconformedList.remove(flowIndex);
                        }
                    }
                }
            }
            else
            {
                if (color == 0)
                {   // packet is conformed
                    conformedCounters[flowIndex] += pktByteLength;
#ifndef NDEBUG
                    conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
#endif
                    IntList::iterator iter = std::find(conformedList.begin(), conformedList.end(), flowIndex);
                    if (iter == conformedList.end())
                    {   // add flow index to active list
                        conformedList.push_back(flowIndex);
                    }
                }
                else
                {   // packet is not conformed
                    nonconformedCounters[flowIndex] += pktByteLength;
#ifndef NDEBUG
                    nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                    IntList::iterator iter = std::find(nonconformedList.begin(), nonconformedList.end(), flowIndex);
                    if (iter == nonconformedList.end())
                    {   // add flow index to active list
                        nonconformedList.push_back(flowIndex);
                    }
                }
//            if (queueEmpty)
//            {
//                if (packetRequested > 0)
//                {
//                    cMessage *dequeued_msg = dequeue();
//                    if (dequeued_msg != NULL)
//                    {
//                        packetRequested--;
//#ifndef NDEBUG
//                        pktReqVector.record(packetRequested);
//#endif
//                        if (warmupFinished == true)
//                        {
//                            numBitsSent[flowIndex] += PK(dequeued_msg)->getBitLength();
//                            numPktsSent[flowIndex]++;
//                        }
//                        sendOut(dequeued_msg);
//                    }
//                }
//            }
            }
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numPktsReceived[flowIndex], numPktsDropped[flowIndex]);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }   // end of if () for self message
    else
    {   // self message is not expected; something wrong.
        error("%s::handleMessage: Unexpected self message", getFullPath().c_str());
    }
}

cMessage *DRRVLANQueue3::dequeue()
{
    cMessage *msg = (cMessage *)NULL;

    // check first the list of queues with non-zero conformed bytes
    while (!conformedList.empty()) {
        int flowIndex = conformedList.front();
        conformedList.pop_front();
        // DEBUG
        ASSERT(!voq[flowIndex]->isEmpty());
        // DEBUG
        int pktByteLength = PK(voq[flowIndex]->front())->getByteLength();
        if (conformedCounters[flowIndex] >= pktByteLength)
        {   // serve the packet
            msg = (cMessage *)voq[flowIndex]->pop();
            voqCurrentSize[flowIndex] -= pktByteLength;
            conformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
            conformedCounterVector[flowIndex]->record(conformedCounters[flowIndex]);
#endif
            // update statistics
            numConformedBitsSent[flowIndex] += 8*pktByteLength;
            numConformedPktsSent[flowIndex]++;

            // check whether the conformed counter value is enough for the HOL packet
            if (!voq[flowIndex]->isEmpty() && (conformedCounters[flowIndex] >= PK(voq[flowIndex]->front())->getByteLength()))
            {
                conformedList.push_back(flowIndex);
            }
            return msg;
        }
        // else
        // {
        //     break;
        // }

        // if (!voq[flowIndex]->isEmpty() && (conformedCounters[flowIndex] > 0))
        // {
        //     conformedList.push_back(flowIndex);
        // }
    }   // end of while ()

    // check then the list of queues with non-zero non-conformed bytes and do regular DRR scheduling

    if (nonconformedList.empty())
    {
        continuation = false;
        return (msg);
    }
    else
    {
        while (!nonconformedList.empty())
        {
            int flowIndex = nonconformedList.front();
            nonconformedList.pop_front();
            // DEBUG
            ASSERT(!voq[flowIndex]->isEmpty());
            // DEBUG
            deficitCounters[flowIndex] += continuation ? 0 : quanta[flowIndex];
            continuation = false;   // reset the flag

            int pktByteLength = PK(voq[flowIndex]->front())->getByteLength();
            if ((deficitCounters[flowIndex] >= pktByteLength) && (nonconformedCounters[flowIndex] >= pktByteLength))
            {   // serve the packet
                msg = (cMessage *)voq[flowIndex]->pop();
                voqCurrentSize[flowIndex] -= pktByteLength;
                deficitCounters[flowIndex] -= pktByteLength;
                nonconformedCounters[flowIndex] -= pktByteLength;
#ifndef NDEBUG
                nonconformedCounterVector[flowIndex]->record(nonconformedCounters[flowIndex]);
#endif
                // update statistics
                numNonconformedBitsSent[flowIndex] += 8*pktByteLength;
                numNonconformedPktsSent[flowIndex]++;
            }

            // check whether the deficit and non-conformed counter values are enough for the HOL packet
            if (!voq[flowIndex]->isEmpty())
            {
                pktByteLength = PK(voq[flowIndex]->front())->getByteLength();                
                if (nonconformedCounters[flowIndex] >= pktByteLength)
                {
                    if (deficitCounters[flowIndex] >= pktByteLength)
                    {   // set the flag and put the index back to the front of the list
                        continuation = true;
                        nonconformedList.push_front(flowIndex);
                    }
                    else
                    {
                        nonconformedList.push_back(flowIndex);
                    }
                }
                else
                {
                    deficitCounters[flowIndex] = 0;
                }
            }
            else
            {
                deficitCounters[flowIndex] = 0;
            }

            if (msg != NULL)
            {
                break;  // from the while loop
            }
        } // end of while()
    }

    return (msg);   // just in case
}

void DRRVLANQueue3::finish()
{
    DRRVLANQueue::finish();

    for (int i=0; i < numFlows; i++)
    {
        std::stringstream ss_conformed_bits_sent, ss_conformed_pkts_sent, ss_nonconformed_bits_sent, ss_nonconformed_pkts_sent;
        ss_conformed_bits_sent << "conformed bits sent from flow[" << i << "]";
        ss_conformed_pkts_sent << "conformed packets sent from flow[" << i << "]";
        ss_nonconformed_bits_sent << "non-conformed bits sent from flow[" << i << "]";
        ss_nonconformed_pkts_sent << "non-conformed packets sent from flow[" << i << "]";
        recordScalar((ss_conformed_bits_sent.str()).c_str(), numConformedBitsSent[i]);
        recordScalar((ss_conformed_pkts_sent.str()).c_str(), numConformedPktsSent[i]);
        recordScalar((ss_nonconformed_bits_sent.str()).c_str(), numNonconformedBitsSent[i]);
        recordScalar((ss_nonconformed_pkts_sent.str()).c_str(), numNonconformedPktsSent[i]);

        // debugging
#ifndef NDEBUG
        std::stringstream ss_debug_info;
        ss_debug_info << "DEBUG: total - conformed/non-conformed bits from flow[" << i << "]";
        recordScalar((ss_debug_info.str()).c_str(), numBitsSent[i] - (numConformedBitsSent[i] + numNonconformedBitsSent[i]));
#endif
    }
}
