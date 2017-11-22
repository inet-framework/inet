/***************************************************************************
                       RtcpPacket.h  -  description
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

/** \file RtcpPacket.h
 * In this file all rtcp packet types are declared: There is a the superclass
 * RtcpPacket which is not intended to be used directly. It defines an enumeration
 * to distinguish the different rtcp packet types and also includes header
 * fields common to all types of rtcp packets.
 * Direct subclasses are RtcpReceiverReportPacket, RtcpSdesPacket and RtcpByePacket.
 * RtcpSenderReportPacket is declared as a subclass of RtcpReceiverReportPacket
 * because it only extends it with a sender report.
 * Application specific rtcp packets are not defined.
 * The class RtcpCompoundPacket isn't derived from RtcpPacket because it just
 * acts as a container for rtcp packets. Only rtcp compound packets are sent
 * over the network.
 */

#ifndef __INET_RTCPPACKET_H
#define __INET_RTCPPACKET_H

#include "inet/transportlayer/rtp/RtcpPacket_m.h"

namespace inet {

namespace rtp {

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTCPPACKET_H

