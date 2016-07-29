package artery.inet.physical;

import inet.physicallayer.contract.packetlevel.IPathLoss;

module TwoRayInterference like IPathLoss
{
    parameters:
        @class(inet::physicallayer::TwoRayInterference);
        @display("i=block/control");
        double epsilon_r = default(1.02);
}
