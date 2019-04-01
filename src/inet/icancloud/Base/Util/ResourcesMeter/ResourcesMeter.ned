// This module performs measures of resources like RAM, CPU, etc...
//
// This module simulates a disk using Lineal Interpolation.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Base.Util.ResourcesMeter;

simple ResourcesMeter

{
    parameters:
        @class(icancloud::ResourcesMeter);
        string outputFile;			// File to write data
        double timer;		// Timer (in sec.)

        @display("i=block/classifier");
}
