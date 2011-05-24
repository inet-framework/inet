/**
 * @short Implementation of a simple packets forward function for IEEE 802.15.4 star network
 *  support device <-> PAN coordinator <-> device transmission
    MAC address translation will be done in MAC layer (refer to Ieee802154Mac::handleUpperMsg())
 * @author Feng Chen
*/

#ifndef IEEE_802154_STAR_ROUTING_H
#define IEEE_802154_STAR_ROUTING_H

#include "Ieee802154AppPkt_m.h"
#include "Ieee802154NetworkCtrlInfo_m.h"

class Ieee802154StarRouting : public cSimpleModule
{
  public:
    virtual void initialize(int);
    virtual void finish();

  protected:
    // Message handle functions
    void                handleMessage           (cMessage*);

    // debugging enabled for this node? Used in the definition of EV
    bool                m_debug;
    bool                isPANCoor;
    const char*     m_moduleName;

    // module gate ID
    int             mUppergateIn;
    int             mUppergateOut;
    int             mLowergateIn;
    int             mLowergateOut;

    // for statistical data
    double          numForward;
};
#endif

