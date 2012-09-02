///
/// @file   UDPVideoStreamCliWithTrace2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-02-07
///
/// @brief  Implements UDPVideoStreamCliWithTrace2 class.
///
/// @note
/// This file implements UDPVideoStreamCliWithTrace, modeling a video
/// streaming client based on trace files from ASU video trace library [1]
/// with delay to loss conversion based on play-out buffering.
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// </ol>
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include "UDPVideoStreamCliWithTrace2.h"


Define_Module(UDPVideoStreamCliWithTrace2);


void UDPVideoStreamCliWithTrace2::initialize()
{
    UDPVideoStreamCliWithTrace::initialize();

    framePeriod = 1.0 / par("fps").longValue();
    startupFrameReceived = false;
    startupFrameArrivalTime = 0.0;
    startupFrameNumber = 0L;
}

void UDPVideoStreamCliWithTrace2::updateStartupFrameVariables(long frameNumber)
{
    if (frameNumber > startupFrameNumber)
    {   // regular update
        startupFrameArrivalTime += framePeriod*(frameNumber - startupFrameNumber);
    }
    else
    {   // handle frame wrap around
        startupFrameArrivalTime += framePeriod*(frameNumber + numTraceFrames - startupFrameNumber);
    }
    startupFrameNumber = frameNumber;
}

// check frame delay and return 1 if the delay exceeds a certain bound and 0 otherwise
int UDPVideoStreamCliWithTrace2::convertFrameDelayIntoLoss(long frameNumber, simtime_t frameArrivalTime, FrameType frameType)
{
    if (frameArrivalTime > startupFrameArrivalTime + framePeriod*(frameNumber - startupFrameNumber) + startupDelay)
    {   // frame delay/loss conversion
        return 1;
    }
    else
    {
        numFramesReceived++; ///< count the current frame as well
        switch (frameType)
        {
        case IDR:
        case I:
            prevIFrameNumber = currentFrameNumber;
            break;
        case P:
            prevPFrameNumber = currentFrameNumber;
            break;
        case B:
            break;
        default:
        	error("%s: Unexpected frame type: %d", getFullPath().c_str(), frameType);
        	break;            
        }
        return 0;
    }
}

