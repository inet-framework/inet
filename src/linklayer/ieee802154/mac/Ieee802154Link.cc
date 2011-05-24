#include "Ieee802154Link.h"

// Operations for HLISTLINK
int addHListLink(HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn)
{
    HLISTLINK *tmp;
    if (*hlistLink2 == 0)       //not exist yet
    {
        *hlistLink2 = new HLISTLINK(hostid, sn);
        if (*hlistLink2 == 0) return 1;
        *hlistLink1 = *hlistLink2;
    }
    else
    {
        tmp=new HLISTLINK(hostid, sn);
        if (tmp == 0) return 1;
        tmp->last = *hlistLink2;
        (*hlistLink2)->next = tmp;
        *hlistLink2 = tmp;
    }
    return 0;
}

int updateHListLink(int oper, HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn)
{
    HLISTLINK *tmp;
    int ok;

    ok = 1;

    tmp = *hlistLink1;
    while (tmp != 0)
    {
        if (tmp->hostID == hostid)
        {
            if (oper == hl_oper_del)    //delete an element
            {
                if (tmp->last != 0)
                {
                    tmp->last->next = tmp->next;
                    if (tmp->next != 0)
                        tmp->next->last = tmp->last;
                    else
                        *hlistLink2 = tmp->last;
                }
                else if (tmp->next != 0)
                {
                    *hlistLink1 = tmp->next;
                    tmp->next->last = 0;
                }
                else
                {
                    *hlistLink1 = 0;
                    *hlistLink2 = 0;
                }
                delete tmp;
            }
            if (oper == hl_oper_rpl)    //replace
            {
                if (tmp->SN != sn)
                    tmp->SN = sn;
                else
                {
                    ok = 2;
                    break;
                }
            }
            ok = 0;
            break;
        }
        tmp = tmp->next;
    }
    return ok;
}

int chkAddUpdHListLink(HLISTLINK **hlistLink1, HLISTLINK **hlistLink2, UINT_16 hostid, UINT_8 sn)
{
    int i;

    i = updateHListLink(hl_oper_rpl, hlistLink1, hlistLink2, hostid, sn);
    if (i == 0) return 1;
    else if (i == 2) return 2;

    i = addHListLink(hlistLink1, hlistLink2, hostid, sn);
    if (i == 0) return 0;
    else return 3;
}

void emptyHListLink(HLISTLINK **hlistLink1, HLISTLINK **hlistLink2)
{
    HLISTLINK *tmp, *tmp2;

    if (*hlistLink1 != 0)
    {
        tmp = *hlistLink1;
        while (tmp != 0)
        {
            tmp2 = tmp;
            tmp = tmp->next;
            delete tmp2;
        }
        *hlistLink1 = 0;
    }
    *hlistLink2 = *hlistLink1;
}

void dumpHListLink(HLISTLINK *hlistLink1, UINT_16 hostid)
{
    HLISTLINK *tmp;
    int i;

    tmp = hlistLink1;
    i = 1;
    while (tmp != 0)
    {
        tmp = tmp->next;
        i++;
    }
}
