/**
 * @simple traffic generator to test IEEE 802.15.4 protocols
*/

#ifndef IEEE_802154_TEST_APP
#define IEEE_802154_TEST_APP

#include "TrafGenPar.h"
#include "Ieee802154AppPkt_m.h"
//#include "Ieee802154UpperCtrlInfo_m.h"

class Ieee802154TestApp : public TrafGenPar
{
  public:

    // LIFECYCLE
    // this takes care of constructors and destructors

    virtual void initialize(int);
    virtual void finish();

  protected:

    // OPERATIONS
    virtual void handleSelfMsg(cMessage*);
    virtual void handleLowerMsg(cMessage*);

    virtual void SendTraf(cPacket *msg, const char*);

  private:
    bool    m_debug;        // debug switch
    int     mLowergateIn;
    int     mLowergateOut;

    int     mCurrentTrafficPattern;

    double  mNumTrafficMsgs;
    double  mNumTrafficMsgRcvd;
    double  mNumTrafficMsgNotDelivered;

    const char* m_moduleName;
    simtime_t   sumE2EDelay;
    double  numReceived;
    double  totalByteRecv;
    cOutVector e2eDelayVec;
    cOutVector meanE2EDelayVec;


};

#endif
