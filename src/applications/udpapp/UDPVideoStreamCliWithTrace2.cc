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
//#include "IPAddressResolver.h"


Define_Module(UDPVideoStreamCliWithTrace2);


void UDPVideoStreamCliWithTrace2::initialize()
{
    UDPVideoStreamCliWithTrace::initialize();

    firstFrameReceived = false;
}

void UDPVideoStreamCliWithTrace2::receiveStream(UDPVideoStreamPacket *pkt)
{
    EV << "Video stream packet:\n";
#ifndef NDEBUG
    printPacket(PK(pkt));
#endif

    // FIXME Delay to loss conversion yet to be implemented in the following

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
    			if (isFragmentStart == true)
    			{
                    // initialize variables for handling sequence number and frame number
    				prevSequenceNumber = seqNumber;
                    prevPFrameNumber = (frameNumber > 0) ? frameNumber - numBFrames - 1 : -1;
                        ///< to avoid loss of B frames after this frame
                    currentFrameNumber = frameNumber;
                    currentEncodingNumber = encodingNumber;
                    currentFrameType = frameType;

                    if (isFragmentEnd == true)
            		{
            			// this frame consists of this packet only
            			currentFrameFinished = true;
            			prevIFrameNumber = frameNumber;
            			numFramesReceived++;	///< count the current frame as well
            		}
            		else
            		{
            			// more fragments to come!
            			currentFrameDiscard = false;
    					currentFrameFinished = false;
                        prevIFrameNumber = -1;
            		}

                    numPacketsReceived++;	///< update packet statistics
    				warmupFinished = true;	///< set the flag
    			}	// end of fragmentStart check
    		}	// end of frameType check
    	}	// end of warm-up period check
    }	// end of warm-up flag check and related processing
    else
	{
    	// detect loss of packet(s)
    	// TODO: implement advanced packet and frame loss processing
		// (e.g., based on client playout modeling related with startup buffering)
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
        		if (isFragmentEnd == true)
        		{
        			// this frame consists of this packet only
        			currentFrameFinished = true;
        			prevIFrameNumber = frameNumber;
        			numFramesReceived++;	///< count the current frame as well
        		}
        		else
        		{
        			// more fragments to come!
        			currentFrameDiscard = false;
					currentFrameFinished = false;
        		}

        		// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;
        	}	// end of processing of the first packet of I/IDR frame
        	else
        	{
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
        				currentNumFramesLost++;
        			}
        			currentFrameFinished = true;
            	}
        	}	// end of processing of the non-first packet of I/IDR frame
        	break;

        case P:
        	if (isFragmentStart == true)
        	{
        		if (
                    prevIFrameNumber == frameNumber - numBFrames - 1 ||
                    prevPFrameNumber == frameNumber - numBFrames - 1
                    )
				{
					// I or P frame that the current frame depends on was successfully decoded
        			currentFrameDiscard = false;

					if (isFragmentEnd == true)
					{
						currentFrameFinished = true;	/// no more packet in this frame
						prevPFrameNumber = frameNumber;
						numFramesReceived++; ///< count the current frame as well
					}
					else
					{
						currentFrameFinished = false;	///< more fragments to come
					}
				}
				else
				{
					// the dependency check failed, so the current frame will be discarded
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

				// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;
        	}	// end of processing of the first packet of P frame
        	else
        	{
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
        				currentNumFramesLost++;
        			}
        			currentFrameFinished = true;
            	}
        	}	// end of processing of the non-first packet of P frame
        	break;

        case B:
        	if (isFragmentStart == true)
        	{
        		// check frame dependency
        		long lastDependonFrameNumber = (frameNumber/(numBFrames + 1))*(numBFrames + 1);
                    ///< frame number of the last I or P frame it depends on
        		long nextDependonFrameNumber = lastDependonFrameNumber + numBFrames + 1;
                    ///< frame number of the next I or P frame it depends on
        		bool passedDependency = false;
        		if (nextDependonFrameNumber % gopSize == 0)
        		{
        			// next dependent frame is I frame, so we need to check
                    // both next (I) and last frames.
        			if (
                        prevPFrameNumber == lastDependonFrameNumber &&
                        prevIFrameNumber == nextDependonFrameNumber
                        )
        			{
        				passedDependency = true;
        			}
        		}
        		else
        		{
        			// next dependent frame is P frame, so we need to check
                    // only next (P) frame.
        			if (prevPFrameNumber == nextDependonFrameNumber)
        			{
        				passedDependency = true;
        			}
        		}

				if (passedDependency == true)
				{
					if (isFragmentEnd == true)
					{
						// this frame consists of this packet only
						currentFrameFinished = true;
						numFramesReceived++; ///< count the current frame as well
					}
					else
					{
						// more fragments to come!
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
						currentNumFramesLost++; ///< count the current frame as well
					}
					else
					{
						// more fragments to come!
						currentFrameFinished = false;
					}
				}

				// update frame-related flags and variables
				currentFrameNumber = frameNumber;
				currentEncodingNumber = encodingNumber;
				currentFrameType = frameType;
        	}	// end of processing of the first packet of B frame
        	else
        	{
        		if (isFragmentEnd == true)
            	{
        			if (currentFrameDiscard == false)
        			{
        				// the frame has been received and decoded successfully
            			numFramesReceived++;
        			}
        			else
        			{
        				currentNumFramesLost++;
        			}
        			currentFrameFinished = true;
            	}
        	}	// end of processing of the non-first packet of B frame
        	break;

        default:
        	error("%s: Unexpected frame type: %d", getFullPath().c_str(), frameType);
		}	// end of switch ()

		// update packet statistics
		numPacketsReceived++;
		numPacketsLost += currentNumPacketsLost;

		// update frame statistics
		numFramesDiscarded += currentNumFramesLost;

	}	// end of 'if (warmupFinshed == false)'

    delete pkt;
}
