/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/


/*
*    File rsvp_message.h
*    RSVP-TE library
*    This file defines rsvp_message class
**/

#include "rsvp_message.h"


/********************************RESERVATION MESSAGE*************************/


/********************************PATH TEAR MESSAGE*************************/

/********************************RESV TEAR MESSAGE*************************/
RSVPResvTear::RSVPResvTear():RSVPPacket()
{
    setKind(RTEAR_MESSAGE);
}

void RSVPResvTear::setHop(RsvpHopObj_t * h)
{
    rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
    rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}

void RSVPResvTear::print()
{
    int sIP = 0;
    ev << "DestAddr = " << IPAddress(getDestAddress()) << "\n" <<
        "ProtId   = " << getProtId() << "\n" <<
        "DestPort = " << getDestPort() << "\n" <<
        "Next Hop = " << IPAddress(getNHOP()) << "\n" <<
        "LIH      = " << IPAddress(getLIH()) << "\n";

    for (int i = 0; i < InLIST_SIZE; i++)
        if ((flow_descriptor_list + i) != NULL)
            if ((sIP = flow_descriptor_list[i].Filter_Spec_Object.SrcAddress) != 0)
            {
                ev << "Receiver =" << IPAddress(sIP) <<
                    ", BW=" << flow_descriptor_list[i].Flowspec_Object.req_bandwidth <<
                    ", Delay=" << flow_descriptor_list[i].Flowspec_Object.link_delay << "\n";


            }
}

/********************************PATH ERROR MESSAGE*************************/


/********************************RESV ERROR MESSAGE*************************/

RSVPResvError::RSVPResvError():RSVPPacket()
{
    setKind(RERROR_MESSAGE);
}

void RSVPResvError::setHop(RsvpHopObj_t * h)
{
    rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
    rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}


