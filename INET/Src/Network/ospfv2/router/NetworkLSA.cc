#include "LSA.h"

bool OSPF::NetworkLSA::Update (const OSPFNetworkLSA* lsa)
{
    bool different = DiffersFrom (lsa);
    (*this) = (*lsa);
    ResetInstallTime ();
    if (different) {
        ClearNextHops ();
        return true;
    } else {
        return false;
    }
}

bool OSPF::NetworkLSA::DiffersFrom (const OSPFNetworkLSA* networkLSA) const
{
    const OSPFLSAHeader& lsaHeader = networkLSA->getHeader ();
    bool differentHeader = ((header_var.getLsOptions () != lsaHeader.getLsOptions ()) ||
                            ((header_var.getLsAge () == MAX_AGE) && (lsaHeader.getLsAge () != MAX_AGE)) ||
                            ((header_var.getLsAge () != MAX_AGE) && (lsaHeader.getLsAge () == MAX_AGE)) ||
                            (header_var.getLength () != lsaHeader.getLength ()));
    bool differentBody   = false;

    if (!differentHeader) {
        differentBody = ((networkMask_var != networkLSA->getNetworkMask ()) ||
                         (attachedRouters_arraysize != networkLSA->getAttachedRoutersArraySize ()));

        if (!differentBody) {
            unsigned int routerCount = attachedRouters_arraysize;
            for (unsigned int i = 0; i < routerCount; i++) {
                if (attachedRouters_var[i] != networkLSA->getAttachedRouters (i)) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return (differentHeader || differentBody);
}
