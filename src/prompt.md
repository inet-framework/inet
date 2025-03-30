# Task: Add resolveDirective() Methods to INET Framework Classes

Your task is to add resolveDirective() methods to specific classes in the INET Framework and update their corresponding NED file documentation. The resolveDirective() methods allow modules to display dynamic information in their visualization text.

## Background

In the INET Framework, modules can display information using format strings with directives (like %p, %d, etc.). The resolveDirective() method in a class resolves these directives to actual values that should be displayed.

For example, in MacRelayUnitBase.cc, the resolveDirective() method handles directives 'p' (number of processed frames) and 'd' (number of dropped frames):

```cpp
std::string MacRelayUnitBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(numProcessedFrames);
        case 'd':
            return std::to_string(numDroppedFrames);
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}
```

The corresponding NED file should:
1. Include a default format string in the displayStringTextFormat parameter
2. Document the directives in the main comment above the module definition

For example, in MacRelayUnitBase.ned:

```ned
// MacRelayUnitBase is... (existing documentation)
// ...
//
// The format string can contain the following directives: (this is the new part)
//  - %p number of processed frames
//  - %d number of discarded frames
//  - %t frame processing time
//
simple MacRelayUnitBase
{
    parameters:
        string displayStringTextFormat = default("proc: %p\ndisc: %d");
        // other parameters...
}
```

## Requirements

For each class in the input file:

1. Add a resolveDirective() method to the .cc file that handles the specified directives,
   and also declare it as protected method in the .h file
2. Update the corresponding .ned file to:
   - Document the directives in the main comment above the module definition
   - Set an appropriate default format string in the displayStringTextFormat parameter
3. Generate appropriate descriptions for each directive based on the variable name and purpose

## Input File Format

The input file contains a list of classes and their variables to expose through directives. The format is:

```
path/to/file.cc:ClassName
- variableName - directiveChar

anotherpath/file.cc:AnotherClass
- variableName1 - directiveChar1
- variableName2 - directiveChar2
```

For each class, you need to:
- Examine the code to understand what each variable represents
- Generate appropriate descriptions for each directive
- Implement the resolveDirective() method with proper handling of each variable type
- Update the NED file with documentation and format string

## Example Input File

```
base/MacRelayUnitBase.cc:MacRelayUnitBase
- numProcessedFrames - p
- numDiscardedFrames - d
- processingTime - t

bmac/BMac.cc:BMac
- numSent - s
- numReceived - r
- numCollisions - c
- macState - t
- queueLength - q

common/ExampleQosClassifier.cc:ExampleQosClassifier
- numPacketsProcessed - p
- numClassified - c

common/QosClassifier.cc:QosClassifier
- numPacketsProcessed - p
- numClassified - c
```

## Implementation Guidelines

1. For each class, add a resolveDirective() method that handles the specified directives
2. If the base class already has a resolveDirective() method:
   - Declare the method with the override keyword
   - Call the base class method for unhandled directives instead of throwing an error
3. The method should return a string representation of the corresponding variable
4. For enum types (like macState), convert to a meaningful string representation
5. For classes without a base class implementation, add a default case that throws an error for unknown directives
6. Update the .ned file to document the directives in the main comment above the module definition and set an appropriate default format string
7. Make sure to declare any new member variables needed (e.g., numPacketsProcessed, numClassified)

## Example Implementation

For MacRelayUnitBase, you would:

1. Update MacRelayUnitBase.h to declare the resolveDirective method:

```cpp
// In MacRelayUnitBase.h:
protected:
    // Add this if the base class doesn't have resolveDirective
    virtual std::string resolveDirective(char directive) const;

    // OR add this if the base class already has resolveDirective
    virtual std::string resolveDirective(char directive) const override;

    // Add the new member variable
    simtime_t processingTime;
```

2. Update MacRelayUnitBase.cc to implement the resolveDirective method:

```cpp
// If the base class doesn't have resolveDirective:
std::string MacRelayUnitBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(numProcessedFrames);
        case 'd':
            return std::to_string(numDroppedFrames);
        case 't':
            return std::to_string(processingTime.dbl()) + " s";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

// OR if the base class already has resolveDirective:
std::string MacRelayUnitBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'p':
            return std::to_string(numProcessedFrames);
        case 'd':
            return std::to_string(numDroppedFrames);
        case 't':
            return std::to_string(processingTime.dbl()) + " s";
        default:
            return LayeredProtocolBase::resolveDirective(directive);
    }
}
```

3. Update MacRelayUnitBase.ned to document the directives:

```ned
// MacRelayUnitBase is... (existing documentation)
// ...
//
// The format string can contain the following directives:
//  - %p number of processed frames
//  - %d number of discarded frames
//  - %t frame processing time
//
simple MacRelayUnitBase
{
    parameters:
        string displayStringTextFormat = default("proc: %p\ndisc: %d\ntime: %t");
        // other parameters...
}
```

Follow this pattern for all the classes in the list.
