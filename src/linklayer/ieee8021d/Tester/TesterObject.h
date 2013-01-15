 /**
******************************************************
* @file TesterObject.h
* @brief Test messages generator
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __A_TESTEROBJECT_H
#define __A_TESTEROBJECT_H

#include "BPDU.h"
#include "MACAddress.h"
#include "EtherFrame_m.h"

//Parsing definitions
enum
{
	BPDU_MESSAGE,
	MVRPDU_MESSAGE,
	ETHERNETII_MESSAGE,
	ETHERNET1Q_MESSAGE,
	ETHERNET1AD_MESSAGE,
	ETHERNET1AS_MESSAGE,
	ETHERNET1AH_MESSAGE
};


/**
 * TesterObject
 */
class TesterObject: public cSimpleModule
{
  protected:
	  /*Dynamic data.*/
	  /*Static data. */
	  bool verbose;
	  bool testing;
	  bool detailedRSTP;
	  bool detailedMVRP;
	  bool detailedEthII;
	  bool detailedEth1Q;
	  bool detailedEth1ad;
	  bool detailedEth1ah;


	  //Statistics
	  int ReceivedMessages;
	  int SentMessages;
	  int ReceivedBPDUs;
	  int ReceivedMVRPDUs;
	  int ReceivedEthernetII;
	  int ReceivedEthernet1Q;
	  int ReceivedEthernet1AD;
	  int ReceivedEthernet1AH;

	  MVRPDU * LastMVRPDU;
	  EthernetIIFrame * LastEthII;
	  EthernetIIFrame * LastEth1Q;
	  EthernetIIFrame * LastEth1ad;
	  EthernetIIFrame * LastEth1ah;

	  //Details
	  int ReceivedTCNs;

  public:
    TesterObject() {}
    virtual ~TesterObject();
    virtual void finish();
    virtual void printState();

  protected:
    virtual void initialize(int stage);

    /**
     * Generic message handling.
     */
    virtual void handleMessage(cMessage *);

    /**
     * BPDU handling
     */
    virtual void handleIncomingFrame(BPDUieee8021D *);

    /**
     * MVRPDU handling
     */
    virtual void handleIncomingFrame(MVRPDU *);

    /**
     * EthernetIIFrame handling
     */
    virtual void handleIncomingFrame(EthernetIIFrame *);

    /**
     * Message scheduler
     */
    virtual void scheduleMessage(cXMLElement *);

};




#endif

