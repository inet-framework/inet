package inet.physicallayer.pathloss;

import inet.physicallayer.contract.packetlevel.IPathLoss;

module TwoRayInterference like IPathLoss
{
    parameters:
        @class(TwoRayInterference);
        @display("i=block/control");
        double epsilon_r = default(1.02);
}
