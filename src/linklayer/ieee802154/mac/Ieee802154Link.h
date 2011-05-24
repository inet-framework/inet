
#ifndef IEEE_802154_LINK_H
#define IEEE_802154_LINK_H

#include "Ieee802154Def.h"

#define hl_oper_del 1
#define hl_oper_est 2
#define hl_oper_rpl 3


// for pkt duplication detection
class HLISTLINK
{
  public:
    UINT_16 hostID;     // source address
    UINT_8 SN;              //SN of packet last received
    HLISTLINK *last;
    HLISTLINK *next;
    HLISTLINK(UINT_16 hostid, UINT_8 sn)
    {
        hostID = hostid;
        SN = sn;
        last = 0;
        next = 0;
    }
};

int     addHListLink            (HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn);
int     updateHListLink     (int oper, HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn = 0);
int     chkAddUpdHListLink      (HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn);
void        emptyHListLink          (HLISTLINK **hlistLink1, HLISTLINK **hlistLink2);
void        dumpHListLink           (HLISTLINK *hlistLink1, UINT_16 hostid);

// for device association

// for transaction (indirect transmission)
#endif
