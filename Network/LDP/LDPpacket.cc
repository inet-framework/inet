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
    
/********************************************************************************
*                                                                                *
*                                LDP GENERIC PACKET                                *
*                                                                                *
*********************************************************************************/

LDPpacket::LDPpacket(): cPacket()
{
    type =UNKNOWN;
}

LDPpacket::LDPpacket(int messageType): cPacket()
{
       type=messageType;
}

// copy constructor
LDPpacket::LDPpacket(const LDPpacket& ldp)
                : cPacket()
{
        setName ( "LDP" );
        operator=( ldp );
}

// encapsulation
void LDPpacket::encapsulate(cPacket *p)
{
        cPacket::encapsulate(p);
}

// decapsulation: convert to cPacket *
cPacket *LDPpacket::decapsulate()
{
        return (cPacket *)(cPacket::decapsulate());
}


    
/********************************************************************************
*                                                                                *
*                                LDP MAPPING MESSAGE                                *
*                                                                                *
*********************************************************************************/

LabelMappingMessage::LabelMappingMessage():LDPpacket()
{
      type=LABEL_MAPPING;
      setKind(type);
}


int LabelMappingMessage::getType()
{
    return LABEL_MAPPING;
}

void LabelMappingMessage::printInfo(ostream& os)
{
    
os << "LDP LABEL MAPPING MESSAGE " << "\n";

os << "FEC: " << getFec() << 
"LABEL: " << getLabel() << "\n";

}


    
/********************************************************************************
*                                                                                *
*                                LDP LABEL REQUEST                                *
*                                                                                *
*********************************************************************************/


LabelRequestMessage::LabelRequestMessage(): LDPpacket()
{
    type=LABEL_REQUEST;
    setKind(type);
}


void LabelRequestMessage::printInfo(ostream& os)
{
  
    os << "LDP LABEL REQUEST MESSAGE " << "\n";
    os << "FEC: " << getFec() << "\n";
}



int LabelRequestMessage::getType()
{
    return LABEL_REQUEST;
}




    
/********************************************************************************
*                                                                                *
*                                LDP HELLO MESSAGE                                *
*                                                                                *
*********************************************************************************/

HelloMessage::HelloMessage()
{
         type=  HELLO;
         setKind(type);
}


HelloMessage::HelloMessage(double time, bool tbit, bool rbit)
{
         holdTime=time;
         Tbit=tbit;
         Rbit=rbit;
         type=HELLO;
}

int HelloMessage::getType()
{
    return HELLO;
}
void HelloMessage::printInfo(ostream& os)
{
        os << "LDP HELLO MESSAGE" << " T=" << Tbit << "R=" << Rbit;
}

IniMessage::IniMessage():LDPpacket()
{
        type=INITIALIZATION;
}

IniMessage::IniMessage(double time, bool abit, bool dbit, int pvlim, string r_ldp_id)
{
         type=INITIALIZATION;
         KeepAliveTime=time;
         Abit=abit;
         Dbit=dbit;
         PVLim=pvlim;
         Receiver_LDP_Identifier = r_ldp_id;

}

void IniMessage::printInfo(ostream& os)
{
         os << "LDP INITIALIZATION MESSAGE" <<"A=" <<Abit <<
          "D=" <<Dbit << " PL limit=" << PVLim <<
          " Receiver ID=" << Receiver_LDP_Identifier;

}

int IniMessage::getType()
{
    return INITIALIZATION;
}

AddressMessage::AddressMessage(): LDPpacket()
{
      type=ADDRESS;
}

AddressMessage::AddressMessage(bool iswithdraw, string addFamily, vector<string> *addressList)
{
      type=ADDRESS;
      isWithdraw=iswithdraw;
      family=addFamily;
      addresses=addressList;

}

void AddressMessage::printInfo(ostream& os)
{
    os << "LDP ADDRESS MESSAGE " <<"\n";
    for(int i=0;i<addresses->size();i++)
        os << (*addresses)[i] << "\n";

}

int AddressMessage::getType()
{
    return ADDRESS;
}



