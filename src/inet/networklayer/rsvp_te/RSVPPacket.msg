//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


import inet.networklayer.rsvp_te.IntServ;

enum RSVP_ENUMS
{
    PATH_MESSAGE = 1;
    RESV_MESSAGE = 2;
    PTEAR_MESSAGE = 3;
    RTEAR_MESSAGE = 4;
    PERROR_MESSAGE = 5;
    RERROR_MESSAGE = 6;
    HELLO_MESSAGE = 7;
};

namespace inet;

//
// RSVP message common part
//
packet RSVPMessage
{
    int rsvpKind;
}

//
// FIXME missing documentation
//
packet RSVPPacket extends RSVPMessage
{
    @customize(true);
    SessionObj_t session;
    bool checksumValid = true;
}
