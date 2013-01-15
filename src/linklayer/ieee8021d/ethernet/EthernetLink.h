 /**
******************************************************
* @file EthernetLink.h
* @brief EthernetIIFrame encapsulation.
* Modified from EtherLLC model.
*  - Removed LLC capabilities.
*  - Just EthernetIIFrames.
*  - Pause frames have also been removed.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/

#ifndef __INET_ETHERLINK_H
#define __INET_ETHERLINK_H

#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "8021Q.h"



/**
 * Implements the LINK upper sub-layer of the Datalink Layer in Ethernet networks. (No LLC skills)
 */
class EthernetLink : public cSimpleModule
{
  protected:
	int seqNum;
	int outputFrame;
	bool verbose;
	vid defaultVID;
	vid defaultSVID;
	simtime_t interFrameTime;

	MACAddress address;

    // statistics
    long totalFromHigherLayer;  // total number of packets received from higher layer
    long totalFromMAC;          // total number of frames received from MAC
    long totalPassedUp;         // total number of packets passed up to higher layer

  public:
    EthernetLink();
    ~EthernetLink();
  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const {return 2;}
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void processPacketFromHigherLayer(cPacket *msg);  //Generates EthernetII, 802.1q, 802.1ad frames with higher layer info
    virtual void processFrameFromMAC(EthernetIIFrame *msg); //Decapsulates application info from ethernetII, 802.1q and 802.1ad frames
    virtual void handleSelfMessage(cMessage * msg);  //MVRPDU timer

    // utility function
    virtual void updateDisplayString();
};

#endif


