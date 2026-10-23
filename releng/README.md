
## Release checklist

- check if all fixed issues are correctly assigned to the milestone for this version 
- modify version number in common/INETDefs.h
- run tests
- If new functionality was added, `tests/fingerprint/SelfDoc.json` must be updated by running `tests/fingerprint/fingerprinttest_selfdoc`. The `SelfDoc.json` file is part of the repo so it should be checked in.
- write the release notes into WHATSNEW and the migration guide: run `doc/project/enforcement/whatsnew.py release "<heading>" --next "<next heading>"`, then write the introduction of the new section (see `doc/whatsnew/README.md`)
- tag the git repo with a vA.B.C tag
- run `releng/makedist`
- rename the created archive (remove hash from the filename)
- push the vA.B.x branch and the vA.B.C tag to the remote
- create release from the above tag on github (add release notes based on whatsnew and upload the archive)
- update models.omnetpp.org and modify the latest release version for inet.xml (for older omnetpp versions too) -> PUSH
- update inet.omnetpp.org: add the version to `doc/src/versions.toml` on master, update `doc/src/site/Download.rst` and add a news post there, then move the `website` branch to master and push it (see doc/src/README.md)
- update `opp_env` database. add new version to inet.py -> release a new version of opp_env by tagging it with a new version
- rebuild any INET based docker images (github.com:omnetpp/dockerfiles.git)
- Write an announcement email to the mailing list and on https://forum.omnetpp.org in the announcement section.
- Major/Minor releases (x.y) should be announced on the main omnetpp.org site, too.
