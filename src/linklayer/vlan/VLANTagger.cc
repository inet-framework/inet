///
/// @file   VLANTagger.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/5/2012
///
/// @brief  Implements 'VLANTagger' class for VLAN tagging based on IEEE 802.1Q.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

#include "VLANTagger.h"

// Register modules.
Define_Module(VLANTagger);

VLANTagger::VLANTagger()
{
}

VLANTagger::~VLANTagger()
{
}

void VLANTagger::initialize()
{
    // verbose = par("verbose");
    tagged = (bool) par("tagged");
    dynamicTagging = (bool) par("dynamicTagging");
    pvid = (VID) par("pvid");

    if (tagged == false)
    {
        if ((pvid >= 1) && (pvid <= 4094))
        {
            if (dynamicTagging == true)
            {
                // get an access to the relay module for dynamic VLAN tagging
                cModule *relayModule = getParentModule()->getSubmodule("relay");
                relay = check_and_cast<MACRelayUnitNPWithVLAN *>(relayModule);
            }
        }
        else
        {
            error("PVID value is beyond the allowed range of VID values.");
        }
    }
    else
    {
        vidSet.clear();
        std::string vids = par("vidSet").stdstringValue();
        if (vids.size() > 0)
        {
            // parse 'vids' and add results into 'vidSet'
            int i = 0;
            std::string::size_type idx1 = 0, idx2 = 0;
            while ((idx1 != std::string::npos) && (idx2 != std::string::npos))
            {
                idx2 = vids.find(' ', idx1);
                vidSet.push_back(atoi((vids.substr(idx1, idx2)).c_str()));
                if (idx2 != std::string::npos)
                {
                    idx1 = vids.find_first_not_of(' ', idx2 + 1);
                }
                i++;
            }
        }
    }
}

void VLANTagger::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        error("No self message in VLANTagger.");
    }
    
    if (msg->getArrivalGateId() == findGate("ethg$i"))
    {
        // ingress rule checking
        if (tagged)
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
            {
                VID vid = dynamic_cast<EthernetIIFrameWithVLAN *>(msg)->getVid();
                if ((vid == 0) || (vid == 4095))    // 0 (null VID) or 4095 (xFFF; reserved VID) is not allowed
                {
                    EV << "";
                    delete msg;
                }
                else
                {
                    // processTaggedFrame(msg);
                    send(msg, "relayg$o");
                }
            }
            else
            {
                EV << "Tagged port cannot receive a frame without a VLAN tag. Drop.";
                delete msg;
            }
        }
        else
        {
            if (dynamic_cast<EthernetIIFrame *>(msg) != NULL)
            {
                TagFrame(check_and_cast<EthernetIIFrame *>(msg));
                // processTaggedFrame(msg);
                send(msg, "relayg$o");
            }
            else
            {
                EV << "Untagged port cannot receive a frame with a VLAN tag. Drop.";
                delete msg;
            }
        }
    }
    else if (msg->getArrivalGateId() == findGate("relayg$i"))
    {
        // egress rule checking
        if (tagged)
        {
            // processTaggedFrame(msg);
            send(msg, "macg$o");
        }
        else
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
            {
                UntagFrame(check_and_cast<EthernetIIFrameWithVLAN *>(msg));
                // processTaggedFrame(msg);
                send(msg, "macg$o");
            }
            else
            {
                error("Unknown frame. Modify VLANTagger to make it allowed or filtered.");
                delete msg;
            }
        }            
    }
    else
    {
        error("Unknown arrival gate");            
    }
}

EthernetIIFrameWithVLAN *VLANTagger::TagFrame(EthernetIIFrame *frame)
{
    EthernetIIFrameWithVLAN *vlanFrame = new EthernetIIFrameWithVLAN;
    vlanFrame->setDest(frame->getDest());
    vlanFrame->setSrc(frame->getSrc());
    vlanFrame->setEtherType(frame->getEtherType());
    if (dynamicTagging == true)
    {
        // TODO: dynamic tagging based on the VLAN address table

        vlanFrame->setVid(0);
    }
    else
    {
        // static VLAN tagging
        vlanFrame->setVid(pvid);
    }
    vlanFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_VLAN_TAG_LENGTH);
    cPacket * temp = frame->decapsulate();
    if (temp != NULL)
        vlanFrame->encapsulate(temp);
    delete frame;
    return vlanFrame;
}

EthernetIIFrame *VLANTagger::UntagFrame(EthernetIIFrameWithVLAN *vlanFrame)
{
    return check_and_cast<EthernetIIFrame *>(vlanFrame);
}

