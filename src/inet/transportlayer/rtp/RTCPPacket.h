/***************************************************************************
                       RTCPPacket.h  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/** \file RTCPPacket.h
 * In this file all rtcp packet types are declared: There is a the superclass
 * RTCPPacket which is not intended to be used directly. It defines an enumeration
 * to distinguish the different rtcp packet types and also includes header
 * fields common to all types of rtcp packets.
 * Direct subclasses are RTCPReceiverReportPacket, RTCPSDESPacket and RTCPByePacket.
 * RTCPSenderReportPacket is declared as a subclass of RTCPReceiverReportPacket
 * because it only extends it with a sender report.
 * Application specific rtcp packets are not defined.
 * The class RTCPCompoundPacket isn't derived from RTCPPacket because it just
 * acts as a container for rtcp packets. Only rtcp compound packets are sent
 * over the network.
 */

#ifndef __INET_RTCPPACKET_H
#define __INET_RTCPPACKET_H

#include "inet/transportlayer/rtp/RTCPPacket1.h"
#include "inet/transportlayer/rtp/RTCPPacket2.h"
#include "inet/transportlayer/rtp/RTCPPacket3.h"

namespace inet {

namespace rtp {

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTCPPACKET_H

