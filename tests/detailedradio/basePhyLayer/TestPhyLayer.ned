package inet.tests.basePhyLayer;

import inet.physicallayer.radio.detailed.DetailedRadio;

simple TestPhyLayer extends DetailedRadio
{
    parameters:
    	@class(TestPhyLayer);
    	int protocol = default(0);
}
