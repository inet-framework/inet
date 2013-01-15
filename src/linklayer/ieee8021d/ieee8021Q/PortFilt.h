/**
******************************************************
* @file PortFilt.h
* @brief 802.1Q Tagging and filtering skills
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __INET_PortFilt1Q_H
#define __INET_PortFilt1Q_H

#include "Ethernet.h"
#include "EtherFrame_m.h"
#include "8021Q.h"


/**
 * Tagging skills
 */
class PortFilt : public cSimpleModule
{
  protected:
	bool tagged;		///Port type.
	bool verbose;
	int cost;			///RSTP port cost.
	int priority;		///RSTP port priority
	vid defaultVID;		///default VID in case untagged.
	simtime_t interFrameTime;  /// inter MVRPDU time.

  public:
	PortFilt();
	~PortFilt();
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    /**
     * Checking and calling tagging and untagging functions
     */
    virtual void processTaggedFrame(cMessage *msg);

    /**
     * 802.1Q Tagging
     */
    virtual void TagFrame(EthernetIIFrame *msg);

    /**
     * 802.1Q Untagging
     */
    virtual void UntagFrame(EthernetIIFrame *msg);
    /**
     * MVRPDU sending
     */
    virtual void sendMVRPDUs();

 // utility functions
    /**
     * Shows port type at Tkenv.
     */
    virtual void updateDisplayString();

  public:
    /**
     * @return RSTP port cost.
     */
    virtual int getCost();

    /**
     * @return RSTP port priority.
     */
    virtual int getPriority();

    /**
     * @return true if it is an edge port.
     */
    virtual bool isEdge();
};

#endif


