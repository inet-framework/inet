package inet.test.moduletest.lib;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// Error model that returns the packet error rate and the bit error rate as
// they are set, even if they are inconsistent with each other.
//
module ConstantRatesErrorModel extends ErrorModelBase
{
    parameters:
        double packetErrorRate = default(nan);
        double bitErrorRate = default(nan);
        bool initializeBase = default(true); // false: do not call ErrorModelBase::initialize()
        @class(ConstantRatesErrorModel);
}
