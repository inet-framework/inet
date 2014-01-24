package inet.tests.testUtils;

import inet.tests.TestObject;

//Database module for TestModules. This module has to be present
//in you simulation as a global module if you wan't to use the class 
//TestModule
simple TestManager extends TestObject
{
    parameters:
        @class(TestManager);
    @display("i=block/segm");
}
