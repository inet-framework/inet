# The standard texts move to the sibling `standards` project

**Status:** done. Started and finished 2026-09-24 on `topic/standards-tests-wave0` (not landed yet),
worktree `/home/levy/workspace/inet-standards-tests-wave0`.

The user added a `standards` project to the workspace, `/home/levy/workspace/standards`, beside
the INET checkouts. It holds the IEEE PDFs under `IEEE/<family>/`, `AS/AS6802.pdf`, and 17 RFCs
as `.docx` under `RFC/`. It is not a git repository. The user's instructions of 2026-09-24:

- Use `standards` for every IEEE, RFC and other document; download what is missing there.
- Remove every RFC copy from the INET tree.
- A reference from INET to a document may assume that `standards` is a sibling folder of the
  INET checkout.
- Format: the rfc-editor `.txt` beside the `.docx` for every RFC (the user's choice).

Facts that shaped the plan:

- The 17 `.docx` files are Google Docs exports of the `.txt` files that `inet-master` cached:
  paragraph N of the `.docx` is line N of the `.txt`, checked for RFC 791, 1122, 9293 and 826.
  So every `rfcNNNN.txt:line` citation stays valid.
- The evidence tree has 1,224 line citations of the form `rfcNNNN.txt:line`, 69 links to a
  cached text and 50 links to a text folder. No CI job runs the link gate; the local gate
  resolves the links against the sibling folder.
- The IEEE texts could never go into git (personal licence stamp). The sibling project solves
  that as well: it is local and not a repository.

## Steps

1. [x] Download the `.txt` of the 61 RFCs of the evidence tree into `standards/RFC/`, and check
   that each one equals the copy in the INET tree. Done: all 61 are identical. `standards/RFC/`
   now holds 61 `.txt` and the 17 `.docx` of the user.
2. [x] The guide: step 1, the table of steps, the naming paragraph and "Where everything lives"
   say where the texts live now. Done in `5c2c632978`.
3. [x] The evidence tree links to `standards/RFC/rfcNNNN.txt`, and the 61 copies leave the tree.
   Done: 128 links in 34 files point into `../standards/RFC/`, and the 17 catalogs link their
   source text there. `evidence/standard/` keeps only the 17 folders with a `catalog.md`.
4. [x] Gates, then move this plan to `plan/done/`. All four gates pass on `master..HEAD`: 225
   files and 0 broken links, seals in step, commits and classification clean.

## Decisions and facts found on the way

- The links had three forms: a link to the text file, a link to the text folder with a slash,
  and a link to the folder without a slash. One rewrite covered all three.
- "Cached" in the prose became "downloaded": the texts are still downloaded from the RFC Editor,
  but INET no longer keeps a copy. "The cached mapping" of ARP stays, because it names the ARP
  cache.
- The link gate resolves a link outside the INET tree: with `rfc2453.txt` hidden, the gate
  reported two missing files in `rip/standards.md`. So the local gate also checks that the
  `standards` project is complete.
- The two plans in `plan/done/` that name `standard/rfcNNNN/rfcNNNN.txt` are history and stay
  as they are.
