//
// Copyright (C) 2015 OpenSim Ltd.
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

cplusplus {{
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacFrameTypes.h"
#include "inet/linklayer/common/MACAddress.h"
}}

class noncobject TypeSubtype;
class noncobject BasicType;
class noncobject MACAddress;

namespace inet::ieee80211;

// 8.3.1.1 Format of control frames p. 404
class Ieee80211FrameControl
{
    int protocolVersion;
    BasicType basicType;
    TypeSubtype typeSubtype;
    bool toDs;
    bool fromDs;
    bool moreFrag;
    bool retry;
    bool powerManagement;
    bool moreData;
    bool protectedFrame;
    bool order;
}

packet Ieee80211NewRtsFrame
{
    Ieee80211FrameControl frameControl;
    // For all CTS frames transmitted by a non-QoS STA in response to RTS frames, the duration value is the
    // value obtained from the Duration field of the immediately previous RTS frame, minus the time, in
    // microseconds, required to transmit the CTS frame and its SIFS interval. If the calculated duration includes a
    // fractional microsecond, that value is rounded up to the next higher integer.
    // At a non-QoS STA, if the CTS is the first frame in the exchange and the pending data or management frame
    // requires acknowledgment, the duration value is the time, in microseconds, required to transmit the pending
    // data or management frame, plus two SIFS intervals plus one ACK frame. At a non-QoS STA, if the CTS is
    // the first frame in the exchange and the pending data or management frame does not require
    // acknowledgment, the duration value is the time, in microseconds, required to transmit the pending data or
    // management frame, plus one SIFS interval. If the calculated duration includes a fractional microsecond, that
    // value is rounded up to the next higher integer.
    // For other CTS transmissions by a QoS STA, the duration value is set as defined in 8.2.5.
    simtime_t duration;
    // The RA field of the RTS frame is the address of the STA, on the WM, that is the intended immediate
    // recipient of the pending individually addressed data, management, or control frame.
    MACAddress ra;
    // The TA field is the address of the STA transmitting the RTS frame.
    MACAddress ta;
    // Frame check sequence
    int fcs;
}

packet Ieee80211NewCtsFrame
{
    Ieee80211FrameControl frameControl;
    simtime_t duration;
    // When the CTS frame follows an RTS frame, the RA field of the CTS frame is copied from the TA field of
    // the immediately previous RTS frame to which the CTS is a response. When the CTS is the first frame in a
    // frame exchange, the RA field is set to the MAC address of the transmitter.
    MACAddress ra;
    int fcs;
}

packet Ieee80211NewAckFrame
{
    Ieee80211FrameControl frameControl;
    // For ACK frames sent by non-QoS STAs, if the More Fragments bit was equal to 0 in the Frame Control field
    // of the immediately previous individually addressed data or management frame, the duration value is set to 0.
    // In other ACK frames sent by non-QoS STAs, the duration value is the value obtained from the Duration/ID
    // field of the immediately previous data, management, PS-Poll, BlockAckReq, or BlockAck frame minus the
    // time, in microseconds, required to transmit the ACK frame and its SIFS interval. If the calculated duration
    // includes a fractional microsecond, that value is rounded up to the next higher integer.
    simtime_t duration;
    // The RA field of the ACK frame is copied from the Address 2 field of the immediately previous individually
    // addressed data, management, BlockAckReq, BlockAck, or PS-Poll frames.
    MACAddress ra;
    int fcs;
}

packet Ieee80211NewPsPoll
{
    Ieee80211FrameControl frameControl;
    // The AID is the value assigned to the STA transmitting the frame by the AP in the association response
    // frame that established that STA’s current association.
    int aid;
    // The BSSID is the address of the STA contained in the AP.
    MACAddress bssid;
    // The TA field is the address of the STA transmitting the frame.
    MACAddress ta;
}

packet Ieee80211NewCfEnd
{
    Ieee80211FrameControl frameControl;
    // The Duration field is set to 0.
    simtime_t duration = 0;
    // The RA field is the broadcast group address.
    MACAddress ra;
    // The BSSID field is the address of the STA contained in the AP.
    MACAddress bssid;
}

packet Ieee80211NewCfEndCfAck
{
    Ieee80211FrameControl frameControl;
    // The Duration field is set to 0.
    simtime_t duration = 0;
    // The RA field is the broadcast group address.
    MACAddress ra;
    // The BSSID field is the address of the STA contained in the AP.
    MACAddress bssid;
    int fcs;
}

class SequenceControl
{
    int fragmentNumber;
    int sequenceNumber;
}

// 8.3.2.1 Data frame format - p. 413
packet Ieee80211NewDataFrame
{
    Ieee80211FrameControl frameControl;
    simtime_t duration;
    MACAddress address1;
    MACAddress address2;
    MACAddress address3;
    SequenceControl sequenceControl;
    MACAddress address4;
    // TODO: QoS Control
    // HT Control
    int fcs;
}

packet Ieee80211NewManagementFrame
{
    Ieee802111FrameControl frameControl;
    simtime_t duration;
    MACAddress address1;
    MACAddress address2;
    MACAddress address3;
    SequenceControl sequenceControl;
    // TODO: HT Control
    int fcs;
}
