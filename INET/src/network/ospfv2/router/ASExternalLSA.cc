#include "LSA.h"

bool OSPF::ASExternalLSA::Update (const OSPFASExternalLSA* lsa)
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

bool OSPF::ASExternalLSA::DiffersFrom (const OSPFASExternalLSA* asExternalLSA) const
{
    const OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader ();
    bool differentHeader = ((header_var.getLsOptions () != lsaHeader.getLsOptions ()) ||
                            ((header_var.getLsAge () == MAX_AGE) && (lsaHeader.getLsAge () != MAX_AGE)) ||
                            ((header_var.getLsAge () != MAX_AGE) && (lsaHeader.getLsAge () == MAX_AGE)) ||
                            (header_var.getLength () != lsaHeader.getLength ()));
    bool differentBody   = false;

    if (!differentHeader) {
        const OSPFASExternalLSAContents& lsaContents  = asExternalLSA->getContents ();
        unsigned int                     tosInfoCount = contents_var.getExternalTOSInfoArraySize ();

        differentBody = ((contents_var.getNetworkMask () != lsaContents.getNetworkMask ()) ||
                         (contents_var.getE_ExternalMetricType () != lsaContents.getE_ExternalMetricType ()) ||
                         (contents_var.getRouteCost () != lsaContents.getRouteCost ()) ||
                         (contents_var.getForwardingAddress () != lsaContents.getForwardingAddress ()) ||
                         (contents_var.getExternalRouteTag () != lsaContents.getExternalRouteTag ()) ||
                         (tosInfoCount != lsaContents.getExternalTOSInfoArraySize ()));

        if (!differentBody) {
            for (unsigned int i = 0; i < tosInfoCount; i++) {
                const ExternalTOSInfo& currentTOSInfo = contents_var.getExternalTOSInfo (i);
                const ExternalTOSInfo& lsaTOSInfo     = lsaContents.getExternalTOSInfo (i);

                if ((currentTOSInfo.tosData.tos != lsaTOSInfo.tosData.tos) ||
                    (currentTOSInfo.tosData.tosMetric[0] != lsaTOSInfo.tosData.tosMetric[0]) ||
                    (currentTOSInfo.tosData.tosMetric[1] != lsaTOSInfo.tosData.tosMetric[1]) ||
                    (currentTOSInfo.tosData.tosMetric[2] != lsaTOSInfo.tosData.tosMetric[2]) ||
                    (currentTOSInfo.E_ExternalMetricType != lsaTOSInfo.E_ExternalMetricType) ||
                    (currentTOSInfo.forwardingAddress != lsaTOSInfo.forwardingAddress) ||
                    (currentTOSInfo.externalRouteTag != lsaTOSInfo.externalRouteTag))
                {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return (differentHeader || differentBody);
}
