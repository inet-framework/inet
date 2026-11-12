package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;

//
// Two ad-hoc hosts, 20 m apart, with a wireless interface, for the tests of
// the error models of the receivers. The type of the radio medium is
// selected by the test.
//
network StochasticErrorModelTest
{
    submodules:
        configurator: Ipv4NetworkConfigurator;
        radioMedium: <> like IRadioMedium;
        source: AdhocHost;
        sink: AdhocHost;
}
