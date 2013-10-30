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

#include "MACRelayUnitPP.h"
#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "MACAddress.h"


Define_Module( MACRelayUnitPP );

simsignal_t MACRelayUnitPP::processedBytesSignal = registerSignal("processed");
simsignal_t MACRelayUnitPP::droppedBytesSignal = registerSignal("dropped");
simsignal_t MACRelayUnitPP::usedBufferBytesSignal = registerSignal("usedBufferBytes");

/* unused for now
static std::ostream& operator<< (std::ostream& os, cMessage *msg)
{
    os << "(" << msg->getClassName() << ")" << msg->getFullName();
    return os;
}
*/

MACRelayUnitPP::MACRelayUnitPP()
{
    buffer = NULL;
}

MACRelayUnitPP::~MACRelayUnitPP()
{
    for (int i = 0; i < numPorts; ++i)
    {
        cancelAndDelete(buffer[i].timer);
    }
    delete [] buffer;
}

void MACRelayUnitPP::initialize(int stage)
{
    MACRelayUnitBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        numProcessedFrames = numDroppedFrames = 0;
        WATCH(numProcessedFrames);
        WATCH(numDroppedFrames);

        processingTime = par("processingTime");
        bufferSize = par("bufferSize");
        highWatermark = par("highWatermark");
        pauseUnits = par("pauseUnits");

        bufferUsed = 0;
        WATCH(bufferUsed);

        buffer = new PortBuffer[numPorts];
        for (int i = 0; i < numPorts; ++i)
        {
            buffer[i].port = i;
            buffer[i].cpuBusy = false;
            buffer[i].timer = new cMessage("endProcessing");
            buffer[i].timer->setContextPointer(&buffer[i]);
            char qname[40];
            sprintf(qname, "portQueue%d", i);
            buffer[i].queue.setName(qname);
        }

        EV << "Parameters of (" << getClassName() << ") " << getFullPath() << "\n";
        EV << "processing time: " << processingTime << "\n";
        EV << "ports: " << numPorts << "\n";
        EV << "buffer size: " << bufferSize << "\n";
        EV << "address table size: " << addressTableSize << "\n";
        EV << "aging time: " << agingTime << "\n";
        EV << "high watermark: " << highWatermark << "\n";
        EV << "pause time: " << pauseUnits << "\n";
        EV << "\n";
    }
}

void MACRelayUnitPP::handleMessage(cMessage *msg)
{
    if (!msg->isSelfMessage())
    {
        if(!isOperational)
        {
            EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
            delete msg;
            return;
        }
        // Frame received from MAC unit
        handleIncomingFrame(check_and_cast<EtherFrame *>(msg));
    }
    else
    {
        if(!isOperational)
            throw cRuntimeError("model error: self message arrived when module status is DOWN");
        // Self message signal used to indicate a frame has been finished processing
        processFrame(msg);
    }
}

void MACRelayUnitPP::handleIncomingFrame(EtherFrame *frame)
{
    // If buffer not full, insert payload frame into buffer and process the frame in parallel.

    long length = frame->getByteLength();
    if (length + bufferUsed < bufferSize)
    {
        int inputport = frame->getArrivalGate()->getIndex();
        buffer[inputport].queue.insert(frame);
        buffer[inputport].port = inputport;
        bufferUsed += length;

        // send PAUSE if above watermark
        if (pauseUnits>0 && highWatermark>0 && bufferUsed>=highWatermark)
            sendPauseFramesIfNeeded(pauseUnits);

        if (buffer[inputport].cpuBusy)
        {
            EV << "Port CPU " << inputport << " busy, incoming frame " << frame << " enqueued for later processing\n";
        }
        else
        {
            EV << "Port CPU " << inputport << " free, begin processing of incoming frame " << frame << endl;
            buffer[inputport].cpuBusy = true;
            scheduleAt(simTime() + processingTime, buffer[inputport].timer);
        }
    }
    // Drop the frame and record the number of dropped frames
    else
    {
        EV << "Buffer full, dropping frame " << frame << endl;
        delete frame;
        ++numDroppedFrames;
        emit(droppedBytesSignal, length);
    }

    // Record statistics of buffer usage levels
    emit(usedBufferBytesSignal, bufferUsed);
}

void MACRelayUnitPP::processFrame(cMessage *msg)
{
    // Extract frame from the appropriate buffer;
    PortBuffer *pBuff = (PortBuffer*)msg->getContextPointer();
    EtherFrame *frame = (EtherFrame*)pBuff->queue.pop();
    long length = frame->getByteLength();
    int inputport = pBuff->port;

    EV << "Port CPU " << inputport << " completed processing of frame " << frame << endl;

    handleAndDispatchFrame(frame, inputport);
    printAddressTable();

    bufferUsed -= length;
    emit(usedBufferBytesSignal, bufferUsed);
    emit(processedBytesSignal, length);

    numProcessedFrames++;

    // Process next frame in queue if they are pending
    if (!pBuff->queue.empty())
    {
        EV << "Begin processing of next frame\n";
        scheduleAt(simTime()+processingTime, msg);
    }
    else
    {
        EV << "Port CPU idle\n";
        pBuff->cpuBusy = false;
    }
}

void MACRelayUnitPP::start()
{
    MACRelayUnitBase::start();
}

void MACRelayUnitPP::stop()
{
    for (int i = 0; i < numPorts; ++i)
    {
        cancelEvent(buffer[i].timer);
        buffer[i].queue.clear();
        buffer[i].cpuBusy = false;
    }
    MACRelayUnitBase::stop();
}

