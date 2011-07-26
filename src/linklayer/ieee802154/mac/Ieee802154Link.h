
#ifndef IEEE_802154_LINK_H
#define IEEE_802154_LINK_H

#include "INETDefs.h"

#include "Ieee802154Def.h"

#define hl_oper_del 1
#define hl_oper_est 2
#define hl_oper_rpl 3


// for pkt duplication detection
class HListLink
{
  public:
    uint16_t hostID;     // source address
    uint8_t SN;              //SN of packet last received
    HListLink *last;
    HListLink *next;

    HListLink(uint16_t hostid, uint8_t sn)
    {
        hostID = hostid;
        SN = sn;
        last = 0;
        next = 0;
    }
};

int addHListLink(HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn);
int updateHListLink(int oper, HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn = 0);
int chkAddUpdHListLink(HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn);
void emptyHListLink(HListLink **hlistLink1, HListLink **hlistLink2);
void dumpHListLink(HListLink *hlistLink1, uint16_t hostid);

// for device association

// for transaction (indirect transmission)
#endif

