# DSDV protocol tests

The [protocol-test guide](../../../doc/project/evidence/model/dsdv/protocol-tests.md) explains both scenarios, their topologies, packet paths, assertions, timing windows, limits and run commands.

| Test | Purpose |
| --- | --- |
| [DsdvForwarding.test](DsdvForwarding.test) | Check DSDV-owned routes and real IPv4 forwarding, with disabled and isolated controls. |
| [DsdvMobile.test](DsdvMobile.test) | Check delivery before a relay leaves, silence during a partition and delivery after it returns. |
