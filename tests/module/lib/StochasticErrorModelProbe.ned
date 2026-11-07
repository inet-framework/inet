package inet.test.moduletest.lib;

import inet.physicallayer.wireless.common.errormodel.packetlevel.StochasticErrorModel;

//
// StochasticErrorModel that records the smallest and the largest value of
// every rate and signal part that it returned (and of the header and data
// parts of every packet) as scalars, for the module tests.
//
module StochasticErrorModelProbe extends StochasticErrorModel
{
    parameters:
        @class(StochasticErrorModelProbe);
}
