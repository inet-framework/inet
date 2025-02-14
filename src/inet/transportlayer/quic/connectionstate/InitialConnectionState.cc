/*
 * InitialConnectionState.cc
 *
 *  Created on: 7 Feb 2025
 *      Author: msvoelker
 */


#include "InitialConnectionState.h"

namespace inet {
namespace quic {

ConnectionState *InitialConnectionState::processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processInitialPacket in " << name << endl;

    processFrames(pkt);
}

} /* namespace quic */
} /* namespace inet */
