:orphan:

.. _dg:cha:logging:

Logging Guidelines
==================

This chapter describes how to write and organize log statements in the
INET framework.

General Guidelines
------------------

Log output should be valid English sentences starting with an uppercase
letter and ending with correct punctuation. Log output that spans
multiple lines should use indentation where it isn’t immediately obvious
that the lines are related to each other. Dynamic content should be
marked with single quotes. Key value pairs should be labeled and
separated by ’=’. Enumerated values should be properly separated with
spaces.

Target Audience
---------------

The people who read the log output can be divided based on the knowledge
they have regarding the protocol specification. They may know any of the
following:

-  public interface of the protocol

-  internal operation of the protocol

-  actual implementation of the protocol

Log Levels
----------

This section describes when to use the various log levels provided by
OMNeT++. The rules presented here extend the rules provided in the
OMNeT++ documentation.

Fatal log level
~~~~~~~~~~~~~~~

Target for people (not necessarily programmers) who know the public
interface. Don’t report programming errors, use C++ exceptions for this
purpose. Report protocol specific unrecoverable fatal error situations.
Use rarely if at all.

Error log level
~~~~~~~~~~~~~~~

Target for people (not necessarily programmers) who know the public
interface. Don’t report programming errors, use C++ exceptions for this
purpose. Report protocol specific recoverable error situations.

Warn log level
~~~~~~~~~~~~~~

Target for people (not necessarily programmers) who know the public
interface. Report protocol specific exceptional situations. Don’t report
things that occur too often such as collisions on a radio channel.

Info log level
~~~~~~~~~~~~~~

Target for people (not necessarily programmers) who know the public
interface. Think about the module as a closed (black) box. Report
protocol specific public input, output, state changes and decisions.

Detail log level
~~~~~~~~~~~~~~~~

Target for users (not necessarily programmers) who know the internals.
Think about the module as an open (white) box. Report protocol specific
internal state changes and decisions. Report scheduling and processing
of protocol specific timers.

Debug log level
~~~~~~~~~~~~~~~

Target for developers/maintainers who know the actual implementation.
Report implementation specific state changes and decisions. Report
important internal variable and data structure states and changes.
Report current states and transitions of state machines.

Trace log level
~~~~~~~~~~~~~~~

Target for developers/maintainers who know the actual implementation.
Report the execution of initialize stages, operation stages. Report
entering/leaving functions, loops, code blocks, conditional branches,
and recursions.

Log Categories
--------------

-  *test*: report output that is checked for in automated tests

-  *time*: report performance related data (e.g. measured wall clock
   time)

-  *parameter*: report actual parameter values picked up during
   initialization

-  *default (empty)*: report any other information using the default
   category
