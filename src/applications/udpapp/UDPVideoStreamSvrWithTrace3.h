//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_UDPVIDEOSTREAMSVR_WITH_TRACE3_H
#define __INET_UDPVIDEOSTREAMSVR_WITH_TRACE3_H

#include "UDPVideoStreamSvrWithTrace2.h"


///
/// @class UDPVideoStreamSvrWithTrace3
/// @brief Implementation of a video streaming server based on trace files
///        from ASU video trace library [1] for the source clock frequency
///        recovery with jitter time-stamp approach [2].
///
/// @par References:
/// <ol>
///	<li><a href="http://trace.eas.asu.edu/">Video trace library, Arizona State University</a>
/// </li>
/// <li>W. Su and I. F. Akyildiz,
///     "The jitter time-stamp approach for clock recovery of real-time variable
///     bit-rate traffic," IEEE/ACM Trans. Networking, vol. 9., no. 6,
///     Dec. 2001, pp. 746-754.
/// </li>
/// </ol>
///


class INET_API UDPVideoStreamSvrWithTrace3 : public UDPVideoStreamSvrWithTrace2
{
  protected:
    int timingPktPeriod;
    int pktInterval;            // based on clockFrequency (i.e., 'f_r' in [2])

  protected:
    // send a packet of the given video stream
    virtual void sendStreamData(cMessage *pktTimer);

  protected:
    ///@name Overidden cSimpleModule functions
    //@{
    virtual void initialize();
    //@}
};


#endif	// UDPVIDEOSTREAMSVR_WITH_TRACE3_H
