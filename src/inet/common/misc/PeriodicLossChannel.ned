package inet.common.misc;

//
// This channel uses the per parameter for periodic packet loss. That means it
// delivers exactly 1/per packets, followed by one drop. It does not use the
// BER paramater.
//
channel PeriodicLossChannel extends ned.DatarateChannel
{
    @class(PeriodicLossChannel);
}

