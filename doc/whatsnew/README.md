# Release notes of the version in development

Each release note of the version in development is a file of its own in this folder. At the release, `doc/project/enforcement/whatsnew.py release` writes the files into `WHATSNEW` as numbered lists and removes them. Two pull requests that add two release notes therefore add two different files, and they cannot conflict. [RR-WHATSNEW](../project/rule/release.md#rr-whatsnew) says what a release note says.

## Add a release note

1. Choose the folder of the list that the release note goes into:
   - `incompatible/`: "Notable backward incompatible changes".
   - `compatible/`: "Notable backward compatible changes".
   - `fixes/`: "Notable bug fixes".
2. Name the file `<area>-<topic>.txt`. The area is the area of the commit subject in lower case, for example `tcp`, `ieee80211`, `ipv6`, `queueing` or `radio`. The topic is a few words of the title. Use only lower-case letters, digits and hyphens. The release puts the entries of a list in the order of their file names, so the entries of one area stand together.
3. Write the title on line 1, without a number. Leave line 2 empty. Then write the text: each paragraph on one line, with one empty line between paragraphs. Indent a table or a configuration line by two spaces.
4. Put the file in the commit that makes the change ([TR-SHIP-WITH](../project/rule/testing.md#tr-ship-with)). To change a release note, change its file.

An example, `incompatible/tcp-algorithm-submodule.txt`:

```
The algorithm of a TCP connection is its submodule

Each TCP connection now creates its congestion control algorithm as a submodule.

The parameters that only one algorithm reads moved from Tcp to the type of that algorithm:

  **.tcp.cubicBeta              -> **.tcp.conn-*.algorithm.beta
```

Do not add an entry to `WHATSNEW` itself. Its section of the version in development has only a sentence that points to this folder.

## Check and read the release notes

From the repository root:

```sh
doc/project/enforcement/whatsnew.py check     # the folder, the name and the format of each file
doc/project/enforcement/whatsnew.py preview   # the section as the release will show it
```

## Make a release

Run `whatsnew.py release` with the heading of the release and the heading of the next version in development, then write the introduction of the release in `WHATSNEW`:

```sh
doc/project/enforcement/whatsnew.py release "INET-4.8 (March 2027) — feature release" --next "INET-4.8.x (in development)"
git add -A WHATSNEW doc/whatsnew
```

Text in the section of the version in development other than the sentence that points here becomes the introduction of the release.
