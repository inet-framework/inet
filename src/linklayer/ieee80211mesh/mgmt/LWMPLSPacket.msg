//
// Copyright (C) 2007 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

//
// Required for MACAddress declarations
//
cplusplus
{{
#include "MACAddress.h"
}};

class noncobject MACAddress;

enum LwmplsType
{
  WMPLS_NORMAL    = 0;
  WMPLS_BEGIN     = 1;
  WMPLS_REFRES    = 2;
  WMPLS_END       = 3;
  WMPLS_BREAK     = 4;
  WMPLS_NOTFOUND  = 5;
  WMPLS_ACK       = 6;
  WMPLS_ADITIONAL = 7;
  WMPLS_BEGIN_W_ROUTE = 8;
  WMPLS_SEND = 9;
  WMPLS_BROADCAST=10;
  // Use the mac addess like labels
  WMPLS_MACBASEDLABEL_BEGIN=11;
  WMPLS_MACBASEDLABEL_BEGIN_W_ROUTE=12;
  WMPLS_ANNOUNCE_GATEWAY =14;
  WMPLS_REQUEST_GATEWAY =15;
};

//
// Capability Information field format. This field is not used
// by the model (and is omitted from management frame formats too),
// because its fields are related to features not supported by this model
// (PCF, privacy, and beacons in ad-hoc mode). It is only provided here
// for completeness.
//
packet LWMPLSPacket
{
     int label;// 20 bits 
     int labelReturn;
     int type enum(LwmplsType); // 4 bits
     bool nextHeader=false; //1 bit
			 // 7 bits route long
     unsigned int counter;
     byteLength = 6;
     MACAddress source; // This fields are in the 4 address I'm include here for simplicity
     MACAddress dest;   //
     short TTL;
     MACAddress vectorAddress[];
};

packet  LWMPLSControl extends LWMPLSPacket
{
     byteLength = 12;
     unsigned char gateAddressPtr[];
     unsigned char assocAddressPtr[];
};
