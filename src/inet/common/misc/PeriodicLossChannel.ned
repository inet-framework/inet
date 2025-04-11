package inet.common.misc;

//
// This channel uses the per parameter for periodic packet loss. That means it
// drops exactly every 1/per packet. It does not use the BER paramater.
//
channel PeriodicLossChannel extends ned.DatarateChannel
{
    @class(PeriodicLossChannel);
}

