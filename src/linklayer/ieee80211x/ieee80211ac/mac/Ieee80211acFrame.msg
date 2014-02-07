//
// Copyright (C) 2014 Andrea Tino
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

//
// Ieee80211acFrame.msg
//

cplusplus {{
#include "Ieee80211acConsts.h"
#include "MACAddress.h"
#include "Ieee80211acFrameParts.h"
}}

class noncobject MACAddress;
class noncobject Ieee80211acFrameLSTF;
class noncobject Ieee80211acFrameLLTF;
class noncobject Ieee80211acFrameLSIG;
class noncobject Ieee80211acFrameVHTSIGA;
class noncobject Ieee80211acFrameVHTSTF;
class noncobject Ieee80211acFrameVHTLTF;
class noncobject Ieee80211acFrameVHTSIGB;
class noncobject Ieee80211acFrameDATA;

cplusplus {{
//
// 802.11 frame type constants (type+subtype), for the "type" field of
// Ieee80211FrameControl
//
enum Ieee80211acFrameType {
  // Management:
  ST_ASSOCIATIONREQUEST    = 0x00,
  ST_ASSOCIATIONRESPONSE   = 0x01,
  ST_REASSOCIATIONREQUEST  = 0x02,
  ST_REASSOCIATIONRESPONSE = 0x03,
  ST_PROBEREQUEST          = 0x04,
  ST_PROBERESPONSE         = 0x05,
  ST_BEACON                = 0x08,
  ST_ATIM                  = 0x09,
  ST_DISASSOCIATION        = 0x0a,
  ST_AUTHENTICATION        = 0x0b,
  ST_DEAUTHENTICATION      = 0x0c,
  ST_ACTION                = 0x0d,
  ST_NOACKACTION           = 0x0e,

  // Control (CFEND/CFEND_CFACK omitted):
  ST_PSPOLL                = 0x1a,
  ST_RTS                   = 0x1b,
  ST_CTS                   = 0x1c,
  ST_ACK                   = 0x1d,
  ST_BLOCKACK_REQ          = 0x18,
  ST_BLOCKACK              = 0x19,

  // Data (CFPOLL/CFACK subtypes omitted):
  ST_DATA                  = 0x20,

  //Feedback frame for multicast tramsmission
  ST_LBMS_REQUEST          = 0x30,
  ST_LBMS_REPORT           = 0x31
};
}}

packet Ieee80211acFrame {
  // Preamble
  Ieee80211acFrameLSTF preambleLSTF; // L-STF
  Ieee80211acFrameLLTF preambleLLTF; // L-LTF
  Ieee80211acFrameLSIG preambleLSIG; // L-SIG

  // Body
  Ieee80211acFrameVHTSIGA bodyVHTSIGA; // VHT-SIG-A
  Ieee80211acFrameVHTSTF bodyVHTSTF; // VHT-STF
  Ieee80211acFrameVHTLTF bodyVHTLTF; // VHT-LTFs
  Ieee80211acFrameVHTSIGB bodyVHTSIGB; // VHT-SIG-B

  // Payload
  Ieee80211acFrameDATA data;

  MACAddress a;
}

