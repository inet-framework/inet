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


/*******************************PATH MESSAGE***************************/

RSVPPathMsg::RSVPPathMsg():RSVPPacket()
{
    setKind(PATH_MESSAGE);
}


bool RSVPPathMsg::equalST(SenderTemplateObj_t * s)
{
    if (sender_descriptor.Sender_Template_Object.SrcAddress ==
        s->SrcAddress &&
        sender_descriptor.Sender_Template_Object.SrcPort ==
        s->SrcPort && sender_descriptor.Sender_Template_Object.Lsp_Id == s->Lsp_Id)
        return true;
    return false;

}

bool RSVPPathMsg::equalSD(SenderDescriptor_t * s)
{
    if (sender_descriptor.Sender_Template_Object.SrcAddress ==
        s->Sender_Template_Object.SrcAddress &&
        sender_descriptor.Sender_Template_Object.SrcPort ==
        s->Sender_Template_Object.SrcPort &&
        sender_descriptor.Sender_Template_Object.Lsp_Id ==
        s->Sender_Template_Object.Lsp_Id &&
        sender_descriptor.Sender_Tspec_Object.link_delay ==
        s->Sender_Tspec_Object.link_delay &&
        sender_descriptor.Sender_Tspec_Object.req_bandwidth == s->Sender_Tspec_Object.req_bandwidth)
        return true;
    return false;
}




void RSVPPathMsg::setHop(RsvpHopObj_t * h)
{
    rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
    rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;
}

void RSVPPathMsg::setSenderTspec(SenderTspecObj_t * s)
{
    sender_descriptor.Sender_Tspec_Object.link_delay = s->link_delay;
    sender_descriptor.Sender_Tspec_Object.req_bandwidth = s->req_bandwidth;
}

void RSVPPathMsg::setSenderTemplate(SenderTemplateObj_t * s)
{
    sender_descriptor.Sender_Template_Object.SrcAddress = s->SrcAddress;
    sender_descriptor.Sender_Template_Object.SrcPort = s->SrcPort;
    sender_descriptor.Sender_Template_Object.Lsp_Id = s->Lsp_Id;
}

void RSVPPathMsg::print()
{
    ev << "DestAddr = " << IPAddress(getDestAddress()) << "\n" <<
        "ProtId   = " << getProtId() << "\n" <<
        "DestPort = " << getDestPort() << "\n" <<
        "SrcAddr  = " << IPAddress(getSrcAddress()) << "\n" <<
        "SrcPort  = " << getSrcPort() << "\n" <<
        "Lsp_Id = " << getLspId() << "\n" <<
        "Next Hop = " << IPAddress(getNHOP()) << "\n" <<
        "LIH      = " << IPAddress(getLIH()) << "\n" <<
        "Delay    = " << getDelay() << "\n" << "Bandwidth= " << getBW() << "\n";

}

void RSVPPathMsg::setContent(RSVPPathMsg * pMsg)
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

RSVPResvMsg::RSVPResvMsg():RSVPPacket()
{
    setKind(RESV_MESSAGE);
}



void RSVPResvMsg::setHop(RsvpHopObj_t * h)
{
    rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
    rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;
}

void RSVPResvMsg::setContent(RSVPResvMsg * rMsg)
{
    setSession(rMsg->getSession());
    setHop(rMsg->getHop());
    setFlowDescriptor(rMsg->getFlowDescriptor());
    setStyle(rMsg->getStyle());
}


void RSVPResvMsg::print()
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
                    ",OutLabel=" << flow_descriptor_list[i].label <<
                    ", BW=" << flow_descriptor_list[i].Flowspec_Object.req_bandwidth <<
                    ", Delay=" << flow_descriptor_list[i].Flowspec_Object.link_delay << "\n";
                ev << "RRO={";
                for (int c = 0; c < MAX_ROUTE; c++)
                {
                    int rroEle = flow_descriptor_list[i].RRO[c];
                    if (rroEle != 0)
                        ev << IPAddress(rroEle) << ",";
                }
                ev << "}\n";
            }


}

