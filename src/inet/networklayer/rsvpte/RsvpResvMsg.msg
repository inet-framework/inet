//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

import inet.common.INETDefs;
import inet.networklayer.contract.ipv4.Ipv4Address;
import inet.networklayer.rsvpte.IntServ;
import inet.networklayer.rsvpte.RsvpPacket;

namespace inet;

//
// FIXME missing documentation
//
class RsvpResvMsg extends RsvpPacket
{
    RsvpHopObj hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RESV_MESSAGE;
}

cplusplus(RsvpResvMsg) {{
  public:
    inline Ipv4Address getNHOP() const { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() const { return getHop().Logical_Interface_Handle; }
}}

//
// FIXME missing documentation
//
class RsvpResvTear extends RsvpPacket
{
    RsvpHopObj hop;
    FlowDescriptorVector flowDescriptor;
    rsvpKind = RTEAR_MESSAGE;
}

cplusplus(RsvpResvTear) {{
  public:
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
}}

//
// FIXME missing documentation
//
class RsvpResvError extends RsvpPacket
{
    RsvpHopObj hop;
    Ipv4Address errorNode;
    int errorCode;
    rsvpKind = RERROR_MESSAGE;
}

cplusplus(RsvpResvError) {{
  public:
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
}}

