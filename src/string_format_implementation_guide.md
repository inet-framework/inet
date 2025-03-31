# Update Your C++ Classes with StringFormat Support in INET Framework

To enhance your C++ classes in the INET framework with display string formatting capabilities, please implement the following changes for each class you wish to update:

## 1. Add StringFormat::IDirectiveResolver as a Base Class

Modify your class declaration to inherit from StringFormat::IDirectiveResolver:

```cpp
#include "inet/common/StringFormat.h"

class YourClass : public BaseClass, public StringFormat::IDirectiveResolver
{
    // ...
};
```

## 2. Implement the Required Interface Method

The implementation of the resolveDirective method required by the IDirectiveResolver interface should already exist in the code.

## 3. Add the refreshDisplay() Method

Add the refreshDisplay() method to update the display string dynamically:

```cpp
virtual void refreshDisplay() const override
{
    std::string text = StringFormat::formatString(par("displayStringTextFormat"), this);
    getDisplayString().setTagArg("t", 0, text.c_str());
}
```

## 4. Add Parameter Declaration

In your NED file, add the displayStringTextFormat parameter:

```
parameters:
    string displayStringTextFormat = default("%n"); // Format string with directives
```

## Example

Here's a complete example of the changes for a class called `Foo`:

```cpp
#include "inet/common/StringFormat.h"

class Foo : public BaseClass, public StringFormat::IDirectiveResolver
{
public:
    // Existing methods...
    
    virtual std::string resolveDirective(char directive) const override;
    virtual void refreshDisplay() const override;
};

std::string Foo::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return getFullName();
        // Add more directives as needed
        default:
            return "";
    }
}

void Foo::refreshDisplay() const
{
    std::string text = StringFormat::formatString(par("displayStringTextFormat"), this);
    getDisplayString().setTagArg("t", 0, text.c_str());
}
```

This implementation will allow your class to display formatted text in the simulation GUI based on the specified format string parameter.
