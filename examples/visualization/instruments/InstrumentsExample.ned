//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.visualization.instruments;

import inet.common.misc.SignalSource;


network InstrumentsExample
{
    parameters:
        @display("bgb=800,400");

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

        @figure[image](type=indexedImage; pos=220,65; size=16,16; images=status/off,status/green,status/red);
        @statistic[image](source=signalSourceOutput; record=figure; targetFigure=image); // note: indexedImage takes the value modulo the number of images

        signalSource.value = default(dropUnit(simTime()));

    submodules:
        signalSource: SignalSource {
            @display("p=50,350;is=s");
        }
}

network InstrumentsExample2 extends InstrumentsExample
{
    parameters:
        @figure[text](textFormat="value=%g [text]"; initialValue=0);
        @figure[label](textFormat="value=%g [label]"; initialValue=0);
        @figure[gauge](backgroundColor=lightBlue; needleColor=yellow; initialValue=0; minValue=-10; maxValue=10; tickSize=2; label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey"; colorStrip=green 0.6 yellow 0.9 red);
        @figure[lineargauge](backgroundColor=lightBlue; needleColor=magenta; initialValue=0; cornerRadius=5; minValue=-10; maxValue=10; tickSize=2; label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[progress](backgroundColor=white; initialValue=0; stripColor=darkorange; text="%g  (%.4g%%)"; textFont="Arial,14pt,bold"; textColor="blue4"; cornerRadius=5; borderWidth=4; minValue=-10; maxValue=10; label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[counter](backgroundColor=white; initialValue=0; decimalPlaces=5; digitBackgroundColor=lightGreen; digitBorderColor=grey; digitFont="Arial,20pt,bold"; digitColor=grey;  label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[thermometer](mercuryColor=firebrick; initialValue=0; minValue=-10; maxValue=10; tickSize=2; label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey");
        @figure[image](images=trafficlight_off,trafficlight_red,trafficlight_yellow,trafficlight_green,trafficlight_redyellow; initialValue=9; size=-,-; opacity=0.8; tintColor=yellow; tintAmount=0.1; labelOffset=1, 35; label="some label"; labelFont="Arial,10pt,italic"; labelColor="grey");

        signalSource.value = 12*sin(0.05 * dropUnit(simTime()));
}
