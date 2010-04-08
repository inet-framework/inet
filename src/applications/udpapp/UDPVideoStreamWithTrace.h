///
/// @file   UDPVideoStreamWithTrace.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-04-02
///
/// @brief  Defines constants and types for UDP video streaming with trace
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef UDPVIDEOSTREAM_WITH_TRACE_H
#define UDPVIDEOSTREAM_WITH_TRACE_H

#include <vector>


enum TraceFormat {ASU_TERSE, ASU_VERBOSE}; ///< trace file formats

enum FrameType {I, IDR, P, B};	/// H.264 AVC frame types

//enum MessageKind {FRAME_START = 100, PACKET_TX = 200}; ///< kind values for self messages

typedef std::vector<char> CharVector;
typedef std::vector<double> DoubleVector;
typedef std::vector<FrameType> FrameTypeVector;
typedef std::vector<long> LongVector;
typedef std::vector<std::string> StringVector;


#endif	// UDPVIDEOSTREAM_WITH_TRACE_H
