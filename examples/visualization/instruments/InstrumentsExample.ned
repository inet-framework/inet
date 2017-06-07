package inet.examples.visualization.instruments;

import inet.common.misc.SignalSource;
import inet.common.figures.DelegateSignalConfigurator;

network InstrumentsExample
{
    parameters:
        @display("bgb=800,450");

        @figure[text](type=indicatorText; pos=20,50; anchor=w);
        @statistic[text](source=signalSourceOutput; record=figure; targetFigure=text);

        @figure[label](type=indicatorLabel; pos=20,80; anchor=w);
        @statistic[label](source=signalSourceOutput; record=figure; targetFigure=label);

        @figure[gauge](type=gauge; pos=50,100; size=150,150);
        @statistic[gauge](source=signalSourceOutput; record=figure; targetFigure=gauge);

        @figure[lineargauge](type=linearGauge; pos=300,50; size=300,50);
        @statistic[lineargauge](source=signalSourceOutput; record=figure; targetFigure=lineargauge);

        @figure[progress](type=progressMeter; pos=300,150; size=300,20);
        @statistic[progress](source=signalSourceOutput; record=figure; targetFigure=progress);

        @figure[counter](type=counter; pos=300,250);
        @statistic[counter](source=signalSourceOutput; record=figure; targetFigure=counter);

        @figure[thermometer](type=thermometer; pos=700,50; size=50,300);
        @statistic[thermometer](source=signalSourceOutput; record=figure; targetFigure=thermometer);

        @figure[image](type=indexedImage; pos=240,280; size=16,16; images=status/off,status/green,status/red);
        @statistic[image](source=signalSourceOutput; record=figure; targetFigure=image); // note: indexedImage takes the value modulo the number of images

        @figure[plot](type=plot; pos=420,270; size=200,100);
        @statistic[plot](source=signalSourceOutput; record=figure; targetFigure=plot);

        signalSource.value = default(dropUnit(simTime()));

    submodules:
        signalSource: SignalSource {
            @display("p=50,350;is=s");
        }
}

network InstrumentsExample2 extends InstrumentsExample
{
    parameters:
        @figure[text](textFormat="Value=%g (An Indicator Text)"; initialValue=0);
        @figure[label](textFormat="Value=%g (An Indicator Label)"; initialValue=0);
        @figure[gauge](backgroundColor=lightBlue; needleColor=yellow; initialValue=0; minValue=-10; maxValue=10; tickSize=2; label="A Gauge"; labelFont="Arial,10pt,italic"; labelColor="grey"; colorStrip=green 0.6 yellow 0.9 red);
        @figure[lineargauge](backgroundColor=lightBlue; needleColor=magenta; initialValue=0; cornerRadius=5; minValue=-10; maxValue=10; tickSize=2; label="A Linear Gauge"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[progress](backgroundColor=white; initialValue=0; stripColor=darkorange; text="%g  (%.4g%%)"; textFont="Arial,14pt,bold"; textColor="blue4"; cornerRadius=5; borderWidth=4; minValue=-10; maxValue=10; label="A Progress Meter"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[counter](backgroundColor=white; initialValue=0; decimalPlaces=5; digitBackgroundColor=lightGreen; digitBorderColor=grey; digitFont="Arial,20pt,bold"; digitColor=grey;  label="A Counter"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[thermometer](mercuryColor=firebrick; initialValue=0; minValue=-10; maxValue=10; tickSize=2; label="A Thermometer"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[image](images=trafficlight_off,trafficlight_red,trafficlight_yellow,trafficlight_green,trafficlight_redyellow; initialValue=9; size=-,-; opacity=0.8; tintColor=yellow; tintAmount=0.1; labelOffset=1, 35; label="An Indexed Image"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[plot](backgroundColor=lightBlue; lineColor=red3; minValue=-10; maxValue=10; valueTickSize=5; timeWindow=100; timeTickSize=20; label="A Plot"; labelFont="Arial,10pt,italic"; labelColor="grey");

        signalSource.value = 12*sin(0.05 * dropUnit(simTime()));
}
