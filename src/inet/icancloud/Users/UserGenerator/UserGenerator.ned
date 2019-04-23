//
// Module that implements an user generator. 
// 
// It is defined by a number of cells and a set of cells. Each cell define a group os users with a behaviour that will appear
// at system following a statistical distribution.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator;

import inet.icancloud.Users.UserGenerator.**.*;
import inet.icancloud.Users.UserGenerator.core.UserGenerator_cell;

module UserGenerator
{
    parameters:
        int numCells;
        @display("i=abstract/people;bgl=2");

    submodules:
        cell[numCells]: UserGenerator_cell {
            parameters:
                @display("i=icons/Redirector");
        }

}
