//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.applications.udpapp;

//
// This module is a generic request/response based server application. For each
// request it receives, it generates a different traffic based on the data the
// request contains. The client application can be any source that is capable
// of generating packets with different data. The first byte of the packet data
// determines the response traffic, which can be configured to produce complex
// streams of packets with various data and timing distributions.
//
module UdpRequestResponseApp extends UdpApp
{
    parameters:
        int numKind = default(2);
        sink.typename = default("RequestConsumer");
        sink.numKind = this.numKind;
        sink.responseProducerModule = default("^.source");
        sink.classifier.classifierClass = default("inet::queueing::PacketDataClassifier");
        source.typename = default("ResponseProducer");
        source.numKind = this.numKind;
}
