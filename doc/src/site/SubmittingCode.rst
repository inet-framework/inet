===============
Submitting Code
===============

When you have changes that you'd like to be included in the INET Framework, here is the process:

1. Prepare your changes (see checklist below)
2. Submit your changes. You can send a pull request on GitHub.
3. Your changes will be reviewed (see details below)
4. When the issues found during the review have been resolved, your changes will be merged by the core team

Checklist for the patch or pull request
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

General rules:

- Should apply on top of the ``master`` branch.
  If the target branch has moved on in the meantime, rebase your topic branch on top of it
- Adhere to the coding convention
- Have separate commits for

  - formatting and documentation changes
  - refactoring operations
  - bug fixes
  - new code

- Existing tests (esp. fingerprint tests) should pass
- If you added new functionality, create tests for it

Writing good commit messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A good commit message helps future INET maintainers find out why a particular change
was made to the code, and determine its correctness.

- The first line should be a summary. It should begin with a prefix, see the next item.
- **Begin the summary line with the affected protocol or component name,
  and a colon** (e.g. ``tcp:``). Such a prefix is immensely helpful for people browsing
  the project history. In some cases, using a topic word instead of the component
  name may also be justifiable (e.g. ``coverity:``).
- If the commit is a bug fix, add ``fix:`` and the bug number (if available).
- The second line should be blank.
- After that, add a potentially multi-line explanation of why the change was made.
  Include any background information that may be helpful for later maintainers,
  e.g. pointers to relevant sections of standards documents or textbooks,
  names of tests or examples that the change fixes, etc.

Code review
~~~~~~~~~~~

There will be two reviews: semantic and technical review.

The semantic review will be performed by `advisors <ComponentAdvisors.html>`__
of the affected component and will include:

- Assessing that the given feature is worthwhile to include in the INET codebase
- Checking the correctness and standards compliance of the implementation

The technical review will be done by the core team:

- Code must adhere to the coding conventions and follow the INET architecture
- Documentation should exist (or has been updated)
- Tests should exist (or have been updated)
- Small changes like updating ``ChangeLog`` files, ``AUTHORS`` or ``.oppfeatures``
  and adding smoke/fingerprint tests can be left to the core team
