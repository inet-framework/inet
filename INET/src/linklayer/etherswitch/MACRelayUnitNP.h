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

#ifndef __INET_MACRELAYUNITNP_H
#define __INET_MACRELAYUNITNP_H


#include "MACRelayUnitBase.h"

class EtherFrame;

/**
 * An implementation of the MAC Relay Unit that assumes a shared memory and
 * N CPUs in the switch. The CPUs process frames from a single shared queue.
 */
class INET_API MACRelayUnitNP : public MACRelayUnitBase
{
  public:
    MACRelayUnitNP();
    virtual ~MACRelayUnitNP();

  protected:
    // the shared queue
    cQueue queue;

    // Parameters controlling how the switch operates
    int numCPUs;                // number of processors
    simtime_t processingTime;   // Time taken to switch and process a frame
    int bufferSize;             // Max size of the buffer
    long highWatermark;         // if buffer goes above this level, send PAUSE frames
    int pauseUnits;             // "units" field in PAUSE frames
    simtime_t pauseInterval;    // min time between sending PAUSE frames

    // Other variables
    int bufferUsed;             // Amount of buffer used to store frames
    cMessage **endProcEvents;   // self-messages, one for each processor
    simtime_t pauseLastSent;

    // Parameters for statistics collection
    long numProcessedFrames;
    long numDroppedFrames;
    cOutVector bufferLevel;

  protected:
    /** @name Redefined cSimpleModule member functions. */
    //@{
    virtual void initialize();

    /**
     * Calls handleIncomingFrame() for frames arrived from outside,
     * and processFrame() for self messages.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Writes statistics.
     */
    virtual void finish();
    //@}

    /**
     * Handle incoming Ethernet frame: if buffer full discard it, otherwise, insert
     * it into buffer and start processing if a processor is free.
     */
    virtual void handleIncomingFrame(EtherFrame *msg);

    /**
     * Triggered when a frame has completed processing, it routes the frame
     * to the appropriate port, and starts processing the next frame.
     */
    virtual void processFrame(cMessage *msg);
};

#endif



