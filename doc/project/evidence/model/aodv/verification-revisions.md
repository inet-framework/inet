# Verification revision mapping

> **Kind:** report · **Status:** snapshot 2026-11-09 · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md)

Historic paired tests name the source snapshots actually executed. Before submission, release-fragment headings and one migration underline were corrected in their owning unpublished commits. The three feature trailers were also corrected to the canonical `behavior.add` spelling, preserving their complete trees. The AODV source subtree is byte-identical at every corresponding revision; the mapping below was checked with `git rev-parse <revision>:src/inet/routing/aodv`. No source behavior or existing test expectation changed in these metadata corrections.

| Tested revision | Submitted revision | AODV source tree |
| --- | --- | --- |
| `185bc12f6b80ebc3d61d2cfa271dceda7d34f15d` | `185bc12f6b80ebc3d61d2cfa271dceda7d34f15d` | `0f2f3b90f4751534e8658cd2af2aacce6abe2972` |
| `67bb7ba7b5cc7768c4c0efef08c990217bf475f2` | `67bb7ba7b5cc7768c4c0efef08c990217bf475f2` | `5d9a9a632488be44ed84a3ca86023b8c3636d7c4` |
| `be0ba1067d530b6296a67b3aaa7d080f396250d8` | `be0ba1067d530b6296a67b3aaa7d080f396250d8` | `0244022aa958decc01702b71e36e73337a4d89e0` |
| `ec0c7b08be84526275d0f350703e4a670b51a05f` | `0cbb58d859232b253c7f87d55e64cf938e9ba6fd` | `597b359e2eb20cf496b7a1245b7bb72691750df6` |
| `f8baeb3233d2055d2020cab40c544cc67d5f5c0c` | `fe4590521dd4f871927f23b43b77cf8bdfd753e5` | `92ef99b9d7ae72211b9d784ed079dd1b163cd13c` |
| `335030555e4a8cd90e900df9964309e38b0c3bee` | `b3d400579f42a6d4ab1d766eeeb86ddaec1b4e8f` | `14ba8ff4f53ea28709d5837dfc396fd347ad4550` |
| `5e143cc73e094e9b1c458c079b84417254aaa7f4` | `da096729aff971619c882c37ba5624bdbf0d3074` | `a419abe257881a3d8c8fadc0fda58e457b74da18` |
| `381667c9d0c30755b8fd6c81b955bf88ab2de08b` | `f86932ac41ccfd997fcdbd2fe268f051d8d0fd65` | `eaca458ec3e594e1e7023f554b8f9c8d41e1d75c` |
| `05c0a2c2f8eef5cbbe6d2b65f13e849c58ab597f` | `d9bc3108a2cd02317ff001eb787ab7838fedaa48` | `dfc14c186937b420690715a13088206adf8c48d9` |
| `14013e9c8a2ad634da31041f5220407fd67f286f` | `753d2e6e4d052e4d2dbe447762992d9001bd2d2e` | `11ee182df4b943540994318efd14d9f36b2f9266` |
| `9fd380a795d7f490dea5b9d9ae6ccfebe7daf8cf` | `b3d6b7fe09b021af893fa2cbab32e9078ec6c614` | `75ade26b06159e4540af36c3fb4ae5c23dfdc0a8` |
| `497d3cb7e5ad2585a3d098fdc444e9b5b9a08a86` | `db089a8bc77380daa3922615a8b539e3d6acbf79` | `a5e78e450ffff2d4cab3016080cbab3ce64d65a2` |
| `53f2cd4d4e593113ae4e008ef788f0284cfcf77c` | `3d06973211fa21627d9b50322439fd9f9e78425e` | `f2bd62e7c954872866bfd6b0f9ea0c9e47138c51` |

The final IPv6 comparison parent `edecd5e6ce2c33720e980ca8a3dcd3943f159d58`
has the same complete tree as `3d06973211fa21627d9b50322439fd9f9e78425e`.
