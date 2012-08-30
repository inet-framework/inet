///
/// @file   UDPVideoStreamCliWithTrace2.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-02-07
///
/// @brief  Declares UDPVideoStreamCliWithTrace2 class, modeling a video
///         streaming client based on trace files from ASU video trace
///         library [1] with delay to loss conversion based on play-out
///         buffering [2].
///
/// @par References:
/// <ol>
///	<li>
/// <a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// <li>
/// Jirka Klaue, Berthold Rathke, and Adam Wolisz, &quot;EvalVid - A framework
/// for video transmission and quality evaluation,&quot; Computer Performance
/// Evaluation. Modelling Techniques and Tools, Lecture Notes in Computer Science,
/// vol. 2794, pp. 255-272, 2003.
/// </li>
/// </ol>
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef UDPVIDEOSTREAMCLI_WITH_TRACE2_H
#define UDPVIDEOSTREAMCLI_WITH_TRACE2_H

#include "UDPVideoStreamCliWithTrace.h"


///
/// @class UDPVideoStreamCliWithTrace2
/// @brief Implementation of a video streaming client to work with
///        UDPVideoStreamSvrWithTrace
///
class INET_API UDPVideoStreamCliWithTrace2 : public UDPVideoStreamCliWithTrace
{
protected:
    double framePeriod; ///< frame period for a video trace

    // variables used for delay to loss conversion based on EvalVid algorithm
    bool startupFrameReceived;
    simtime_t startupFrameArrivalTime;
    long startupFrameNumber;
    /* simtime_t prevFrameArrivalTime; */
    /* simtime_t prevOrgFrameArrivalTime; */
    /* simtime_t currFrameArrivalTime; */
    /* simtime_t currOrgFrameArrivalTime; */

protected:
    virtual void initialize();
    virtual void receiveStream(UDPVideoStreamPacket *pkt);
    void updateStartupFrameVariables(long frameNumber);
    int convertFrameDelayIntoLoss(long frameNumber, simtime_t frameArrivalTime, FrameType frameType);
};


#endif	// UDPVIDEOSTREAMCLI_WITH_TRACE2_H
