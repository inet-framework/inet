// from MPLS models  --- FIXME merge or eliminate!

simple MyTCPApp
{
    parameters:
        string local_addr;
        string server_addr;
        volatile double timeout;
        volatile double data_interarrival_time;
        volatile double start_time;
    gates:
        input in;
        output out;
}
