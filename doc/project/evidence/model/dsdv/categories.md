# DSDV check categories

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks](../../protocol/dsdv/checks.md)

| Checks | Category | Why |
| --- | --- | --- |
| choice, arithmetic, encoding | unit | One-message transformations and independent byte oracle |
| graph | unit | Controlled distributed route transitions and independent graph oracle |
| settling, deadlines, incremental, payload | unit and module | Pure scheduling decisions plus real kernel callback ordering |
| expiry, correction, recovery, lifecycle | module | Timers, socket terminal paths and retained state |
| forwarding, periodic | protocol | Real control exchange and IPv4 packet traversal |
| repeatability, observer neutrality | fingerprint | Whole-trajectory comparison, supplementary to correctness checks |
