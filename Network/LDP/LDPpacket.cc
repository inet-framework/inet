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


#include "LDPpacket.h"

//---

LDPPacket::LDPPacket() : cMessage()
{
    type = UNKNOWN;
}

LDPPacket::LDPPacket(int messageType) : cMessage()
{
    type = messageType;
}

// copy constructor
LDPPacket::LDPPacket(const LDPPacket& ldp) : cMessage()
{
    setName("LDP");
    operator=(ldp);
}

//----

LDPLabelMapping::LDPLabelMapping() : LDPPacket()
{
    type = LABEL_MAPPING;
    setKind(type);
}


int LDPLabelMapping::getType()
{
    return LABEL_MAPPING;
}

void LDPLabelMapping::printInfo(ostream & os)
{

    os << "LDP LABEL MAPPING MESSAGE " << "\n";

    os << "FEC: " << getFec() << "LABEL: " << getLabel() << "\n";

}

//---

LDPLabelRequest::LDPLabelRequest():LDPPacket()
{
    type = LABEL_REQUEST;
    setKind(type);
}

void LDPLabelRequest::printInfo(ostream & os)
{

    os << "LDP LABEL REQUEST MESSAGE " << "\n";
    os << "FEC: " << getFec() << "\n";
}

int LDPLabelRequest::getType()
{
    return LABEL_REQUEST;
}

//---

LDPHello::LDPHello()
{
    type = HELLO;
    setKind(type);
}


LDPHello::LDPHello(double time, bool tbit, bool rbit)
{
    holdTime = time;
    Tbit = tbit;
    Rbit = rbit;
    type = HELLO;
}

int LDPHello::getType()
{
    return HELLO;
}

void LDPHello::printInfo(ostream & os)
{
    os << "LDP HELLO MESSAGE" << " T=" << Tbit << "R=" << Rbit;
}

//---

LDPIni::LDPIni():LDPPacket()
{
    type = INITIALIZATION;
}

LDPIni::LDPIni(double time, bool abit, bool dbit, int pvlim, string r_ldp_id)
{
    type = INITIALIZATION;
    KeepAliveTime = time;
    Abit = abit;
    Dbit = dbit;
    PVLim = pvlim;
    Receiver_LDP_Identifier = r_ldp_id;

}

void LDPIni::printInfo(ostream & os)
{
    os << "LDP INITIALIZATION MESSAGE" << "A=" << Abit <<
        "D=" << Dbit << " PL limit=" << PVLim << " Receiver ID=" << Receiver_LDP_Identifier;

}

int LDPIni::getType()
{
    return INITIALIZATION;
}

//---

LDPAddress::LDPAddress():LDPPacket()
{
    type = ADDRESS;
}

LDPAddress::LDPAddress(bool iswithdraw, string addFamily, vector < string > *addressList)
{
    type = ADDRESS;
    isWithdraw = iswithdraw;
    family = addFamily;
    addresses = addressList;

}

void LDPAddress::printInfo(ostream & os)
{
    os << "LDP ADDRESS MESSAGE " << "\n";
    for (int i = 0; i < addresses->size(); i++)
        os << (*addresses)[i] << "\n";

}

int LDPAddress::getType()
{
    return ADDRESS;
}
