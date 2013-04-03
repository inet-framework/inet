///
/// @file   UDPVideoStreamPacket.msg
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-03-30
///
/// @brief  Defines self-messages and frames for hybridpon models.
///
/// @remarks Copyright (C) 2009-2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


cplusplus {{
#include "INETDefs.h"
#include "UDPVideoStreamWithTrace.h"
}}

enum FrameType;


//
// Message class representing application layer (i.e., RTP) packet for
// UDP video streaming application. Partial implementation of RFC
// 6184.
//
packet UDPVideoStreamPacket
{
    // RTP header fields
    bool Marker;                ///< 1-bit marker (e.g., indicating the last packet of a frame)
    uint16_t sequenceNumber;	///< 16-bit packet sequence number
    uint32_t timestamp;			///< 32-bit timestamp

    // part of fragmentation unit (FU) header (1 octet) in RTP payload
    bool fragmentStart; ///< indicating the start of fragemented NAL units
    bool fragmentEnd;   ///< indicating the end of fragemented NAL units

    // fields not defined in RFC 3894
    long frameNumber;   ///< display order of the current frame
    double frameTime;	///< cumulative display time of the current frame in millisecond
    int frameType enum(FrameType);	///< one of I, P, B and IDR (in case of H.264)
//    long frameSize;	///< size of the current frame in byte
}
