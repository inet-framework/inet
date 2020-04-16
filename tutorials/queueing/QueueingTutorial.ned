package inet.tutorials.queueing;

import inet.queueing.buffer.PacketBuffer;
import inet.queueing.buffer.PriorityBuffer;
import inet.queueing.classifier.ContentBasedClassifier;
import inet.queueing.classifier.LabelClassifier;
import inet.queueing.classifier.MarkovClassifier;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.classifier.PriorityClassifier;
import inet.queueing.classifier.WrrClassifier;
import inet.queueing.common.OrdinalBasedDuplicator;
import inet.queueing.common.PacketCloner;
import inet.queueing.common.PacketDelayer;
import inet.queueing.common.PacketDemultiplexer;
import inet.queueing.common.PacketDuplicator;
import inet.queueing.common.PacketGate;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSink;
import inet.queueing.contract.IPassivePacketSource;
import inet.queueing.filter.ContentBasedFilter;
import inet.queueing.filter.OrdinalBasedDropper;
import inet.queueing.filter.RateLimiter;
import inet.queueing.filter.RedDropper;
import inet.queueing.marker.ContentBasedLabeler;
import inet.queueing.marker.ContentBasedTagger;
import inet.queueing.marker.PacketTagger;
import inet.queueing.meter.RateMeter;
import inet.queueing.queue.CompoundPacketQueue;
import inet.queueing.queue.DropTailQueue;
import inet.queueing.queue.PacketQueue;
import inet.queueing.queue.PriorityQueue;
import inet.queueing.scheduler.ContentBasedScheduler;
import inet.queueing.scheduler.MarkovScheduler;
import inet.queueing.scheduler.PacketScheduler;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.scheduler.WrrScheduler;
import inet.queueing.server.PacketServer;
import inet.queueing.server.TokenBasedServer;
import inet.queueing.shaper.LeakyBucket;
import inet.queueing.shaper.TokenBucket;
import inet.queueing.sink.ActivePacketSink;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.sink.RequestConsumer;
import inet.queueing.source.ActivePacketSource;
import inet.queueing.source.PassivePacketSource;
import inet.queueing.source.QueueFiller;
import inet.queueing.source.ResponseProducer;
import inet.queueing.tokengenerator.PacketBasedTokenGenerator;
import inet.queueing.tokengenerator.QueueBasedTokenGenerator;
import inet.queueing.tokengenerator.SignalBasedTokenGenerator;
import inet.queueing.tokengenerator.TimeBasedTokenGenerator;


