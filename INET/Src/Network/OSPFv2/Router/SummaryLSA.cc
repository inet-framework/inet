#include "LSA.h"

bool OSPF::SummaryLSA::Update (const OSPFSummaryLSA* lsa)
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

bool OSPF::SummaryLSA::DiffersFrom (const OSPFSummaryLSA* summaryLSA) const
{
    const OSPFLSAHeader& lsaHeader = summaryLSA->getHeader ();
    bool differentHeader = ((header_var.getLsOptions () != lsaHeader.getLsOptions ()) ||
                            ((header_var.getLsAge () == MAX_AGE) && (lsaHeader.getLsAge () != MAX_AGE)) ||
                            ((header_var.getLsAge () != MAX_AGE) && (lsaHeader.getLsAge () == MAX_AGE)) ||
                            (header_var.getLength () != lsaHeader.getLength ()));
    bool differentBody   = false;

    if (!differentHeader) {
        differentBody = ((networkMask_var != summaryLSA->getNetworkMask ()) ||
                         (routeCost_var != summaryLSA->getRouteCost ()) ||
                         (tosData_arraysize != summaryLSA->getTosDataArraySize ()));

        if (!differentBody) {
            unsigned int tosCount = tosData_arraysize;
            for (unsigned int i = 0; i < tosCount; i++) {
                if ((tosData_var[i].tos != summaryLSA->getTosData (i).tos) ||
                    (tosData_var[i].tosMetric[0] != summaryLSA->getTosData (i).tosMetric[0]) ||
                    (tosData_var[i].tosMetric[1] != summaryLSA->getTosData (i).tosMetric[1]) ||
                    (tosData_var[i].tosMetric[2] != summaryLSA->getTosData (i).tosMetric[2]))
                {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return (differentHeader || differentBody);
}
