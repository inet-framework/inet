
#include "Ieee802154Link.h"

// Operations for HListLink
int addHListLink(HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn)
{
    HListLink *tmp;

    if (*hlistLink2 == 0)       //not exist yet
    {
        *hlistLink2 = new HListLink(hostid, sn);

        if (*hlistLink2 == 0)
            return 1;

        *hlistLink1 = *hlistLink2;
    }
    else
    {
        tmp = new HListLink(hostid, sn);

        if (tmp == 0)
            return 1;

        tmp->last = *hlistLink2;
        (*hlistLink2)->next = tmp;
        *hlistLink2 = tmp;
    }
    return 0;
}

int updateHListLink(int oper, HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn)
{
    HListLink *tmp;
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

int chkAddUpdHListLink(HListLink **hlistLink1, HListLink **hlistLink2, uint16_t hostid, uint8_t sn)
{
    int i;

    i = updateHListLink(hl_oper_rpl, hlistLink1, hlistLink2, hostid, sn);

    if (i == 0)
        return 1;
    else if (i == 2)
        return 2;

    i = addHListLink(hlistLink1, hlistLink2, hostid, sn);
    if (i == 0)
        return 0;
    else
        return 3;
}

void emptyHListLink(HListLink **hlistLink1, HListLink **hlistLink2)
{
    HListLink *tmp, *tmp2;

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

void dumpHListLink(HListLink *hlistLink1, uint16_t hostid)
{
    HListLink *tmp;
    int i;

    tmp = hlistLink1;
    i = 1;

    while (tmp != 0)
    {
        tmp = tmp->next;
        i++;
    }
}
