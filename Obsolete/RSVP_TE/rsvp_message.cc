/*******************************************************************
*
*	This library is free software, you can redistribute it 
*	and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/


/*
*	File rsvp_message.h
*	RSVP-TE library
*	This file defines rsvp_message class
**/
#include "rsvp_message.h"



/********************RSVP PACKET**********************************/
//constructor
RSVPPacket::RSVPPacket(): TransportPacket()
{
  _hasChecksum = true;
  setRSVPLength(0);
}

RSVPPacket::RSVPPacket(const RSVPPacket& p): TransportPacket(p)
{
	setName(p.name());
	operator=(p);
}
RSVPPacket::RSVPPacket(const cMessage &msg): TransportPacket(msg)
{
	setRSVPLength(msg.length());
	setChecksumValidity(true);
	
}
RSVPPacket& RSVPPacket::operator=(const RSVPPacket& p)
{
	TransportPacket::operator=(p);
	setRSVPLength(p.RSVPLength());
	setChecksumValidity(p.checksumValid());
	return *this;
}

void RSVPPacket::setLength(int bitlength)
{
	TransportPacket::setLength(bitlength);
	_rsvpLength = bitlength /8;
}

void RSVPPacket::setRSVPLength(int byteLength)
{
	TransportPacket::setLength(byteLength *8);
	_rsvpLength = byteLength;
}

void RSVPPacket::setSession(SessionObj_t* s)
{

	session.DestAddress = s->DestAddress;
	session.DestPort    = s->DestPort;
	session.Protocol_Id = s->Protocol_Id;
	session.setupPri = s->setupPri;
	session.holdingPri=s->holdingPri;
	session.Tunnel_Id = s->Tunnel_Id;
	session.Extended_Tunnel_Id = s->Extended_Tunnel_Id;
}

bool RSVPPacket::isInSession(SessionObj_t* s)
{
    if(session.DestAddress == s->DestAddress &&
       session.DestPort    == s->DestPort    &&
       session.Protocol_Id == s->Protocol_Id )
       return true;
       return false;
}
/*******************************PATH MESSAGE***************************/

PathMessage::PathMessage(): RSVPPacket()
{
	setKind(PATH_MESSAGE);
}



bool PathMessage::equalST(SenderTemplateObj_t* s)
{
    if(sender_descriptor.Sender_Template_Object.SrcAddress ==
       s->SrcAddress                   &&
       sender_descriptor.Sender_Template_Object.SrcPort    ==
       s->SrcPort						&&
	   sender_descriptor.Sender_Template_Object.Lsp_Id     ==
	   s->Lsp_Id)
       return true;
       return false;
     
}
bool PathMessage::equalSD(SenderDescriptor_t* s)
{
    if(sender_descriptor.Sender_Template_Object.SrcAddress ==
       s->Sender_Template_Object.SrcAddress                   &&
       sender_descriptor.Sender_Template_Object.SrcPort    ==
       s->Sender_Template_Object.SrcPort                      &&
	   sender_descriptor.Sender_Template_Object.Lsp_Id		==
	   s->Sender_Template_Object.Lsp_Id						  &&
       sender_descriptor.Sender_Tspec_Object.link_delay    ==
       s->Sender_Tspec_Object.link_delay                      &&
       sender_descriptor.Sender_Tspec_Object.req_bandwidth ==
       s->Sender_Tspec_Object.req_bandwidth)
       return true;
       return false;
}




void PathMessage::setHop(RsvpHopObj_t* h)
{
	rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
	rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;
}

void PathMessage::setSenderTspec(SenderTspecObj_t* s)
{
	sender_descriptor.Sender_Tspec_Object.link_delay = s->link_delay;
	sender_descriptor.Sender_Tspec_Object.req_bandwidth=s->req_bandwidth;
}

void PathMessage::setSenderTemplate(SenderTemplateObj_t* s)
{
	sender_descriptor.Sender_Template_Object.SrcAddress = s->SrcAddress;
	sender_descriptor.Sender_Template_Object.SrcPort    = s->SrcPort;
	sender_descriptor.Sender_Template_Object.Lsp_Id     = s->Lsp_Id;
}