void UDPVideoStreamCliWithTrace2::receiveStream(UDPVideoStreamPacket *pkt)
{
    EV << "Video stream packet:\n";
#ifndef NDEBUG
    printPacket(PK(pkt));
#endif

    // record vector statistics; note that warm-up period handling is automatically done.
    eed.record(simTime() - pkt->getCreationTime());

    // get packet fields
	uint16_t seqNumber = uint16_t(pkt->getSequenceNumber());
	bool isFragmentStart = pkt->getFragmentStart();
    bool isFragmentEnd = pkt->getFragmentEnd();
    long frameNumber = pkt->getFrameNumber();
	FrameType frameType = FrameType(pkt->getFrameType());
	long encodingNumber = frameEncodingNumber(frameNumber, numBFrames, frameType);

    // in the following, the frame statistics will be updated only when
    // - the end fragment has been received or
    // - the previously handled frame hasn't been properly processed
	int currentNumPacketsLost = 0;
	int currentNumFramesLost = 0;
    if (warmupFinished == false)
    {
    	// handle warm-up flag based on both simulation time and GoP beginning
    	if (simTime() >= simulation.getWarmupPeriod())
    	{
    		if (frameType == I || frameType == IDR)
    		{
    		    if (isFragmentEnd == true)
    		    {
                    // initialize variables for handling sequence number and frame number
    				prevSequenceNumber = seqNumber;
                    prevPFrameNumber = (frameNumber > 0) ? frameNumber - numBFrames - 1 : -1;
                    ///< to avoid loss of B frames after this frame
                    currentFrameNumber = frameNumber;
                    currentEncodingNumber = encodingNumber;
                    currentFrameType = frameType;
                    // note that the above lines are for the fragment start; we do for this special case here

                    currentFrameFinished = true;
                    prevIFrameNumber = frameNumber;
//                    numFramesReceived++; ///< count the current frame as well

                    // initialize variables for frame delay/loss conversion
                    startupFrameReceived = true;
                    startupFrameArrivalTime = simTime();
                    startupFrameNumber = frameNumber;
//                    numPacketsReceived++; ///< update packet statistics
                    warmupFinished = true; ///< set the flag
    			}	// end of fragmentEnd check
    		}	// end of frameType check
    	}	// end of warm-up period check
    }	// end of warm-up flag check
    else
	{   // detect loss of packet(s)
    	if (seqNumber != (prevSequenceNumber + 1) % 65536)
		{
			currentNumPacketsLost =
                (seqNumber > prevSequenceNumber ? seqNumber : seqNumber + 65536)
                - prevSequenceNumber - 1;
			EV << currentNumPacketsLost << " video stream packet(s) lost" << endl;

			// detect loss of frame(s) between the one previously handled and the current one.
			// note that the previously handled frame could be the current one as well.
			if (encodingNumber > (currentEncodingNumber + 1) % numTraceFrames)
			{
				currentNumFramesLost =
                    (encodingNumber > currentEncodingNumber ? encodingNumber : encodingNumber + numTraceFrames)
                    - currentEncodingNumber - 1;
			}

			// check whether the previously handled frame should be discarded or not
			// as a result of current packet loss
			// TODO: implement the case for decoding threshold (DT) < 1
			if (currentFrameFinished == false)
			{
				currentNumFramesLost++;
			}

    		// set frame discard flag for non-first packet of frame
			// TODO: implement the case for decoding threshold (DT) < 1
    		if (isFragmentStart == false)
    		{
    			currentFrameDiscard =true;
    		}
		}	// end of packet and frame loss detection and related-processing
        prevSequenceNumber = seqNumber;	///< update the sequence number

        switch (frameType)
		{
        case IDR:
        case I:
        	if (isFragmentStart == true)
        	{
        		// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;

        		if (isFragmentEnd == true)
        		{   // this frame consists of this packet only
                    currentFrameFinished = true;
                    updateStartupFrameVariables(frameNumber);
                    currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
        		}
        		else
        		{   // more fragments to come!
        			currentFrameDiscard = false;
					currentFrameFinished = false;
        		}
        	}	// end of processing of the first packet of I/IDR frame
        	else
        	{
        		if (isFragmentEnd == true)
            	{
        			currentFrameFinished = true;
        			if (currentFrameDiscard == false)
        			{   // the frame has been received and decoded successfully
                        updateStartupFrameVariables(frameNumber);
                        currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
        			}
        			else
        			{
        				currentNumFramesLost++;
        			}
            	}
        	}	// end of processing of the non-first packet of I/IDR frame
        	break;

        case P:
        	if (isFragmentStart == true)
        	{
				// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;

        		if (
                    prevIFrameNumber == frameNumber - numBFrames - 1 ||
                    prevPFrameNumber == frameNumber - numBFrames - 1
                    )
				{   // I or P frame that the current frame depends on was successfully decoded
        			currentFrameDiscard = false;
					if (isFragmentEnd == true)
					{   // this frame consists of this packet only
						currentFrameFinished = true;	/// no more packet in this frame
                        currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
					}
					else
					{
						currentFrameFinished = false;	///< more fragments to come
					}
				}
				else
				{   // the dependency check failed, so the current frame will be discarded
					currentFrameDiscard = true;
					if (isFragmentEnd == true)
					{
						currentFrameFinished = true;	/// no more packet in this frame
						currentNumFramesLost++; ///< count the current frame as well
					}
					else
					{
						currentFrameFinished = false;	///< more fragments to come
					}
				}
        	}	// end of processing of the first packet of P frame
        	else
        	{
        		if (isFragmentEnd == true)
            	{
        			currentFrameFinished = true;
        			if (currentFrameDiscard == false)
        			{   // the frame has been received and decoded successfully
                        currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
        			}
        			else
        			{
        				currentNumFramesLost++;
        			}
            	}
        	}	// end of processing of the non-first packet of P frame
        	break;

        case B:
        	if (isFragmentStart == true)
        	{
				// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;

                // check frame dependency
        		long lastDependonFrameNumber = (frameNumber/(numBFrames + 1))*(numBFrames + 1);
                ///< frame number of the last I or P frame it depends on
        		long nextDependonFrameNumber = lastDependonFrameNumber + numBFrames + 1;
                ///< frame number of the next I or P frame it depends on
        		bool passedDependency = false;
        		if (nextDependonFrameNumber % gopSize == 0)
        		{   // next dependent frame is I frame, so we need to check both next (I) and last frames.
        			if (
                        prevPFrameNumber == lastDependonFrameNumber &&
                        prevIFrameNumber == nextDependonFrameNumber
                        )
        			{
        				passedDependency = true;
        			}
        		}
        		else
        		{   // next dependent frame is P frame, so we need to check only next (P) frame.
        			if (prevPFrameNumber == nextDependonFrameNumber)
        			{
        				passedDependency = true;
        			}
        		}
				if (passedDependency == true)
				{
					if (isFragmentEnd == true)
					{   // this frame consists of this packet only
						currentFrameFinished = true;
                        currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
					}
					else
					{   // more fragments to come!
						currentFrameDiscard = false;
						currentFrameFinished = false;
					}
				}
				else
				{   // the dependency check failed, so the current frame will be discarded
					currentFrameDiscard = true;
					if (isFragmentEnd == true)
					{   // this frame consists of this packet only
						currentFrameFinished = true;
						currentNumFramesLost++; ///< count the current frame as well
					}
					else
					{   // more fragments to come!
						currentFrameFinished = false;
					}
				}

        	}	// end of processing of the first packet of B frame
        	else
        	{
        		if (isFragmentEnd == true)
            	{
        			currentFrameFinished = true;
        			if (currentFrameDiscard == false)
        			{   // the frame has been received and decoded successfully
                        currentNumFramesLost += convertFrameDelayIntoLoss(frameNumber, simTime(), frameType);
        			}
        			else
        			{
        				currentNumFramesLost++;
        			}
            	}
        	}	// end of processing of the non-first packet of B frame
        	break;

        default:
        	error("%s: Unexpected frame type: %d", getFullPath().c_str(), frameType);
        	break;
		}	// end of switch ()

		// update statistics
		numPacketsReceived++;
		numPacketsLost += currentNumPacketsLost;
		numFramesDiscarded += currentNumFramesLost;
	}	// end of 'if (warmupFinshed == false)'

    delete pkt;
}
