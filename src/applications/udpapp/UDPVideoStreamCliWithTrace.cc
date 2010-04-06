//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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

///
/// @file   UDPVideoStreamCliWithTrace.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-04-02
///
/// @brief  Implements UDPVideoStreamCliWithTrace class.
///
/// @note
/// This file implements UDPVideoStreamCliWithTrace, modeling a video
/// streaming client based on trace files from ASU video trace library [1].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///

#include "UDPVideoStreamCliWithTrace.h"
#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithTrace);


void UDPVideoStreamCliWithTrace::initialize()
{
//    eed.setName("video stream eed");
//    simtime_t startTime = par("startTime");
//
//    if (startTime>=0)
//        scheduleAt(startTime, new cMessage("UDPVideoStreamStart"));

	UDPVideoStreamCli::initialize();

	// initialize module parameters
	startupDelay = par("startupDelay").doubleValue();	///< unit is second
	numTraceFrames = par("numTraceFrames").longValue();
	gopSize = par("gopSize").longValue();
	numBFrames = par("numBFrames").longValue();

	// initialize statistics
	numPacketsReceived = 0;
	numPacketsLost = 0;
    numFramesReceived = 0;
    numFramesLost = 0;

    // initialize flags and other variables
    warmupFinished = false;
//	isFirstPacket = true;
}

void UDPVideoStreamCliWithTrace::finish()
{
	UDPVideoStreamCli::finish();

	recordScalar("packets received", numPacketsReceived);
	recordScalar("packets lost", numPacketsLost);
}

void UDPVideoStreamCliWithTrace::handleMessage(cMessage* msg)
{
	if (msg->isSelfMessage())
	{
		delete msg;
		requestStream();
	}
	else
	{
		receiveStream(check_and_cast<UDPVideoStreamPacket *>(msg));
	}
}

