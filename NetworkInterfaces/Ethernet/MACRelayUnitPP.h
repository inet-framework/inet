/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _MACRELAYUNITPP_H
#define _MACRELAYUNITPP_H


#include "MACRelayUnitBase.h"

class EtherFrame;

/**
 * An implementation of the MAC Relay Unit that assumes
 * one processor assigned to each incoming port, with
 * separate queues.
 */
class MACRelayUnitPP : public MACRelayUnitBase
{
    Module_Class_Members(MACRelayUnitPP,MACRelayUnitBase,0)

  protected:
    // Stores frame buffer, one for each port
    struct PortBuffer
    {
        int port;
        bool cpuBusy;
        cQueue queue;
    };

    // Parameters controlling how the switch operates
    simtime_t processingTime;   // Time taken to switch and process a frame
    int bufferSize;             // Max size of the buffer
    long highWatermark;         // if buffer goes above this level, send PAUSE frames
    int pauseUnits;             // "units" field in PAUSE frames
    simtime_t pauseInterval;    // min time between sending PAUSE frames

    // Other variables
    int bufferUsed;             // Amount of buffer used to store payload
    PortBuffer *buffer;         // Buffers containing Ethernet payloads
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
     * it into buffer and start processing if processor is free.
     */
    void handleIncomingFrame(EtherFrame *msg);

    /**
     * Triggered when a frame has completed processing, it routes the frame
     * to the appropriate port, and starts processing the next frame.
     */
    void processFrame(cMessage *msg);
};

#endif