void PathMessage::print()
{
ev << "DestAddr = " << IPAddress(getDestAddress()).getString() << "\n" <<
      "ProtId   = " << getProtId() << "\n" <<
      "DestPort = " << getDestPort() << "\n" <<
      "SrcAddr  = " << IPAddress(getSrcAddress()).getString() << "\n" <<
      "SrcPort  = " << getSrcPort() << "\n" <<
	  "Lsp_Id = " << getLspId() << "\n" <<
      "Next Hop = " << IPAddress(getNHOP()).getString() << "\n" <<
      "LIH      = " << IPAddress(getLIH()).getString()  << "\n" <<
      "Delay    = " << getDelay() << "\n" <<
      "Bandwidth= " << getBW() << "\n";

}

void PathMessage::setContent(PathMessage* pMsg)
{
	setSession(pMsg->getSession());
	setHop(pMsg->getHop());
	setSenderTemplate(pMsg->getSenderTemplate());
	setSenderTspec(pMsg->getSenderTspec());
	setERO(pMsg->getERO());
	addERO(pMsg->hasERO());
	setLabelRequest(pMsg->getLabelRequest());
	
}

/********************************RESERVATION MESSAGE*************************/

ResvMessage::ResvMessage(): RSVPPacket()
{
	setKind(RESV_MESSAGE);
}



void ResvMessage::setHop(RsvpHopObj_t* h)
{
	rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
	rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;
}

void ResvMessage::setContent(ResvMessage* rMsg)
{
	setSession(rMsg->getSession());
	setHop(rMsg->getHop());
	setFlowDescriptor(rMsg->getFlowDescriptor());
	setStyle(rMsg->getStyle());
}


void ResvMessage::print()
{
int sIP =0;
ev << "DestAddr = " << IPAddress(getDestAddress()).getString() << "\n" <<
      "ProtId   = " << getProtId() << "\n" <<
      "DestPort = " << getDestPort() << "\n" <<
      "Next Hop = " << IPAddress(getNHOP()).getString() << "\n" <<
      "LIH      = " << IPAddress(getLIH()).getString()  << "\n";

	for(int i=0; i< InLIST_SIZE; i++)
	if((flow_descriptor_list + i) !=NULL)
	if( (sIP=flow_descriptor_list[i].Filter_Spec_Object.SrcAddress)!=0)
	{
		ev << "Receiver =" << IPAddress(sIP).getString() <<
		",OutLabel=" << flow_descriptor_list[i].label <<
		", BW="  << flow_descriptor_list[i].Flowspec_Object.req_bandwidth <<
		", Delay=" <<	flow_descriptor_list[i].Flowspec_Object.link_delay <<"\n";
		ev << "RRO={";
		for(int c=0;c<MAX_ROUTE;c++)
		{
			int rroEle =flow_descriptor_list[i].RRO[c];
			if(rroEle!=0)
			ev << IPAddress(rroEle).getString() << ",";
		}
		ev << "}\n";
	}
	

}

/********************************PATH TEAR MESSAGE*************************/
PathTearMessage::PathTearMessage(): RSVPPacket()
{
	setKind(PTEAR_MESSAGE);
}

void PathTearMessage::setHop(RsvpHopObj_t* h)
{
	rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
	rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}

void PathTearMessage::setContent(PathTearMessage* pMsg)
{
	setSession(pMsg->getSession());
	setHop(pMsg->getHop());
	setSenderTemplate(pMsg->getSenderTemplate());

		   
	
	
}

void PathTearMessage::setSenderTemplate(SenderTemplateObj_t* s)
{
	senderTemplate.SrcAddress = s->SrcAddress;
	senderTemplate.SrcPort    = s->SrcPort;
}


bool PathTearMessage::equalST(SenderTemplateObj_t* s)
{
    if(senderTemplate.SrcAddress ==
       s->SrcAddress                   &&
       senderTemplate.SrcPort    ==
       s->SrcPort    )
       return true;
       return false;
     
}


void PathTearMessage::print()
{
	ev << "DestAddr = " << IPAddress(getDestAddress()).getString() << "\n" <<
      "ProtId   = " << getProtId() << "\n" <<
      "DestPort = " << getDestPort() << "\n" <<
      "SrcAddr  = " << IPAddress(getSrcAddress()).getString() << "\n" <<
      "SrcPort  = " << getSrcPort() << "\n" <<
      "Next Hop = " << IPAddress(getNHOP()).getString() << "\n" <<
      "LIH      = " << IPAddress(getLIH()).getString()  << "\n";
      

}
/********************************RESV TEAR MESSAGE*************************/
ResvTearMessage::ResvTearMessage(): RSVPPacket()
{
	setKind(RTEAR_MESSAGE);
}

