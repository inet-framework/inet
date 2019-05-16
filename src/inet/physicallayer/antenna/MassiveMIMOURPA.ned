package inet.physicallayer.antenna;

import inet.physicallayer.base.packetlevel.AntennaBase;



//
// This antenna model describes the well-known dipole antenna or doublet. It
// consists of two identical conductive elements, which are bilaterally
// symmetrical. 
// 
module MassiveMIMOURPA extends AntennaBase
{
    parameters:
        double length @unit(m); // the length of the single antenna
        double distance; // distance between elements
        double freq;
        double phiz; // the steering angle
        int M;  //number of columns
        int N;  //number of rows
        string energySourceModule;
        double interval @unit(s)=default(2s);


        @class(MassiveMIMOURPA);
        @signal[radioModeChangedSignal](type="long");
        @signal[receptionStateChangedSignal](type="long");
        @signal[transmissionStateChangedSignal](type="long");
        @signal[receivedSignalPartChangedSignal](type="long");
        //@statistic[guadagno](title="guadagno"; source="gain"; record=vector; interpolationmode=none);
    submodules:
}