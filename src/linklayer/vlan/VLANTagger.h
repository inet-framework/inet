///
/// @file   VLANTagger.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jan/5/2012
///
/// @brief  Declares 'VLANTagger' class for VLAN tagging based on IEEE 802.1Q.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __INET_VLAN_TAGGER_H
#define __INET_VLAN_TAGGER_H

#include <omnetpp.h>
#include <string>
#include <vector>
#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "MACRelayUnitNPWithVLAN.h"
#include "VLAN.h"

///
/// @class VLANTagger
/// @brief Implements VLAN tagger module based on IEEE 802.1Q, providing tagging/untagging and ingress/egress rule checking.
/// @ingroup vlan
///
class VLANTagger : public cSimpleModule
{
    protected:
        MACRelayUnitNPWithVLAN *relay;    ///< relay module pointer to get access to the VLAN address table for dynamic VLAN tagging
        bool tagged;    ///< port type
        bool dynamicTagging;    ///< (experimental) dynamic tagging for untagged port based on source & destination MAC addresses of incoming frames
        /* bool verbose; */
        int minVid; ///< minimum of the range of allowed VID values
        int maxVid; ///< maximum of the range of allowed VID values
        VID pvid;   ///< Port VLAN Identifier (PVID) for untagged port

        typedef std::vector<VID> VIDVector;
        VIDVector vidSet;   ///< set of VIDs for tagged port

    public:
        VLANTagger();
        ~VLANTagger();
        bool isTagged() const {return tagged;}
        bool isDynamicTagging() const {return dynamicTagging;}
        VID getPvid() const {return pvid;}
        VIDVector getVidSet() const {return vidSet;}

    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);

        typedef std::vector<EthernetIIFrameWithVLAN *> VLANFrameVector;

        // create VLAN-tagged Ethernet frame(s) for a given Ethernet frame
        virtual void TagFrame(EthernetIIFrame *msg, VLANFrameVector& vlanFrames);

        // extract VLAN tag from an Ethernet frame
        virtual EthernetIIFrame *UntagFrame(EthernetIIFrameWithVLAN *msg);
};

#endif // __INET_VLAN_TAGGER_H
