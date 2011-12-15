
package inet.applications.udpapp;


import inet.applications.IUDPApp;

// ChatApp by Leonardo Maccari. 
// This application is an extension of UDPBasicBurst to support:
// - bi-directional bursts
// - multi-session burts
// - random destination choice
// - default values for parameters resembling real chat applications
// in order to be used as a Chat application simulator. The statistics were taken from the papers:
//  Planetary-scale views on a large instant-messaging network
//  Understanding Instant Messaging Traffic Characteristics
// the scale factors are adjusted in order to have a realistic statistic
// compared to the duration of the simulation which makes an average of 
//  one session initiated every 500s
//  session duration of around 500s
//  inter-message wait around 20-30s
//  message length around 90B
//
// Some limitations have been introduced, the inter-message time can not make a message be sent
// beyond the limit of burst duration, the inter-session time is upper bounded to 2 hours.
// This is because the distribution in the paper do fit the behavior only in a 
// certain range, and to make the simulation scalable.  

// It basically works as UDPBasicBurst but:
// - every time a node receives a session (a burst), it starts a session from the originator as answer. The two sessions are indipendent
// - the time between two start of sessions is used as a parameter, not the pause time (as in the papers)
// - each node can have multiple session in parallel 
// - I've added a simple random destination assignment. This can be used as something like destAddresses="random(host)", 
//   supposing that the node is structured like XXXNet.host[X].udpApp[]. 
simple ChatApp like IUDPApp
{
    parameters:
        string chooseDestAddrMode @enum("once","perBurst") = default("perBurst"); // when to change the destination address
        int localPort; // local UDP port number
        int destPort; // remote UDP port number        
        double startTime @unit("s") = default(0s); // application start time (start of the first burst)
        double stopTime @unit("s") = default(-1s); // application stop time (no packets are sent after this time); a negative or zero value means no limit
        // chat traffic features
        double intervalUpperBound @unit("s") = default(7200s); // hom much is the upper bound between two chat sessions? needed to limitate pareto decay
        volatile int messageLength @unit("B") = default(weibull(100B,2.5B)) ; // length of messages to generate, in bytes
        volatile double burstDuration @unit("s") = default(pareto_shifted(2.7,300s,0s)); // burst duration time (zero not allowed)
        volatile double burstInterval @unit("s") = default(pareto_shifted(0.53,300s,0s)); // time between the beginning of two bursts (zero allowed)
        volatile double messageFreq @unit("s") = default(weibull(30s,1s)); // time between messages during bursts; usually a random value, e.g. 0.1s+uniform(-0.001s,0.001s); zero not allowed
        
        double delayLimit @unit("s") = default(0); // maximum accepted delay for a packet; packets with a bigger delay are discarded (dropped), zero value means no limit
        bool debugStats = default(false);
        
        @statistic[sentPkBytes](title="packets sent"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[rcvdPkBytes](title="packets received"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[dropPkBytes](title="packets dropped"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[outOfOrderPk](title="packets received out of order"; record=count,"vector(count)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay";record=histogram,stats,vector);
        // testing the statistic behavior
        
        @statistic[sendInterval](title="Send interval";record=histogram);
        @statistic[bDuration](title="Burst Duration";record=histogram);
        @statistic[bInterval](title="Burst Interval";record=histogram);
        @statistic[messageSize](title="Message size";record=histogram);
        @statistic[startedSessions](title="Started sessions"; record=histogram);
        @statistic[answeredSessions](title="Answered Sessions"; record=histogram);
        @statistic[numUDPErrorsSignal](title="UDP Errors"; record=last);
        @statistic[targetStatisticsSignal](title="Target statistics"; record=histogram);
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);
}

