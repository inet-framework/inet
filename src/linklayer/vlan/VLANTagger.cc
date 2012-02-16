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

void
VLANTagger::initialize()
{
    tagged = (bool) par("tagged");
    dynamicTagging = (bool) par("dynamicTagging");
    minVid = (int) par("minVid");
    maxVid = (int) par("maxVid");

    // assign the range of VID values into 'vidSet'
    // FIXME allow arbitrary set of VID values later
    vidSet.clear();
    for (int i = minVid; i <= maxVid; i++)
    {
        vidSet.push_back((VID)i);
    }
//    // parse 'vids' and add results into 'vidSet'
//    vidSet.clear();
//    std::string vids = par("vidSet").stdstringValue();
//    if (vids.size() > 0)
//    {
//        VID vid;
//        int i = 0;
//        std::string::size_type idx1 = 0, idx2 = 0;
//        while ((idx1 != std::string::npos) && (idx2 != std::string::npos))
//        {
//            idx2 = vids.find(' ', idx1);
//            vid = (VID) atoi((vids.substr(idx1, idx2)).c_str());
//            if ((vid < 1) || (vid > 4094))
//            {
//                error("A VID value is beyond the allowed range.");
//            }
//            else
//            {
//                vidSet.push_back(vid);
//            }
//            if (idx2 != std::string::npos)
//            {
//                idx1 = vids.find_first_not_of(' ', idx2 + 1);
//            }
//            i++;
//        }
//    }

    if (tagged == false)
    {
        if (dynamicTagging == false)
        { // untagged port with static tagging
            if (vidSet.size() == 1)
            {
                pvid = vidSet[0];
            }
            else
            {
                error("No or multiple PVIDs for untagged port with static tagging.");
            }
        }
        else
        { // get an access to the relay module for untagged port with dynamic tagging
            cModule *relayModule = getParentModule()->getSubmodule("relayUnit");
            relay = check_and_cast<MACRelayUnitNPWithVLAN *>(relayModule);
        }
    }
}

void VLANTagger::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        error("No self message in VLANTagger.");
    }
    
    if (msg->getArrivalGateId() == findGate("macg$i"))
    {
        // ingress rule checking
        if (tagged)
        {
            if (dynamic_cast<EthernetIIFrameWithVLAN *>(msg) != NULL)
            {
                VID vid = dynamic_cast<EthernetIIFrameWithVLAN *>(msg)->getVid();
                if ((vid == 0) || (vid == 4095))
                {
                    EV << "VID of 0 (null) or 4095 (reserved) is not allowed.\n";
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
                EV << "Tagged port cannot receive a frame without a VLAN tag. Drop.\n";
                delete msg;
            }
        }
        else
        {
            if (dynamic_cast<EthernetIIFrame *>(msg) != NULL)
            {
            	MACAddress address = ((EthernetIIFrame *)msg)->getDest();
            	VLANFrameVector vlanFrames;
            	TagFrame((EthernetIIFrame *)msg, vlanFrames);
//                EthernetIIFrameWithVLAN *vlanFrame = TagFrame((EthernetIIFrame *)msg);
            	VLANFrameVector::iterator it;
            	for (it=vlanFrames.begin(); it < vlanFrames.end(); it++)
            	{
            	    send(*it, "relayg$o");
            	}
//                if (vlanFrame != NULL)
//                {
//                	send(vlanFrame, "relayg$o");
//                }
//                else
//                {
//                	EV << "There is no VID registered/learned for MAC address:" << address << endl;
//                }
            }
            else
            {
                EV << "Untagged port cannot receive a frame with a VLAN tag. Drop.\n";
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

void VLANTagger::TagFrame(EthernetIIFrame *frame, VLANFrameVector& vlanFrames)
{
    VIDVector vids;

    if (dynamicTagging == true)
    {   // dynamic tagging based on the VLAN address table
    	int vid = relay->getVIDForMACAddress(frame->getDest());
        if (vid < 0)
        {   // either there is no matching VID for the MAC address or the frame is for broadcasting
            vids = vidSet;
        }
        else
        {
            vids.push_back(vid);
        }
    }
    else
    {   // static tagging based on PVID
        vids.push_back(pvid);
    }

    EthernetIIFrameWithVLAN *vlanFrame = new EthernetIIFrameWithVLAN;
    vlanFrame->setDest(frame->getDest());
    vlanFrame->setSrc(frame->getSrc());
    vlanFrame->setEtherType(frame->getEtherType());
    vlanFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_VLAN_TAG_LENGTH);
    cPacket * temp = frame->decapsulate();
    if (temp != NULL)
        vlanFrame->encapsulate(temp);
    delete frame;

    VIDVector::iterator it;
    for (it = vids.begin(); it < vids.end(); it++)
    {
        EthernetIIFrameWithVLAN *vf = vlanFrame->dup();
        vf->setVid(*it);
        vlanFrames.push_back(vf);
    }
    delete vlanFrame;
}

EthernetIIFrame *VLANTagger::UntagFrame(EthernetIIFrameWithVLAN *vlanFrame)
{
    return check_and_cast<EthernetIIFrame *>(vlanFrame);
}
