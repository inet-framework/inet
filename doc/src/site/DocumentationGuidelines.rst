========================
Documentation Guidelines
========================

We maintain documentation in the following forms:

- **INET Framework User's Guide** is meant to provide information for users who want to assemble new models from pre-existing components. It provides an overview of the components and architecture of the framework.
- **INET Framework Developer's Guide** is meant to provide information for the developers who intend to extend INET with new components, written in C++.
- **INET Reference**, built from NED and C++ source, provides cross-linked, browsable reference information about the modules and classes of the implementation.

The following auxiliary files are managed by the core INET maintainers:

- **ChangeLogs**. Every source folder contains a ``ChangeLog`` file in a fixed format. ChangeLogs exist so that users can get a high-level overview of the history of that component. It is usually derived from the git log by heavy editing for conciseness and clarity, e.g. by combining a series of related commit messages into a single change summary. (The git log is often too detailed and contains too much "noise" for end-user consumption.)
- **WHATSNEW** (in the project root directory) is updated before each release and contains a summary of changes in the whole framework.
- **AUTHORS** (in the project root directory) records the names of those who contributed to the project. The `Contributors </Contributors.html>`__ page should always contain a mirror of this file.

INET User's Guide and Developer's Guide
---------------------------------------

Technically:

- Sources are in the repository under `doc/src/users-guide <https://github.com/inet-framework/inet/tree/master/doc/src/users-guide>`__ and `doc/src/developers-guide <https://github.com/inet-framework/inet/tree/master/doc/src/developers-guide>`__, in RestructuredText.
- PDF and/or HTML are built by the release manager for each release.

Content and level of detail:

- The purpose of the User's Guide is to provide an overview of INET and provide high-level information about every implemented protocol model or component, potentially including practical advice on how to use them or combine them with other models.
- The purpose of the Developer's Guide is to help users and contributors who want to understand the internal architecture and mechanisms of INET and wish to extend it with new components, written in C++.
- Must not duplicate information in protocol standards, in the INET Reference, etc.

If you wish to contribute, please check the **Authors Guide** chapters in both documents to see what kind of content is expected before writing anything.

INET Reference
--------------

We use Doxygen and the OMNeT++ IDE (*Project -> Generate NED Documentation*) to generate hyperlinked documentation from source code. The following rules help us achieve that the generated documentation is useful and looks good.

NED documentation:

- We use explicit hyperlinks, i.e. the tilde notation is necessary to create a hyperlink to other modules. Examples: ``the ~TCP module`` (hyperlinks the TCP module), but: ``the TCP protocol`` (no hyperlink).
- NED comments document the external interface: purpose, standards compliance, parametrization, gates, etc.
- Provide a reference to the protocol specification and also describe what parts of the specification are implemented and what parts are missing.

C++ documentation:

- Of the many syntaxes Doxygen allows, use the ``//** */`` one. ``@brief`` lines are usually not needed.
- Class comments should be about the *implementation* of the given protocol (as NED already documents the purpose and external interface). When the implementation is straightforward, *"Please see the NED file for details"* is a perfectly fine comment.
- Only public methods should be documented. Don't use ``/** */`` comments for private data members, private methods, etc. (We recommend ``//`` comments instead.)
