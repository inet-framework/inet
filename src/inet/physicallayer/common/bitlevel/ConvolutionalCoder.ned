//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.common.bitlevel;

import inet.physicallayer.contract.bitlevel.IFecCoder;

// <b>Transfer function (octal) matrix</b>
//
// transferFunctionMatrix = "133 171" means a simple vector with two elements:
// [133, 171]. We use the standard octal representation of the industry-standard
// generators of the 1/2 convolutional encoder. You also have to define your own
// polynomials in octal number system!
// transferFunctionMatrix = "3 1 3; 1 2 2" means a k-by-n matrix:
// <pre>
// | 3 1 3 |
// | 1 2 2 |
// </pre>
// which is a very simple 2/3 encoder.
//
// <b>Puncturing (bool) matrix and punctureK, punctureN parameters</b>
//
// The same rule applies to the puncturing matrix representation.
// A puncturing matrix only consists boolean values represented with
// 0 or 1.
// By default we set it to
// <pre>
// | 1 |
// | 1 |
// </pre>
// This simple puncturing matrix, in the 1/2 case, means no puncturing applies.
// You also have to set the new code rate parameters: a new
// <pre>
// | 1 1 |
// | 1 0 |
// </pre>
// puncturing matrix together with the default settings define a 2/3 encoder,
// so the correct values for punctureK and punctureN are 2 and 3.
//
// <b>Constraint length (integer) vector</b>
//
// The constraint length is a 1-by-k vector that specifies the delay for the
// encoder's k input bit streams.
//
// The default settings define the standard 1/2 convolutional encoder used by
// IEEE802.11 PHY.
//
// <b>Encoding/decoding mode</b>
//
// There are two modes:
//  - "truncated" : The trellis graph traceback path always starts in the all-zeros state
//					and ends with the best metric.
//  - "terminated": The trellis graph traceback path always starts and ends in the all-zeros
//					state.
// The encoding process works in accordance with the industry-standards and starts
// in the all-zeros state.
//
module ConvolutionalCoder like IFecCoder
{
    parameters:
        string transferFunctionMatrix = default("133 171");
        string mode = default("truncated");
        string puncturingMatrix = default("1;1");
        int punctureK = default(1);
        int punctureN = default(2);
        string constraintLengthVector = default("7");
        @display("i=block/segm");
        @class(ConvolutionalCoderModule);
}

