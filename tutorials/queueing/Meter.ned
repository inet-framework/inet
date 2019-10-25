
package inet.tutorials.queueing;

import inet.queueing.source.ActivePacketSource;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.filter.RateLimiter;
import inet.queueing.meter.RateMeter;

network MeterNetwork
{
    @display("bgb=800,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        meter: RateMeter {
            @display("p=300,100");
        }
        limiter: RateLimiter {
            @display("p=500,100");
        }
        consumer: PassivePacketSink {
            @display("p=700,100");
        }
    connections allowunconnected:
        producer.out --> meter.in;
        meter.out --> limiter.in;
        limiter.out --> consumer.in;
}