network ProducerConsumerTutorialStep
{
    @display("bgb=400,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections:
        producer.out --> consumer.in;
}

//-------------------------------------------------

network ProviderCollectorTutorialStep
{
    @display("bgb=400,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        collector: ActivePacketSink {
            @display("p=300,100");
        }
    connections:
        provider.out --> collector.in;
}

//-------------------------------------------------

network PacketQueueTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: PacketQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network DropTailQueueTutorialStep
{
    @display("bgb=700,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: DropTailQueue {
            @display("p=325,100");
        }
        collector: ActivePacketSink {
            @display("p=550,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network ComparatorTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: PacketQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network BufferTutorialStep
{
    submodules:
        buffer: PacketBuffer {
            @display("p=125,350");
        }
        producer1: ActivePacketSource {
            @display("p=125,100");
        }
        producer2: ActivePacketSource {
            @display("p=125,225");
        }
        queue1: PacketQueue {
            @display("p=350,100");
            bufferModule = "^.buffer";
        }
        queue2: PacketQueue {
            @display("p=350,225");
            bufferModule = "^.buffer";
        }
        collector1: ActivePacketSink {
            @display("p=575,100");
        }
        collector2: ActivePacketSink {
            @display("p=575,225");
        }
    connections:
        producer1.out --> queue1.in;
        queue1.out --> collector1.in;
        producer2.out --> queue2.in;
        queue2.out --> collector2.in;
}

//-------------------------------------------------

network PriorityClassifierTutorialStep
{
    @display("bgb=850,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: PriorityClassifier {
            @display("p=300,100");
        }
        queue1: PacketQueue {
            @display("p=525,100");
        }
        queue2: PacketQueue {
            @display("p=525,225");
        }
        collector1: ActivePacketSink {
            @display("p=750,100");
        }
        collector2: ActivePacketSink {
            @display("p=750,225");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        queue1.out --> collector1.in;
        queue2.out --> collector2.in;
}

//-------------------------------------------------

network WrrClassifierTutorialStep
{
    @display("bgb=600,400");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: WrrClassifier {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
        consumer3: PassivePacketSink {
            @display("p=500,300");
        }
        
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
        classifier.out++ --> consumer3.in;
}

//-------------------------------------------------

network ContentBasedClassifierTutorialStep
{
    @display("bgb=600,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: ContentBasedClassifier {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}

//-------------------------------------------------


network MarkovClassifierTutorialStep
{
    @display("bgb=600,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: MarkovClassifier {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}

//-------------------------------------------------

network GenericClassifierTutorialStep
{
    @display("bgb=600,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        classifier: PacketClassifier {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        producer.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}

//-------------------------------------------------

network PrioritySchedulerTutorialStep
{
    @display("bgb=850,300");
    submodules:
        producer1: ActivePacketSource {
            @display("p=100,100");
        }
        producer2: ActivePacketSource {
            @display("p=100,225");
        }
        queue1: PacketQueue {
            @display("p=325,100");
        }
        queue2: PacketQueue {
            @display("p=325,225");
        }
        scheduler: PriorityScheduler {
            @display("p=550,100");
        }
        collector: ActivePacketSink {
            @display("p=750,100");
        }
    connections allowunconnected:
        producer1.out --> queue1.in;
        producer2.out --> queue2.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}

//-------------------------------------------------

network WrrSchedulerTutorialStep
{
    @display("bgb=600,400");
    submodules:
        provider1: PassivePacketSource {
            @display("p=100,100");
        }
        provider2: PassivePacketSource {
            @display("p=100,200");
        }
        provider3: PassivePacketSource {
            @display("p=100,300");
        }
        
        scheduler: WrrScheduler {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider1.out --> scheduler.in++;
        provider2.out --> scheduler.in++;
        provider3.out --> scheduler.in++;
        scheduler.out --> collector.in;
}

//-------------------------------------------------

network ContentBasedSchedulerTutorialStep
{
    @display("bgb=600,300");
    submodules:
        provider1: PassivePacketSource {
            @display("p=100,100");
        }
        provider2: PassivePacketSource {
            @display("p=100,200");
        }
        scheduler: ContentBasedScheduler {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider1.out --> scheduler.in++;
        provider2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}

//-------------------------------------------------

network MarkovSchedulerTutorialStep
{
    @display("bgb=600,300");
    submodules:
        provider1: PassivePacketSource {
            @display("p=100,100");
        }
        provider2: PassivePacketSource {
            @display("p=100,200");
        }
        scheduler: MarkovScheduler {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider1.out --> scheduler.in++;
        provider2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}

//-------------------------------------------------

network GenericSchedulerTutorialStep
{
    @display("bgb=600,300");
    submodules:
        provider1: PassivePacketSource {
            @display("p=100,100");
        }
        provider2: PassivePacketSource {
            @display("p=100,200");
        }
        scheduler: PacketScheduler {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider1.out --> scheduler.in++;
        provider2.out --> scheduler.in++;
        scheduler.out --> collector.in;
}

//-------------------------------------------------

network PriorityBufferTutorialStep
{
    submodules:
        buffer: PriorityBuffer {
            @display("p=125,350");
        }
        producer1: ActivePacketSource {
            @display("p=125,100");
        }
        producer2: ActivePacketSource {
            @display("p=125,225");
        }
        queue1: PacketQueue {
            @display("p=325,100");
            bufferModule = "^.buffer";
        }
        queue2: PacketQueue {
            @display("p=325,225");
            bufferModule = "^.buffer";
        }
        collector1: ActivePacketSink {
            @display("p=525,100");
        }
        collector2: ActivePacketSink {
            @display("p=525,225");
        }
    connections:
        producer1.out --> queue1.in;
        queue1.out --> collector1.in;
        producer2.out --> queue2.in;
        queue2.out --> collector2.in;
}

//-------------------------------------------------


network PriorityQueueTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: PriorityQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network CompoundPacketQueueTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        queue: ExampleCompoundPriorityQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------


module ExampleCompoundPriorityQueue extends CompoundPacketQueue
{
    parameters:
        @class(::inet::queueing::CompoundPacketQueue);
    submodules:
        classifier: WrrClassifier {
            @display("p=100,100");
        }
        queue1: PacketQueue {
            @display("p=325,100");
        }
        queue2: PacketQueue {
            @display("p=325,225");
        }
        queue3: PacketQueue {
            @display("p=475,350");
        }
        consumer: PassivePacketSink {
            @display("p=175,350");
        }
        producer: ActivePacketSource {
            @display("p=325,350");
        }            
        scheduler: PriorityScheduler {
            @display("p=550,100");
        }
    connections:
        in --> { @display("m=w"); } --> classifier.in;
        classifier.out++ --> queue1.in;
        classifier.out++ --> queue2.in;
        classifier.out++ --> consumer.in;
        queue1.out --> scheduler.in++;
        queue2.out --> scheduler.in++;
        producer.out --> queue3.in;
        queue3.out --> scheduler.in++;
        scheduler.out --> { @display("m=e"); } --> out;
}

//-------------------------------------------------

network Filter1TutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        filter: ContentBasedFilter {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> filter.in;
        filter.out --> consumer.in;
}

//-------------------------------------------------


network Filter2TutorialStep
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        filter: ContentBasedFilter {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider.out --> filter.in;
        filter.out --> collector.in;
}

//-------------------------------------------------

network OrdinalBasedDropperTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        dropper: OrdinalBasedDropper {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> dropper.in;
        dropper.out --> consumer.in;
}

//-------------------------------------------------

network RedDropperTutorialStep
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        dropper: RedDropper {
            @display("p=300,100");
        }
        queue: PacketQueue {
            @display("p=500,100");
        }
        collector: ActivePacketSink {
            @display("p=700,100");
        }
    connections:
        producer.out --> dropper.in;
        dropper.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network ServerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        server: PacketServer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> consumer.in;
}

//-------------------------------------------------

network TokenBasedServerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        server: TokenBasedServer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> consumer.in;
}

//-------------------------------------------------

network TimeBasedTokenGeneratorTutorialStep
{
    @display("bgb=600,350");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,125");
        }
        server: TokenBasedServer {
            @display("p=300,125");
        }
        consumer: PassivePacketSink {
            @display("p=500,125");
        }
        tokenGenerator: TimeBasedTokenGenerator {
            @display("p=100,250");
            serverModule = "^.server";
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> consumer.in;
}

//-------------------------------------------------

network PacketBasedTokenGeneratorTutorialStep
{
    @display("bgb=1000,250");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,125");
        }
        provider: PassivePacketSource {
            @display("p=500,125");
        }
        server: TokenBasedServer {
            @display("p=700,125");
        }
        consumer: PassivePacketSink {
            @display("p=900,125");
        }
        tokenGenerator: PacketBasedTokenGenerator {
            @display("p=300,125");
            serverModule = "^.server";
        }
    connections allowunconnected:
        producer.out --> tokenGenerator.in;
        provider.out --> server.in;
        server.out --> consumer.in;
}

//-------------------------------------------------

network QueueBasedTokenGeneratorTutorialStep
{
    @display("bgb=875,350");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,125");
        }
        server: TokenBasedServer {
            @display("p=325,125");
        }
        queue: PacketQueue {
            @display("p=550,125");
        }
        collector: ActivePacketSink {
            @display("p=775,125");
        }
        tokenGenerator: QueueBasedTokenGenerator {
            @display("p=100,250");
            serverModule = "^.server";
            queueModule = "^.queue";
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------

network SignalBasedTokenGeneratorTutorialStep
{
    @display("bgb=600,350");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,125");
        }
        server: TokenBasedServer {
            @display("p=300,125");
        }
        consumer: PassivePacketSink {
            @display("p=500,125");
        }
        tokenGenerator: SignalBasedTokenGenerator {
            @display("p=100,250");
            signals = "packetDropped";
            subscriptionModule = "^.consumer";
            serverModule = "^.server";
        }
    connections allowunconnected:
        provider.out --> server.in;
        server.out --> consumer.in;
}

//-------------------------------------------------

network MeterTutorialStep
{
    @display("bgb=875,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        meter: RateMeter {
            @display("p=325,100");
        }
        limiter: RateLimiter {
            @display("p=550,100");
        }
        consumer: PassivePacketSink {
            @display("p=775,100");
        }
    connections allowunconnected:
        producer.out --> meter.in;
        meter.out --> limiter.in;
        limiter.out --> consumer.in;
}

//-------------------------------------------------

network TaggerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        tagger: PacketTagger {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> tagger.in;
        tagger.out --> consumer.in;
}

//-------------------------------------------------


network ContentBasedTaggerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        tagger: ContentBasedTagger {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> tagger.in;
        tagger.out --> consumer.in;
}

//-------------------------------------------------

network LabelerTutorialStep
{
    @display("bgb=800,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        labeler: ContentBasedLabeler {
            @display("p=300,100");
        }
        classifier: LabelClassifier {
            @display("p=500,100");
        }
        consumer1: PassivePacketSink {
            @display("p=700,100");
        }
        consumer2: PassivePacketSink {
            @display("p=700,200");
        }
    connections allowunconnected:
        producer.out --> labeler.in;
        labeler.out --> classifier.in;
        classifier.out++ --> consumer1.in;
        classifier.out++ --> consumer2.in;
}

//-------------------------------------------------

network LeakyBucketTutorialStep
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        bucket: LeakyBucket {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections allowunconnected:
        producer.out --> bucket.in;
        bucket.out --> consumer.in;
}

//-------------------------------------------------

network TokenBucketTutorialStep
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        bucket: TokenBucket {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
        tokenGenerator: TimeBasedTokenGenerator {
            @display("p=100,200");
            serverModule = "^.bucket.server";
        }
    connections allowunconnected:
        producer.out --> bucket.in;
        bucket.out --> consumer.in;
}

//-------------------------------------------------

network DelayerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        delayer: PacketDelayer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> delayer.in;
        delayer.out --> consumer.in;
}

//-------------------------------------------------

network MultiplexerTutorialStep
{
    parameters:
        int numProducers;
    submodules:
        producer[numProducers]: ActivePacketSource {
            @display("p=100,100,c,100");
        }
        multiplexer: PacketMultiplexer {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        for i=0..numProducers-1 {
            producer[i].out --> multiplexer.in++;
        }
        multiplexer.out --> consumer.in;
}

//-------------------------------------------------


network DemultiplexerTutorialStep
{
    parameters:
        int numCollectors;
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        demultiplexer: PacketDemultiplexer {
            @display("p=300,100");
        }
        collector[numCollectors]: ActivePacketSink {
            @display("p=500,100,c,100");
        }
    connections allowunconnected:
        provider.out --> demultiplexer.in;
        for i=0..numCollectors-1 {
            demultiplexer.out++ --> collector[i].in;
        }
}

//-------------------------------------------------

network Gate1TutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        gate: PacketGate {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections:
        producer.out --> gate.in;
        gate.out --> consumer.in;
}

//-------------------------------------------------


network Gate2TutorialStep
{
    @display("bgb=600,200");
    submodules:
        provider: PassivePacketSource {
            @display("p=100,100");
        }
        gate: PacketGate {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections:
        provider.out --> gate.in;
        gate.out --> collector.in;
}

//-------------------------------------------------


network DuplicatorTutorialStep
{
    @display("bgb=600,200");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        duplicator: PacketDuplicator {
            @display("p=300,100");
        }
        consumer: PassivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        producer.out --> duplicator.in;
        duplicator.out --> consumer.in;
}

//-------------------------------------------------

network OrdinalBasedDuplicatorTutorialStep
{
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        duplicator: OrdinalBasedDuplicator {
            @display("p=200,100");
        }
        consumer: PassivePacketSink {
            @display("p=300,100");
        }
    connections:
        producer.out --> duplicator.in;
        duplicator.out --> consumer.in;
}

//-------------------------------------------------

network ClonerTutorialStep
{
    @display("bgb=600,300");
    submodules:
        producer: ActivePacketSource {
            @display("p=100,100");
        }
        cloner: PacketCloner {
            @display("p=300,100");
        }
        consumer1: PassivePacketSink {
            @display("p=500,100");
        }
        consumer2: PassivePacketSink {
            @display("p=500,200");
        }
    connections allowunconnected:
        producer.out --> cloner.in;
        cloner.out++ --> consumer1.in;
        cloner.out++ --> consumer2.in;
}

//-------------------------------------------------


network QueueFillerTutorialStep
{
    @display("bgb=600,200");
    submodules:
        filler: QueueFiller {
            @display("p=100,100");
            tokenGenerator.queueModule = "queue";
        }
        queue: PacketQueue {
            @display("p=300,100");
        }
        collector: ActivePacketSink {
            @display("p=500,100");
        }
    connections allowunconnected:
        filler.out --> queue.in;
        queue.out --> collector.in;
}

//-------------------------------------------------


network RequestResponseTutorialStep
{
    @display("bgb=400,300");
    submodules:
        requestProducer: ActivePacketSource {
            @display("p=100,100");
        }
        responseConsumer: PassivePacketSink {
            @display("p=300,100");
        }
        requestConsumer: RequestConsumer {
            @display("p=100,200");
            responseProducerModule = "^.responseProducer";
        }
        responseProducer: ResponseProducer {
            @display("p=300,200");
            tokenGenerator.serverModule = "^.^.requestConsumer.server";
        }
    connections allowunconnected:
        requestProducer.out --> requestConsumer.in;
        responseProducer.out --> responseConsumer.in;
}

//-------------------------------------------------


network TelnetTutorialStep
{
    @display("bgb=400,200");
    submodules:
        client: TelnetClientTraffic {
            @display("p=100,100");
        }
        server: TelnetServerTraffic {
            @display("p=300,100");
        }
    connections:
        client.out --> server.in;
        client.in <-- server.out;
}

//-------------------------------------------------


module TelnetClientTraffic
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        characterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = intuniform(97, 122); // lower case ASCII characters
                productionInterval = uniform(0.1s, 0.2s); // typing speed between 5 and 10 characters per second
                @display("p=100,100");
        }
        enterProducer: <default("ActivePacketSource")> like IActivePacketSource {
            parameters:
                packetLength = 1B;
                packetData = 13; // enter character
                productionInterval = 0.1s;
                @display("p=300,100");
        }
        emptyProducer: <default("EmptyPacketSource")> like IActivePacketSource {
            parameters:
                @display("p=500,100");
        }
        scheduler: <default("MarkovScheduler")> like IPacketScheduler {
            parameters:
                transitionProbabilities = "0 1 0 0 0 1 1 0 0"; // character -> enter -> wait -> character
                waitIntervals = "uniform(0,3) 0 uniform(10,30)";
                @display("p=300,200");
        }
        consumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=100,200");
        }
    connections:
        characterProducer.out --> scheduler.in++;
        enterProducer.out --> scheduler.in++;
        emptyProducer.out --> scheduler.in++;
        scheduler.out --> { @display("m=s"); } --> out;
        in --> { @display("m=s"); } --> consumer.in;
}

//-------------------------------------------------


module TelnetServerTraffic
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        cloner: PacketCloner {
            parameters:
                @display("p=300,225");
        }
        responseProvider: <default("PassivePacketSource")> like IPassivePacketSource {
            parameters:
                @display("p=100,100");
        }
        responseServer: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=100,225");
        }
        multiplexer: PacketMultiplexer {
            parameters:
                @display("p=200,350");
        }
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            parameters:
                classifierClass = default("inet::queueing::PacketCharacterOrEnterClassifier");
                @display("p=500,225");
        }
        characterConsumer: <default("PassivePacketSink")> like IPassivePacketSink {
            parameters:
                @display("p=400,350");
        }
        enterTokenGenerator: PacketBasedTokenGenerator {
            parameters:
                serverModule = default("^.responseServer");
                @display("p=600,350");
        }
    connections:
        in --> { @display("m=s"); } --> cloner.in;
        cloner.out++ --> classifier.in;
        cloner.out++ --> multiplexer.in++;
        responseProvider.out --> responseServer.in;
        responseServer.out --> multiplexer.in++;
        classifier.out++ --> characterConsumer.in;
        classifier.out++ --> enterTokenGenerator.in;
        multiplexer.out --> { @display("m=s"); } --> out;
}

//-------------------------------------------------


module ExampleInterface
{
    @display("bgb=400,300;i=device/card");
    gates:
        input lowerIn;
        input upperIn;
        output lowerOut;
        output upperOut;
    submodules:
        queue: PacketQueue {
            @display("p=200,100");
        }
        server: PacketServer {
            @display("p=200,225");
        }
    connections:
        upperIn --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=s"); } --> lowerOut;
        lowerIn --> { @display("m=m,66,100,66,0"); } --> upperOut;
}

//-------------------------------------------------


module ExampleHost
{
    parameters:
        @display("i=device/pc");
    gates:
        input lowerIn;
        output lowerOut;
    submodules:
        sourceApplication: ActivePacketSource {
            @display("p=100,100");
        }
        destinationApplication: PassivePacketSink {
            @display("p=300,100");
        }
        interface: ExampleInterface {
            @display("p=200,200");
        }
    connections:
        sourceApplication.out --> interface.upperIn;
        interface.lowerOut --> lowerOut;
        lowerIn --> interface.lowerIn;
        interface.upperOut --> destinationApplication.in;
}

//-------------------------------------------------


module ExampleCable
{
    parameters:
        @display("i=block/mac");
    gates:
        input aIn;
        output aOut;
        input bIn;
        output bOut;
    submodules:
        aDelayer: PacketDelayer {
            @display("p=100,100");
        }
        bDelayer: PacketDelayer {
            @display("p=100,200");
        }
    connections:
        aIn --> { @display("m=w"); } --> aDelayer.in;
        aDelayer.out --> { @display("m=e"); } --> aOut;
        bIn --> { @display("m=e"); } --> bDelayer.in;
        bDelayer.out --> { @display("m=w"); } --> bOut;
}

//-------------------------------------------------

network ExampleNetworkTutorialStep
{
    submodules:
        source: ExampleHost {
            @display("p=100,100");
        }
        cable: ExampleCable {
            @display("p=200,100");
        }
        destination: ExampleHost {
            @display("p=300,100");
        }
    connections:
        source.lowerOut --> cable.aIn;
        cable.aOut --> destination.lowerIn;
        destination.lowerOut --> cable.bIn;
        cable.bOut --> source.lowerIn;
}

//-------------------------------------------------