void UDPVideoStreamCliWithTrace::receiveStream(UDPVideoStreamPacket *pkt)
{
    EV << "Video stream packet:\n";
    printPacket(PK(pkt));

    // get some packet info. (including those needed for warm-up processing)
	uint16_t seqNumber = uint16_t(pkt->getSequenceNumber());
	bool isFragmentStart = pkt->getFragmentStart();
    bool isFragmentEnd = pkt->getFragmentEnd();
    long frameNumber = pkt->getFrameNumber();
	FrameType frameType = FrameType(pkt->getFrameType());

    // record vector statistics; note that warm-up period handling is automatically done.
    eed.record(simTime() - pkt->getCreationTime());

	int currentNumPacketsLost = 0;
	int currentNumFramesLost = 0;
    if (warmupFinished == false)
    {
    	// handle warm-up period based on both simulation time and GoP beginning
    	if (simTime() >= simulation.getWarmupPeriod())
    		if (frameType == I || frameType == IDR)
    			if (isFragmentStart == true)
    			{
                    // initialize variables for handling sequence number and frame number
                    prevSequenceNumber = (seqNumber > 0) ? (seqNumber - 1) : 65535;
                    prevPFrameNumber = -1;
                    currentFrameNumber = frameNumber;
                    currentDisplayNumber = displayNumberIFrame(frameNumber, numBFrames);
                    currentFrameType = frameType;

                    if (isFragmentEnd == true)
            		{
            			// this frame consists of this packet only
            			currentFrameFinished = true;
            			prevIFrameNumber = currentFrameNumber;
            			numFramesReceived++;	///< count the current frame as well
            		}
            		else
            		{
            			// more frames to come!
            			currentFrameDiscard = false;
    					currentFrameFinished = false;
                        prevIFrameNumber = -1;
            		}

                    numPacketsReceived++;	///< update packet statistics
    				warmupFinished = true;	///< set the flag
    			}
    }	// end of warm-up flag and related processing
    else
	{
    	// handle sequence number and detect packet loss
    	// TODO: Later, implement advanced packet and frame loss processing
		// (e.g., based on client playout modeling related with startup buffering)
    	if ( seqNumber != (prevSequenceNumber + 1) % 65536 )
		{
			// detected loss of previous packet(s)
    		currentNumPacketsLost = (seqNumber > prevSequenceNumber) ?
    				(seqNumber - prevSequenceNumber - 1) : (seqNumber + 65536 - prevSequenceNumber - 1);

			EV << currentNumPacketsLost << " video stream packet(s) lost" << endl;
		}
        prevSequenceNumber = seqNumber;	///< update the sequence number of previously received packet


        // note that in the following, the frame statistics will be updated only when
        // - the end fragment has been received
        // - the previously handled frame hasn't been properly processed
        switch (frameType)
		{
        case IDR:
        case I:
        	if (isFragmentStart == true)
        	{
        		// handle frame number and detect frame loss
        		long displayNumber = displayNumberIFrame(frameNumber, numBFrames);
        		if ( displayNumber != (currentDisplayNumber + 1) % numTraceFrames )
        		{
        			// detected loss of frame(s) between previously handled frame and the current one.
        			currentNumFramesLost = (displayNumber > currentDisplayNumber) ?
        					(displayNumber - currentDisplayNumber - 1) : (displayNumber + numTraceFrames - currentDisplayNumber - 1);
        		}

        		// check whether the previous frame has been processed completely
    			if (currentFrameFinished == false)
    			{
    				// there must be packet losses before the finish of its processing,
    				// so include the previously handled frame in the loss count.
    				currentNumFramesLost++;
    			}

        		// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentDisplayNumber = displayNumberIFrame(currentFrameNumber, numBFrames);
				currentFrameType = frameType;
        		if (isFragmentEnd == true)
        		{
        			// this frame consists of this packet only
        			currentFrameFinished = true;
        			prevIFrameNumber = currentFrameNumber;
        			numFramesReceived++;	///< count the current frame as well
        		}
        		else
        		{
        			// more frames to come!
        			currentFrameDiscard = false;
					currentFrameFinished = false;
        		}
        	}	// end of processing of the first packet of I/IDR frame
        	else
        	{
        		// check whether there has been any packet loss in the same frame
        		if (currentNumPacketsLost > 0)
        		{
        			currentFrameDiscard =true;
        		}
        		if (isFragmentEnd == true)
            	{
        			if (currentFrameDiscard == false)
        			{
        				// the frame has been received and decoded successfully
            			prevIFrameNumber = currentFrameNumber;
            			numFramesReceived++;
        			}
        			else
        			{
        				numFramesLost++;
        			}
        			currentFrameFinished = true;
            	}
        	}	// end of processing of the non-first packet of I/IDR frame
        	break;

        case P:
        	if (isFragmentStart == true)
        	{
				if (prevIFrameNumber == currentFrameNumber - numBFrames - 1 || prevPFrameNumber == currentFrameNumber - numBFrames - 1)
				{
					// I or P frame that the current frame depends was successfully-decoded

					// handle frame number and detect frame loss
					long displayNumber = displayNumberPFrame(frameNumber, numBFrames);
					if ( displayNumber != (currentDisplayNumber + 1) % numTraceFrames )
					{
						// detected loss of frame(s) between previously handled frame and the current one.
						currentNumFramesLost = (displayNumber > currentDisplayNumber) ?
						(displayNumber - currentDisplayNumber - 1) : (displayNumber + numTraceFrames - currentDisplayNumber - 1);
					}

					// check whether the previous frame has been processed completely
					if (currentFrameFinished == false)
					{
						// there must be packet losses before the finish of its processing,
						// so include the previously handled frame in the loss count.
						currentNumFramesLost++;
					}

					if (isFragmentEnd == true)
					{
						// this frame consists of this packet only
						currentFrameFinished = true;
						prevPFrameNumber = currentFrameNumber;
						numFramesReceived++; ///< count the current frame as well
					}
					else
					{
						// more frames to come!
						currentFrameDiscard = false;
						currentFrameFinished = false;
					}
				}
				else
				{
					// the dependency check failed, so the current frame will be discarded
					currentFrameDiscard = true;

					if (isFragmentEnd == true)
					{
						// this frame consists of this packet only
						currentFrameFinished = true;
						numFramesLost++; ///< count the current frame as well
					}
					else
					{
						// more frames to come!
						currentFrameFinished = false;
					}
				}

				// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentDisplayNumber = displayNumberPFrame(currentFrameNumber, numBFrames);
				currentFrameType = frameType;
        	}	// end of processing of the first packet of P frame
        	else
        	{
        		// check whether there has been any packet loss in the same frame
        		if (currentNumPacketsLost > 0)
        		{
        			currentFrameDiscard =true;
        		}
        		if (isFragmentEnd == true)
            	{
        			if (currentFrameDiscard == false)
        			{
        				// the frame has been received and decoded successfully
            			prevPFrameNumber = currentFrameNumber;
            			numFramesReceived++;
        			}
        			else
        			{
        				numFramesLost++;
        			}
        			currentFrameFinished = true;
            	}
        	}	// end of processing of the non-first packet of P frame
        	break;

        case B:
        	// TODO!!!
        	break;

        default:
        	error("%s: Unexpected frame type: %d", getFullPath().c_str(), frameType);
		}	// end of switch ()

		// update packet statistics
		numPacketsReceived++;
		numPacketsLost += currentNumPacketsLost;

		// update frame statistics
		numFramesLost += currentNumFramesLost;

	}	// end of 'if (warmupFinshed == false)'

    delete pkt;
}
