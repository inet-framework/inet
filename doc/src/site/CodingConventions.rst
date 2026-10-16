==================
Coding Conventions
==================

Code Format
-----------

We know that indentation style is a personal matter but the codebase needs to look consistent, so please adhere to the conventions below.

- Bracket style: largely Stroustrup (Egyptian braces), except for classes, functions, and the like where we prefer to place the opening braces on a new line
- 4-space indentation, NO TAB CHARACTERS, no trailing spaces
- Do not align variable names in declaration blocks or multi-line function declarations. They are hard to maintain,
  and look awkward when very short and very long type names are mixed (e.g. ``int`` vs ``const std::vector<SomeVeryLongTypeName>&``).
- Spaces around ``if``, ``for``, ``while``, ``return``, and other keywords
- Spaces around ``=``, ``==``, ``&&``, ``||``, and other operators
- No parens around the value in ``return`` statements
- No multi-line blanks within class/functions
- Non-documentation comments: use ``//`` not ``/**/``

The OMNeT++ IDE provides you pre-configured code styles in *Window -> Preferences -> C++ -> Code Style*.
Choose *OMNeT++* as your code style. Pressing Ctrl+Shift+F will reformat your code to match this style.

Naming Convention
-----------------

Rules for variable names, function names, class names, and other identifiers.

- Use camel case
- Variables and functions start with lowercase
- Types start with uppercase
- Constants and enum values are capitalized, with underscore as word separator
- Don't abbreviate (too much)
- Function names start with a verb
- Functions returning a bool preferably start with ``is`` or ``has``

Commenting
----------

- Add copyright headers at the top of each source file. Use ``//`` comments for consistency with NED and MSG files.
- Generally, use ``//`` comments in the code (except for Doxygen comments)
- No decorations such as ASCII art or separator lines like ``========``, ``--------`` or ``########``
- For documentation comments (Doxygen and NED), see `Documentation Guidelines <DocumentationGuidelines.html>`__

Class Layout
------------

Consistent layout of class declarations throughout the project helps maintainers to remain sane.

- Data members first, protected/private methods next, public methods last
- Sort data members into *configuration* (or *parameters*), *state*, and *statistics* sections
- Methods that are not public API should be private or protected
- Use Doxygen comments (``/** */``) for public API only
- Comment data members with ``//`` comments
- Non-public methods can be left undocumented, or use ``//`` comments where necessary

Functions
---------

Misnamed and "kitchen-sink" functions are unfortunately quite common and make the code hard to follow and reason about. Here's what you can do:

- Functions should be focused (i.e. do one thing only), and the name should reflect the purpose
- Try to minimize side effects (modify class members and other non-local state as little as possible)
- If you cannot find an accurate name for it, it is a strong indicator that the function should not exist in that form (e.g. needs to be split up or the code organized in a different way)
- If it has a side effect, the name should reflect that (e.g. *get* methods must not modify state)
- Local variables should be declared close to their use, in the smallest scope possible (i.e. not C-style, at the top of the function)
- Commenting: we prefer breaking up the function body into logical blocks and adding a ``//`` comment above each block to document its intended *purpose*

C++ Style
---------

Most users of INET are not C++ scholars. We think the codebase should be written in a style and in a subset of C++ that makes it accessible to our users.

- Emphasis on readability, i.e. refrain from clever tricks and relatively obscure C++ features
- Use of the Boost library and other excessively templated code is not allowed

C++11 Features
--------------

From INET 3.0, the following `C++11 features <http://www.stroustrup.com/C++11FAQ.html>`__ will be allowed (and recommended):

- Non-static member initializers (preferred to constructors)
- ``auto`` for iterators
- ``nullptr``
- ``static_assert``
- Range-based ``for`` loops
- ``final`` method keyword
- ``override`` method keyword
- Delegating constructors