void ResvTearMessage::setHop(RsvpHopObj_t* h)
{
	rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
	rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}

void ResvTearMessage::setContent(ResvTearMessage* rMsg)
{
	setSession(rMsg->getSession());
	setHop(rMsg->getHop());
	setFlowDescriptor(rMsg->getFlowDescriptor());
}

void ResvTearMessage::print()
{
	int sIP =0;
	ev << "DestAddr = " << IPAddress(getDestAddress()).getString() << "\n" <<
      "ProtId   = " << getProtId() << "\n" <<
      "DestPort = " << getDestPort() << "\n" <<
      "Next Hop = " << IPAddress(getNHOP()).getString() << "\n" <<
      "LIH      = " << IPAddress(getLIH()).getString()  << "\n";

	for(int i=0; i< InLIST_SIZE; i++)
	if((flow_descriptor_list + i) !=NULL)
	if( (sIP= flow_descriptor_list[i].Filter_Spec_Object.SrcAddress)!=0)
	{
	ev << "Receiver =" << IPAddress(sIP).getString() <<
	", BW="  << flow_descriptor_list[i].Flowspec_Object.req_bandwidth <<
	", Delay=" <<	flow_descriptor_list[i].Flowspec_Object.link_delay <<"\n";
	

	}
}

/********************************PATH ERROR MESSAGE*************************/

PathErrorMessage::PathErrorMessage(): RSVPPacket()
{
	setKind(PERROR_MESSAGE);

}

void PathErrorMessage::setContent(PathErrorMessage* pMsg)
{
	setSession(pMsg->getSession());	
	setErrorCode(pMsg->getErrorCode());
	setErrorNode(pMsg->getErrorNode());
	setSenderTspec(pMsg->getSenderTspec());
	setSenderTemplate(pMsg->getSenderTemplate());

}

bool PathErrorMessage::equalST(SenderTemplateObj_t* s)
{
    if(sender_descriptor.Sender_Template_Object.SrcAddress ==
       s->SrcAddress                   &&
       sender_descriptor.Sender_Template_Object.SrcPort    ==
       s->SrcPort    )
       return true;
       return false;
     
}
bool PathErrorMessage::equalSD(SenderDescriptor_t* s)
{
    if(sender_descriptor.Sender_Template_Object.SrcAddress ==
       s->Sender_Template_Object.SrcAddress                   &&
       sender_descriptor.Sender_Template_Object.SrcPort    ==
       s->Sender_Template_Object.SrcPort                      &&
       sender_descriptor.Sender_Tspec_Object.link_delay    ==
       s->Sender_Tspec_Object.link_delay                      &&
       sender_descriptor.Sender_Tspec_Object.req_bandwidth ==
       s->Sender_Tspec_Object.req_bandwidth)
       return true;
       return false;
}

void PathErrorMessage::setSenderTspec(SenderTspecObj_t* s)
{
	sender_descriptor.Sender_Tspec_Object.link_delay = s->link_delay;
	sender_descriptor.Sender_Tspec_Object.req_bandwidth=s->req_bandwidth;
}

void PathErrorMessage::setSenderTemplate(SenderTemplateObj_t* s)
{
	sender_descriptor.Sender_Template_Object.SrcAddress = s->SrcAddress;
	sender_descriptor.Sender_Template_Object.SrcPort    = s->SrcPort;
	sender_descriptor.Sender_Template_Object.Lsp_Id     = s->Lsp_Id;
}

/********************************RESV ERROR MESSAGE*************************/

ResvErrorMessage::ResvErrorMessage(): RSVPPacket()
{
	setKind(RERROR_MESSAGE);
}

void ResvErrorMessage::setHop(RsvpHopObj_t* h)
{
	rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
	rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}

void ResvErrorMessage::setContent(ResvErrorMessage* rMsg)
{
	setSession(rMsg->getSession());
	setHop(rMsg->getHop());
	setErrorCode(rMsg->getErrorCode());
	setErrorNode(rMsg->getErrorNode());

}