/********************************PATH TEAR MESSAGE*************************/
RSVPPathTear::RSVPPathTear():RSVPPacket()
{
    setKind(PTEAR_MESSAGE);
}

void RSVPPathTear::setHop(RsvpHopObj_t * h)
{
    rsvp_hop.Logical_Interface_Handle = h->Logical_Interface_Handle;
    rsvp_hop.Next_Hop_Address = h->Next_Hop_Address;

}

void RSVPPathTear::setContent(RSVPPathTear * pMsg)
{
    setSession(pMsg->getSession());
    setHop(pMsg->getHop());
    setSenderTemplate(pMsg->getSenderTemplate());




}

void RSVPPathTear::setSenderTemplate(SenderTemplateObj_t * s)
{
    senderTemplate.SrcAddress = s->SrcAddress;
    senderTemplate.SrcPort = s->SrcPort;
}


bool RSVPPathTear::equalST(SenderTemplateObj_t * s)
{
    if (senderTemplate.SrcAddress == s->SrcAddress && senderTemplate.SrcPort == s->SrcPort)
        return true;
    return false;

}


void RSVPPathTear::print()
{
    ev << "DestAddr = " << IPAddress(getDestAddress()) << "\n" <<
        "ProtId   = " << getProtId() << "\n" <<
        "DestPort = " << getDestPort() << "\n" <<
        "SrcAddr  = " << IPAddress(getSrcAddress()) << "\n" <<
        "SrcPort  = " << getSrcPort() << "\n" <<
        "Next Hop = " << IPAddress(getNHOP()) << "\n" <<
        "LIH      = " << IPAddress(getLIH()) << "\n";


}

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

void RSVPResvTear::setContent(RSVPResvTear * rMsg)
{
    setSession(rMsg->getSession());
    setHop(rMsg->getHop());
    setFlowDescriptor(rMsg->getFlowDescriptor());
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

RSVPPathError::RSVPPathError():RSVPPacket()
{
    setKind(PERROR_MESSAGE);

}

void RSVPPathError::setContent(RSVPPathError * pMsg)
{
    setSession(pMsg->getSession());
    setErrorCode(pMsg->getErrorCode());
    setErrorNode(pMsg->getErrorNode());
    setSenderTspec(pMsg->getSenderTspec());
    setSenderTemplate(pMsg->getSenderTemplate());

}

bool RSVPPathError::equalST(SenderTemplateObj_t * s)
{
    if (sender_descriptor.Sender_Template_Object.SrcAddress ==
        s->SrcAddress && sender_descriptor.Sender_Template_Object.SrcPort == s->SrcPort)
        return true;
    return false;

}

bool RSVPPathError::equalSD(SenderDescriptor_t * s)
{
    if (sender_descriptor.Sender_Template_Object.SrcAddress ==
        s->Sender_Template_Object.SrcAddress &&
        sender_descriptor.Sender_Template_Object.SrcPort ==
        s->Sender_Template_Object.SrcPort &&
        sender_descriptor.Sender_Tspec_Object.link_delay ==
        s->Sender_Tspec_Object.link_delay &&
        sender_descriptor.Sender_Tspec_Object.req_bandwidth == s->Sender_Tspec_Object.req_bandwidth)
        return true;
    return false;
}

void RSVPPathError::setSenderTspec(SenderTspecObj_t * s)
{
    sender_descriptor.Sender_Tspec_Object.link_delay = s->link_delay;
    sender_descriptor.Sender_Tspec_Object.req_bandwidth = s->req_bandwidth;
}

void RSVPPathError::setSenderTemplate(SenderTemplateObj_t * s)
{
    sender_descriptor.Sender_Template_Object.SrcAddress = s->SrcAddress;
    sender_descriptor.Sender_Template_Object.SrcPort = s->SrcPort;
    sender_descriptor.Sender_Template_Object.Lsp_Id = s->Lsp_Id;
}

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

void RSVPResvError::setContent(RSVPResvError * rMsg)
{
    setSession(rMsg->getSession());
    setHop(rMsg->getHop());
    setErrorCode(rMsg->getErrorCode());
    setErrorNode(rMsg->getErrorNode());

}
