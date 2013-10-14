/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "MACRelayUnitNP.h"
#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "MACAddress.h"


Define_Module( MACRelayUnitNP );


/* unused for now
static std::ostream& operator<< (std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}
*/

MACRelayUnitNP::MACRelayUnitNP()
{
    endProcEvents = NULL;
    numCPUs = 0;
}

MACRelayUnitNP::~MACRelayUnitNP()
{
    for (int i=0; i<numCPUs; i++)
    {
        cMessage *endProcEvent = endProcEvents[i];
        EtherFrame *etherFrame = (EtherFrame *)endProcEvent->getContextPointer();
        if (etherFrame)
        {
            endProcEvent->setContextPointer(NULL);
            delete etherFrame;
        }
        cancelAndDelete(endProcEvent);
    }
    delete [] endProcEvents;
}

void MACRelayUnitNP::initialize(int stage)
{
    MACRelayUnitBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        bufferLevel.setName("buffer level");
        queue.setName("queue");

        numProcessedFrames = numDroppedFrames = 0;
        WATCH(numProcessedFrames);
        WATCH(numDroppedFrames);

        numCPUs = par("numCPUs");

        processingTime = par("processingTime");
        bufferSize = par("bufferSize");
        highWatermark = par("highWatermark");
        pauseUnits = par("pauseUnits");

        bufferUsed = 0;
        WATCH(bufferUsed);

        endProcEvents = new cMessage *[numCPUs];
        for (int i=0; i<numCPUs; i++)
        {
            char msgname[40];
            sprintf(msgname, "endProcessing-cpu%d", i);
            endProcEvents[i] = new cMessage(msgname, i);
        }

        EV_DEBUG << "Parameters of (" << getClassName() << ") " << getFullPath() << "\n";
        EV_DEBUG << "number of processors: " << numCPUs << "\n";
        EV_DEBUG << "processing time: " << processingTime << "\n";
        EV_DEBUG << "ports: " << numPorts << "\n";
        EV_DEBUG << "buffer size: " << bufferSize << "\n";
        EV_DEBUG << "address table size: " << addressTableSize << "\n";
        EV_DEBUG << "aging time: " << agingTime << "\n";
        EV_DEBUG << "high watermark: " << highWatermark << "\n";
        EV_DEBUG << "pause time: " << pauseUnits << "\n";
        EV_DEBUG << "\n";
    }
}

void MACRelayUnitNP::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
    {
        if(!isOperational)
        {
            EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropped it\n";
            delete msg;
            return;
        }
        // Frame received from MAC unit
        EV_INFO << "Received " << msg << " from lower layer.\n";
        handleIncomingFrame(check_and_cast<EtherFrame *>(msg));
    }
    else
    {
        if(!isOperational)
            throw cRuntimeError("model error: self message arrived when module status is DOWN");
        // Self message signal used to indicate a frame has finished processing
        processFrame(msg);
    }
}

void MACRelayUnitNP::handleIncomingFrame(EtherFrame *frame)
{
    // If buffer not full, insert payload frame into buffer and process the frame in parallel.

    long length = frame->getByteLength();
    if (length + bufferUsed < bufferSize)
    {
        bufferUsed += length;

        // send PAUSE if above watermark
        if (pauseUnits>0 && highWatermark>0 && bufferUsed>=highWatermark)
            sendPauseFramesIfNeeded(pauseUnits);

        // assign frame to a free CPU (if there is one)
        int i;
        for (i=0; i<numCPUs; i++)
            if (!endProcEvents[i]->isScheduled())
                break;
        if (i==numCPUs)
        {
            EV_DETAIL << "All CPUs busy, enqueueing incoming frame " << frame << " for later processing\n";
            queue.insert(frame);
        }
        else
        {
            EV_DETAIL << "Idle CPU-" << i << " starting processing of incoming frame " << frame << endl;
            cMessage *msg = endProcEvents[i];
            ASSERT(msg->getContextPointer()==NULL);
            msg->setContextPointer(frame);
            scheduleAt(simTime() + processingTime, msg);
        }
    }
    // Drop the frame and record the number of dropped frames
    else
    {
        EV_WARN << "Buffer full, dropping frame " << frame << endl;
        delete frame;
        ++numDroppedFrames;
    }

    // Record statistics of buffer usage levels
    bufferLevel.record(bufferUsed);
}

void MACRelayUnitNP::processFrame(cMessage *msg)
{
    int cpu = msg->getKind();
    EtherFrame *frame = (EtherFrame *) msg->getContextPointer();
    ASSERT(frame);
    msg->setContextPointer(NULL);
    long length = frame->getByteLength();
    int inputport = frame->getArrivalGate()->getIndex();

    EV_DETAIL << "CPU-" << cpu << " completed processing of frame " << frame << endl;

    handleAndDispatchFrame(frame, inputport);
    printAddressTable();

    bufferUsed -= length;
    bufferLevel.record(bufferUsed);

    numProcessedFrames++;

    // Process next frame in queue if they are pending
    if (!queue.empty())
    {
        EtherFrame *newframe = (EtherFrame *) queue.pop();
        msg->setContextPointer(newframe);
        EV_DETAIL << "CPU-" << cpu << " starting processing of frame " << newframe << endl;
        scheduleAt(simTime()+processingTime, msg);
    }
    else
    {
        EV_DETAIL << "CPU-" << cpu << " idle\n";
    }
}

void MACRelayUnitNP::finish()
{
    recordScalar("processed frames", numProcessedFrames);
    recordScalar("dropped frames", numDroppedFrames);
}

void MACRelayUnitNP::start()
{
    MACRelayUnitBase::start();
}

void MACRelayUnitNP::stop()
{
    for (int i=0; i<numCPUs; i++)
    {
        cMessage *endProcEvent = endProcEvents[i];
        EtherFrame *etherFrame = (EtherFrame *)endProcEvent->getContextPointer();
        if (etherFrame)
        {
            endProcEvent->setContextPointer(NULL);
            delete etherFrame;
        }
        cancelEvent(endProcEvent);
    }
    MACRelayUnitBase::stop();
}

