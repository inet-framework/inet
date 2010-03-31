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
}}

//
// Message class representing application layer (e.g., RTP) packet for UDP video streaming application.
//
packet UDPVideoStreamPacket
{
    unsigned short sequenceNumber;	///< packet sequence number
    long frameNumber;
    double frameTime;	///< cumulative frame display time
    char frameType;	///< one of I, P, B
//    long frameSize;	///< frame size in byte
}
