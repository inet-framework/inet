package inet.test.moduletest.lib;

channel PacketLoggerChannel extends ned.DatarateChannel
{
    @class(PacketLoggerChannel);
    string logfile = default("");   // logfile name
}
