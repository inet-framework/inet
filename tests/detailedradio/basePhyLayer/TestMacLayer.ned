package inet.tests.basePhyLayer;

import inet.linklayer.contract.IMACProtocol;
import inet.linklayer.common.MACProtocolBase;

simple TestMacLayer extends MACProtocolBase like IMACProtocol
{
    parameters:
        @class(TestMacLayer);
        // TODO: headerLength = 16bit;
}

