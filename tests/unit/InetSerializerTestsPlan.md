# Terv: INET Serializer-tesztek

## Kontextus

Az INET-ben ~80 serializer-osztály van (263 `Register_Serializer(...)`), de a
tesztlefedettségük hézagos és szétaprózott. A cél: több, egymást kiegészítő
módszerrel tesztelni a serializereket, **elsőként pcap-készlet alapú
round-trippel** (pcap beolvasás → deserialize → serialize → egyezés az
eredetivel), és **kimutatni, mely regisztrált serializer marad teszteletlen**.

A munka a `topic/bz/serializertest` branchen folyik (inet repo:
`/home/zoli/Projects/OMNET/inet`). Már létezik részleges infrastruktúra, amire
építünk, illetve amit kiváltunk.

### Kulcsfelismerés
A round-trip **önmagában csak önkonzisztenciát bizonyít** (serialize∘deserialize
megőrzi a bájtokat), a wire-formátum helyességét nem. Ezért több módszer kell, és
a hibás round-trip-eltéréseket külső orákulummal (tshark) érdemes szűrni.

## Meglévő, újrahasználandó elemek

- `PcapReader::readPacket()` → `Packet*`, kezeli a byte-swapet/fejléceket
  (`src/inet/common/packet/recorder/PcapReader.h`). **Hiányzik** belőle a
  `network`/linktype getter — pótolni kell.
- `PacketDissector` + `PacketDissector::ChunkBuilder`
  (`src/inet/common/packet/dissector/PacketDissector.h`) — automatikusan
  végigjárja a regisztrált protokoll-disszektorokat és típusos chunkokká bont.
  **Ez váltja ki** a jelenlegi kézi `deserialize()` if/else láncot.
- `Packet::peekAllAsBytes()` → nyers bájtok; `BytesChunk` bájtszintű
  összehasonlításhoz.
- `Chunk` korrektség-flagek (`src/inet/common/packet/chunk/Chunk.h:257`):
  `isCorrect()`, `isComplete()`, `isProperlyRepresented()`.
- `MemoryInputStream::isReadBeyondEnd()` — túlolvasás jelzése (nem dob kivételt).
- `ChunkSerializerRegistry` (`src/inet/common/packet/serializer/ChunkSerializerRegistry.h`)
  — `typeid → serializer` map, **jelenleg nem bejárható** (pótolni kell).

### Kiváltandó / átírandó
- `tests/unit/serializer.test` — jelenleg saját pcap-parser + ~250 soros kézi
  `deserialize()` lánc, csak `LINKTYPE_ETHERNET`, 5 pcap.
- `tests/unit/IPv6_serializers.test`, `Bgp_open_capability_serializer.test`,
  `MLD_serializer.test`, `PIM_serializer.test`, `OSPFv3_serializer.test` —
  mindegyik külön lemásolja ugyanazt a `roundTrip<T>()` sablont.

## Terv

### 0. munkacsomag — Közös helper (`tests/unit/lib/`)
Új `SerializerTestLib.h/.cc` a meglévő `tests/unit/lib`-ben (rendes C++ lib,
`Makefile` már épít `libtest_dbg.so`-t). Tartalma:
- generikus `roundTrip<T>()` sablon (a most 5 helyre másolt kód kiváltása),
- a pcap round-trip motor (1. csomag),
- „exercised" (érintett) `typeid`-gyűjtő a lefedettség-riporthoz (2. csomag).

A meglévő `.test`-ek `%includes`-szal behúzzák a headert, és átállnak a közös
`roundTrip`-re.

### 0b. munkacsomag — Meglévő per-protokoll tesztek: megtartás + megerősítés
A `IPv6_serializers.test`, `Bgp_open_capability_serializer.test`,
`MLD_serializer.test`, `PIM_serializer.test`, `OSPFv3_serializer.test`
**már master-en vannak**, és mindegyik egy **konkrét serializer-funkcióval együtt**
született (RFC-specifikus opció-/altípus-formák: RFC 4760, RFC 4601, MIPv6 stb.)
→ funkció-/RFC-kötött regressziós védők.
- **Komplementerek** a pcap-teszttel, nem redundánsak: a pcap valós ground-truth de
  csak a capture-ben lévő eseteket éri el; ezek él-eseteket/altípusokat és
  **capture-nehéz protokollokat** fednek, mezőnkénti hibalokalizációval. (A megtartás
  + átalakítás lépéseit lásd a lenti „Explicit, lépcsős sors"-ban.)
- **Golden-hex fogalma (a megerősítéshez):** a round-trip csak
  *önkonzisztenciát* bizonyít (serialize/deserialize egymás inverze) — attól a
  wire-bájtok még lehetnek „önkonzisztensen rosszak". A golden-hex egy fix,
  külső forrásból (RFC-példa / valós capture / **tshark**, WP3) vett ismert-helyes
  bájtsor, amihez a `serialize(objektum)` kimenetét hasonlítjuk → *wire-helyesség*.
  Csak az RFC-kritikus formákra (kézzel kurált, ezért szűken).
- **Bővítés csak célzottan** (lásd 2. csomag riportja): új per-protokoll teszt ott,
  ahol a serializer capture-nehéz ÉS nincs lefedve; nem blanket-bővítés.

**Explicit, lépcsős sors (`IPv6_serializers.test` és társai):**
1. **Maradnak** (master-en lévő, RFC-/funkció-kötött regressziós pinek) — nem törlődnek.
2. **WP0:** átállás a közös `roundTrip<T>`-re → csak a duplikált helper tűnik el,
   **viselkedés változatlan** (kis, biztonságos, master-tracked commit).
3. **WP3 (tshark-kal):** a bennük lévő formákhoz golden-hex / tshark-ellenőrzés →
   önkonzisztencia helyett wire-helyesség.
4. **WP6:** a *lapos* mező:érték eseteik átvihetők az adatvezérelt táblázatba a
   generikus futtató alá; kézzel marad a beágyazott/TLV/opció-konstrukció (hook) +
   a kurált RFC-szcenáriók.
5. **Nem kötelező** a meglévő zöld master-teszteket táblázatra átírni — az **újakat**
   írjuk így, a meglévőket elég **deduplikálni** (alacsony churn a master-en).

**Irreducibilis maradék — mit tesztelnek, amit a WP6/WP7 sem tud.** A mechanikus
részt (építés/serialize/deserialize/ciklus) a WP6/WP7 kiváltja; ami marad, ahhoz
emberi tudás kell (a protokoll + az INET objektummodell):
- **A) INET objektummodell-invariánsok** — amit a tshark (wire-only) és a
  bájt-round-trip (önkonzisztencia) sem lát: deserialize utáni **helyes altípus**
  (`dynamic_cast<BgpCapabilityMultiprotocol>`), `L3Address::IPv6` típus-tag, enum/flag
  szemantika (OSPFv3 `v6Bit`, PIM `S`).
- **B) Kurált regressziós szcenárió** — az egykor eltört pontos mező-/opció-kombináció
  (RFC 4760/4601/…); a korpusz/random driver nem biztos, hogy eltalálja.
- **C) Orákulum nélküli protokoll** — se capture, se tshark → csak kézzel, RFC-ből
  levezetett golden hex ad wire-igazságot.
- **D) Serialize-helyesség él-értékre** — amit a capture nem tartalmaz és a WP7 random
  driver csak önkonzisztensen (nem *helyesen*) ellenőriz; kézi teszt a pontos elvárt
  bájtokat állítja.

### 1. munkacsomag — PCAP-készlet alapú round-trip (első prioritás)
- `PcapReader`-re épül (a kézi pcap-parser eldobása).
- **`src` módosítás:** `PcapReader::getNetworkType()` getter hozzáadása.
- linktype→`Protocol*` leképző tábla (ETHERNET, IPv4/IPv6 raw, PPP, 802.11,
  LINUX_SLL…).
- A kézi `deserialize()` láncot **`PacketDissector` + `ChunkBuilder`** váltja ki
  → egy kódból minden regisztrált protokollra megy.
- Folyamat rekordonként: read → dissect a linktype protokolljával → `ChunkBuilder`
  visszaépíti a tartalmat egy új `Packet`-be → `peekAllAsBytes()` → bájt-összevetés
  az eredetivel; eltérésnél az **első eltérő offset** + környezet kiírása.
- Kezelendő: FCS, snaplen, csonkolt rekordok, padding, checksum-normalizálás;
  per-fájl „ismert-eltérés" jelölés a jóindulatú normalizálásokra.
- Korpusz: valós capture-ök `tests/unit/pcap/<protokoll>/` alá.

### 2. munkacsomag — „Teszteletlen serializerek" riport
- **`src` módosítás:** `ChunkSerializerRegistry`-be publikus felsoroló
  (pl. `std::vector<std::type_index> getRegisteredTypes() const`).
- Exercised-halmaz: a dissector `visitChunk`-ja minden érintett chunk `typeid`-jét
  naplózza a pcap- és field-tesztek futása közben.
- Riport a futás végén: `regisztrált − érintett = teszteletlen` lista →
  gap-analízis, ami vezérli a korpusz-bővítést.

### 3. munkacsomag — tshark mint orákulum (telepítve: TShark 4.6.4)
`tshark` **elérhető** (4.6.4) → aktív; ha máshol hiányzik, a keret skippel.
- **PDML** (`tshark -T pdml`): a teljesen disszektált csomagfa XML-ben; mezőnként
  `<field name="ip.src" pos="26" size="4" show="10.0.0.1" value="0a000001"/>` —
  kanonikus mezőnév + nyers hex + dekódolt érték + bájt-offszet/hossz. Alternatíva
  `-T json` (scriptből könnyebb). Ajánlás: `-T json`, PDML ha a `pos/size` kell.
- **(a) Round-trip eltérés szűrése:** ha a visszaírt bájtok eltérnek, mindkét
  változatot tsharkon átfuttatva, ha a mezőértékek egyeznek → jóindulatú
  normalizálás (nem bug); ha a mezők is eltérnek → valódi bug.
- **(b) Wire-helyesség:** a mi disszekciónk / mezőértékeink a tshark mezőivel
  összevetve megfogható az „önkonzisztensen rossz" kódolás is; a golden-hex
  vektorok is tsharkkal validálhatók/generálhatók.

Megjegyzés: az (a) arbitrációhoz **nem** kell INET-névtábla (mindkét oldal tshark);
a tshark↔INET mezőnév-megfeleltetés csak a WP6 field-compare-jéhez kell — ott is van.

### 4. munkacsomag — Fuzz / robusztusság (két irány)

**4a. Bájt-szintű fuzz (wire → deserialize → [serialize]).** A wire-oldalról
indulva megkerüli a „beírható ≠ serializálható" problémát.
- **Orákulum:** a `deserialize` összeomlás/beragadás nélkül terminál, és **vagy**
  tiszta `cRuntimeError`-t dob, **vagy** olyan chunkot ad, amire
  `isCorrect() && isComplete() && isProperlyRepresented() && !isReadBeyondEnd()`.
  Bájt-round-trip egyenlőséget **csak** ilyenkor ellenőrzünk.
- **Keret-védelem:** fix seed + bukó bemenetek mentése regressziónak; kis snaplen
  (≤1500 B); iteráció-/idő-watchdog a deserialize köré (végtelen ciklus / óriás
  allokáció ellen); célzottan a regisztrált típusra adott bemenet (attribúció).
- `ASSERT`-abort vagy beragadás gyakran **valódi robusztussági bug** (a
  serializernek `cRuntimeError`-t kéne dobnia) → egyesével triage.

**4b. Descriptor-kitöltés (objektum → serialize → deserialize), a fordított
irány.** A `cClassDescriptor::setFieldValueAsString`-gel állítunk mezőket, majd
serializálunk. **Talált kód-tény:** a serializerek jellemzően **nem** dobnak
explicit `cRuntimeError`-t érvénytelen mezőértékre; a tartomány-ellenőrzés a
`MemoryOutputStream` szintjén `ASSERT`-tel történik (`writeUint4` →
`ASSERT(value >> 4 == 0)` majd maszkolás, `MemoryOutputStream.h:219`). Ezért:
- debug buildben tartományon kívüli érték → **`ASSERT`-abort**;
- release buildben → **csendes csonkolás** (maszkolás) → lossy bájtok.

- **A kérdés, amit ez tesztel:** nem kell előre tudnunk a mező wire-tartományát
  (ez volt a korábbi aggály) — a round-trip + az abort megfigyelése **maga a
  felderítő mechanizmus**. Oráculum egy alap-érvényes objektumból egy-egy mezőt
  mutálva:
  - serialize **sikerül + round-trip veszteségmentes** → jó (érték a wire-tartományban);
  - serialize **sikerül, de round-trip lossy** → **valódi bug**: csendes csonkítás
    (a serializer „hazudott"); a legértékesebb találat;
  - serialize **`ASSERT`-abortál** → nincs elegáns validáció; jelölt arra, hogy
    `cRuntimeError`-rá emeljük (hogy release build ne korruptáljon).
- **Gyakorlati kezelés:** debug build (hogy az abort látható legyen, ne csendes
  csonkolás); az abort **izolálása** (per-eset subprocess/fork), hogy egy abort ne
  vigye el a köteget; **kihagyandó** a származtatott/számított mezők (checksum,
  hosszmezők — a serializer úgyis felülírja → várt eltérés) és a readonly /
  `@editable(false)` mezők; per-mező izoláció attribúciós buktatói miatt egy
  egyébként érvényes alapobjektumot mutáljunk egyszerre egy mezőben.

**4c. Hogyan legyen „értelmes" a random bemenet.** Alapelv: a **tiszta random
adat nem a korrektség-orákulumhoz való**. Szét kell választani:
- *Korrektség* (veszteségmentes round-trip) → csak **érvényes/értelmes** bemenet;
- *Robusztusság* (nem szabad elszállnia) → itt jó a tiszta/mutált random, az
  orákulum a „nem crashel + korrektség-flagek".

Az értelmes bemenet előállításának technikái (új `RandomPacketFiller` a
`tests/unit/lib`-ben — jelenleg nincs ilyen helper):
1. **Serializer-gyártotta bájt a deserializernek:** a deserializernek ne random
   bájtot adjunk, hanem amit a serializer egy érvényes objektumból előállított →
   strukturálisan garantáltan érvényes, és él a round-trip orákulum.
2. **Korpusz-vezérelt mutáció (indulj érvényesből, mutálj keveset):** seed a
   pcap-korpuszból (1. csomag) és a konstruált objektumokból; kis mutációk
   (bit-/bájtflip, egy mező tweak). A „majdnem érvényes" mély kódutakat ér el
   (opciók, altípus-diszpécs), nem pattan le az első verzió-/hosszellenőrzésen. A
   pcap-korpusz egyben fuzz-seed korpusz is.
3. **Struktúra-/típus-tudatos descriptor-kitöltés (4b-hez):** a `.msg` metaadat már
   kódolja a wire-korlátokat — `@bit(N)` annotáció (pl. `reservedBit // @bit(1)`,
   `Ipv4Header.msg:184`), enum-típus, C++ típusszélesség; ezek a descriptoron át
   kiolvashatók → **tartományon belüli** értékek generálhatók. Ez oldja fel a
   „beírható ≠ serializálható" gondot; ahol nincs metaadat, típusszélesség vagy
   határérték-próba a fallback.
4. **Repair/normalizáló menet:** a *szemantikus* mezők randomizálása után a
   *származtatott* mezőket újraszámoljuk az önkonzisztenciáért (hosszmező = valódi
   hossz, checksum `FCS_COMPUTED`, IHL, next-header diszkriminátor = a tényleges
   következő chunk). A diszkriminátor/típus-mezőt **először** állítjuk be, aztán
   csak az adott altípushoz érvényes mezőket töltjük.
5. **Határértékek uniform random helyett:** az „értelmes" jellemzően
   határérték/ekvivalencia-osztály: {0, 1, max, max−1, max+1 (túlcsordulás a
   validáció tesztelésére}, enum-határok — több bugot talál és reprodukálható.
6. **Érvényes fejléc + random farok (4a-hoz):** egy érvényesen serializált fejléc
   után csak a payload/opció-régiót randomizáljuk, fokozatosan növelve.

### 5. munkacsomag — Fingerprint-alapú rendszerszintű ellenőrzés (`D` / PACKET_DATA)
Rendszerszintű, „széles" ellenőrzés a meglévő fingerprint-teszteken keresztül.
- Az INET saját fingerprint-hozzávalói: `INET_FINGERPRINT_INGREDIENTS = "~UNID"`
  (`src/inet/common/FingerprintCalculator.h:15`). A **`D` = PACKET_DATA** hozzávaló
  minden csomagra `peekAllAsBytes()`-t hív
  (`src/inet/common/FingerprintCalculator.cc:60`) → a szimuláció **minden csomagját
  végigserializálja**.
- **Módszer:** a fingerprint-teszt `~tND` hozzávalókkal futtatva ugyanazt a `tplx`
  fingerprintet kell adja, mint a sima `tplx` számítás. Ha eltér (vagy a `D` miatt a
  serialize kivételt/`ASSERT`-et dob), akkor serializer-hiba van.
- **Erősség:** ingyenes, széles lefedettség — minden meglévő szcenárió összes
  ténylegesen használt serializerét megmozgatja.
- **Gyengeség (a felhasználó jelezte):** nagyon lassú (teljes szimulációkat futtat),
  és **nem lokalizálja a hibát** (melyik csomag / melyik serializer). Csak azokat a
  serializereket éri el, amik a meglévő szcenáriókban előfordulnak → a 2. csomag
  „teszteletlen serializerek" riportja mutatja meg, mi marad ki.
- **Fontos megkötés:** a `D`-s és a `tplx`-only futást **ugyanazon a lokális
  buildon** kell összevetni, nem a tárolt upstream fingerprintekkel — a lokális
  build eleve driftelhet az upstream fingerprintektől (lásd a
  `fingerprint-env-baseline` emléket).
- Jó **regressziós hálóként**: a részletes lokalizációt az 1.–4. csomag adja, ez a
  „nem szivárgott-e be hiba sehol" végső ellenőrzés.

### 6. munkacsomag — Hex-elsődlegű helyességi tesztelés (a fő korrektség-motor)

**Elv: a hex az elsődleges (nem az objektum).** Egyetlen adatból két irányt igazol:
```
hex (wire-igazság)
  ├─ deserialize → objektum → descriptor→JSON-dump  == elvárt JSON   [deserialize helyes]
  └─ objektum → serialize → hex'                     == eredeti hex   [serialize helyes]
```
Mivel a hex→objektum egyezik a JSON-nal ÉS az objektum→hex visszaadja a hexet, a
serialize és deserialize **külön-külön, külső igazsághoz mérve** helyes → megfogja
az „önkonzisztensen rossz" esetet is.

**Kulcselőny — eltűnik a nehéz irány:** a hex-elsődlegű formában **sosem
konstruálunk** JSON-ból (a deserializer építi az objektumfát a bájtból); a JSON
csak **összevetésre** kell, az pedig rekurzív descriptor→JSON dump + diff. Ezért a
lapos-JSON-**konstruálás** korlátai (polimorf / beágyazott / számolt-hossz mezők —
lásd a WP7 lábjegyzetét) itt **nem jelentkeznek**: a dump rekurzívan kezeli a
beágyazottat, a hossz már helyes (valós bájtból jött), az altípus `__type__` mező.

**Serialize-vissza megkötés:** nem-kanonikus wire-formáknál (padding, opció-sorrend,
újraszámolt checksum) a hex' ≠ hex lehet *jogosan* → ugyanaz a known-diff /
normalizálás kell, mint a pcap round-tripnél; a golden vektorokat kanonikusra
választjuk.

**Egyesített pcap+tshark pipeline (a hex-elsődlegű forma automatizálása skálán):**
ez összeköti a WP1 + WP3 + WP6-ot:
- pcap rekord = az elsődleges hex;
- INET deserialize → objektum → JSON-dump;
- `tshark -T json` ugyanazon a bájton = **független** „elvárt JSON" (nem kézzel);
- INET-JSON vs tshark-JSON → szemantikus helyesség; serialize vissza → pcap hex.
- **Mezőnév-megfeleltetés (a fő költség):** (1) per-protokoll névtábla kézzel a
  top-protokollokra (`ip.ttl`↔`timeToLive`, ~10–30 mező/protokoll); (2) név-független
  érték/offszet-illesztés backstopként (PDML `pos`/`size`/`value` ↔ INET bájt-tartomány,
  chunk-granularitáson ingyen, field-szinthez stream-instrumentálás); (3) per-típus
  érték-normalizáló (tshark-hex ↔ INET dekódolt string).
- **Snapshot-figyelmeztetés:** ha az elvárt JSON az INET saját dekódjából jön
  (snapshot), az csak *regressziót* fog, nem a jelenlegi hibát → független igazsághoz
  a JSON kézzel ellenőrzött **vagy** tshark-ból való.

**Vektor-generálás (nem kézzel írjuk a hex-json párokat — generáljuk):**
A generálás és a tesztelés ugyanaz a pipeline. Bemenet egy pcap rekord (vagy hex),
kimenet `(hex, expected_json)`:
```
pcap rekord → hex
   ├─ tshark -T json        → tshark_json   (FÜGGETLEN elvárt érték)
   └─ INET deserialize→dump → inet_json
   egyeznek? ── IGEN → auto-elfogadott vektor {hex, expected_json}
             └─ NEM  → ember elé: INET-bug VAGY névtábla-hézag (a keresett hiba)
```
- A `hex` a WP1 pcap-korpuszból, az `expected_json` a **tsharkból** jön → az emberi
  munka **review/kuráció**, nem szerzés. Az egyezés ingyen legyártja a vektort, az
  eltérés maga a hasznos találat.
- **Két szint:** (1) *regressziós snapshot* — `expected_json` = INET saját dumpja,
  olcsó tömeges lefedés, csak regressziót fog; (2) *helyességi vektor* — tshark /
  kézzel ellenőrzött, független igazság, a fontos protokollokra.
- **Ahol nincs capture/tshark:** **Scapy** mint második független generátor
  (`bytes(pkt)` a hex, scapy-disszekció a mezők); végső esetben INET
  construct→serialize→hex + néhány mező kézi ellenőrzése (snapshot-only).
- A **névtábla is bootstrap-elhető** ugyanebből az érték/offszet-egyezésből
  (javaslat → ember jóváhagyja, nem nulláról ír).
- Egy `tests/unit/lib/gen_hex_vectors` szkript/eszköz gyártja és frissíti a
  `hex_vectors/` tartalmát; canonicalizálás vagy known-diff jelölés a serialize-vissza
  lábhoz.

**Eredmény:** a kurált korrektség-tesztek `hex_vectors/<protokoll>.json` adatként
állnak, egy generikus `.test` futtatja őket — per-protokoll C++ helyett. A meglévő
per-protokoll tesztek (0b) így a kurált maradékká zsugorodnak, nem törlődnek.

### 7. munkacsomag — Generikus random round-trip meghajtó (széles lefedés)
Cél: a WP2 „teszteletlen serializerek" listáját **olcsón, per-protokoll kód nélkül**
lefedni. Orákulum: **önkonzisztencia** (serializer/deserializer eltérést és
crasht/abortot talál, wire-hibát nem — azt a WP6 golden vektorai adják).
A kódban igazolt út:
- chunk `.msg`-osztályok kapnak `Register_Class` + `Register_ClassDescriptor` →
  osztálynév alapján példányosíthatók és van descriptoruk;
- `opp_typename(typeid)` áthidalja a WP2 `typeid`-halmazát a factory/descriptor felé;
- `ChunkSerializer::serialize/deserialize(…, typeInfo)` a registryn át generikus
  belépő (`ChunkSerializerRegistry::getSerializer(typeInfo)`).

Ciklus minden regisztrált típusra (WP2 listája):
1. `opp_typename(typeid)` → osztálynév → `cObjectFactory::find(name)->createOne()`
   (ha nincs factory / absztrakt → skip, riportba);
2. kitöltés a `RandomPacketFiller`-rel (WP4c: descriptor + `@bit`/`@enum`/típus +
   repair-menet) → érvényes-ish példány;
3. serialize → deserialize a registryn át → új chunk;
4. összevetés: bájt-round-trip és/vagy descriptor mezőnkénti diff (a
   származtatott/normalizált mezőkre a WP4b skip-lista).

Marad kézzel (kicsi): **per-típus builder-hook** a keresztmező-kényszerekhez, amit
a filler+repair nem elégít ki (típusra kulcsolt callback, nem egész `.test`), és a
**kurált regressziós szcenáriók**.

> **Lábjegyzet — miért a hex-elsődlegű a fő, és nem egy objektum-konstruáló tábla.**
> Ha hex helyett objektumot konstruálnánk egy lapos `(típus,{mező:érték})` táblából,
> három dolog nem fejezhető ki: polimorf birtokolt al-objektumok (BGP capability,
> OSPFv3 LSA), többszintű beágyazott struct/tömb-fák (PIM `JoinPruneGroup`), és a
> serialize előtt számolt hossz-mezők (`chunkLength`). A **hex-elsődlegű forma ezt
> elkerüli**, mert sosem konstruál, csak összevet. Objektum-konstruáló táblát csak
> ott érdemes, ahol nincs kész hex és serialize-oldalról akarunk konkrét értékeket
> tesztelni; a nem-lapos eseteihez ott is builder-hook kell. (Referenciának: MLD
> teljesen lapos; BGP/PIM/OSPFv3 tartalmazza mindhárom nehézséget.)

## Módosítandó / létrehozandó fájlok

- `tests/unit/lib/SerializerTestLib.h` + `.cc` — **új**, közös `roundTrip<T>` +
  pcap-motor + lefedettség-gyűjtő + fuzz-orákulum.
- `tests/unit/lib/RandomPacketFiller.h` + `.cc` — **új**, descriptor + `@bit`/`@enum`/
  típus-tudatos kitöltés + repair-menet (WP4c); a WP6 meghajtó is ezt használja.
- `tests/unit/lib/GenericSerializerRoundTrip.h` + `.cc` — **új**, a WP7 generikus,
  descriptor-vezérelt random meghajtó (registry-típuslistán iterál).
- `tests/unit/lib/ChunkJsonDump.h` + `.cc` — **új**, rekurzív descriptor→JSON dump
  (a hex-elsődlegű összevetéshez; polimorf altípusnév `__type__`-ként).
- `tests/unit/lib/TsharkOracle.h` + `.cc` — **új**, `tshark -T json` futtatás +
  per-protokoll névtábla + per-típus érték-normalizáló (WP3 + hex-pipeline).
- `tests/unit/lib/Makefile` — a lib build kiterjesztése az új fájlokra (ha kell).
- `src/inet/common/packet/recorder/PcapReader.h/.cc` — `getNetworkType()` getter.
- `src/inet/common/packet/serializer/ChunkSerializerRegistry.h/.cc` — publikus
  típus-felsoroló (WP2 + WP6 alapja).
- `tests/unit/serializer.test` — átírás az új motorra (a jelenlegi 5 pcap zöld marad).
- `tests/unit/IPv6_serializers.test` + a többi per-protokoll `.test` — átállás a
  közös `roundTrip`-re; hosszú távon a kurált maradékká zsugorodnak (WP6, 0b).
- `tests/unit/generic_serializer_roundtrip.test` — **új**, a WP7 random meghajtót futtatja.
- `tests/unit/hex_vectors/<protokoll>.json` — **új**, hex-elsődlegű golden vektorok
  (`{hex, expected_json}`), egy generikus WP6 `.test`-ből futtatva.
- `tests/unit/pcap/<protokoll>/*.pcap` — bővülő korpusz (egyben a pcap+tshark
  hex-pipeline bemenete).

## Javasolt sorrend
1. 0. + 1. + 2. csomag együtt (közös lib + pcap-motor + lefedettség-riport).
   **Előbb a meglévő 5 pcap fájlon zöld** az új `PacketDissector`-alapú motor
   (regresszió: a jelenlegi viselkedést megőrizzük), és fut a teszteletlen-riport.
2. **Csak ezután korpusz-bővítés a teljes lefedettségért**, a 2. csomag
   „teszteletlen serializerek" riportjától vezérelve (célzottan azokra a
   protokollokra/linktype-okra, amik hiányoznak) + ismert-eltérés mechanizmus.
3. WP3 (tshark-orákulum) — a pcap round-trip bájt-eltéréseinek arbitrációja.
4. WP6 (hex-elsődlegű korrektség-motor) — a WP1 korpuszra + WP3 tsharkra épül;
   ez a fő korrektség-tesztelés (pcap+tshark pipeline + golden vektorok).
5. WP4 (fuzz/robusztusság).
6. WP7 (generikus random meghajtó) — a WP2 típuslistára és a WP4c fillerre épül,
   ezért azok után; a teszteletlen serializerek olcsó, széles lefedése.
7. WP5 (fingerprint `~tND` regressziós háló) — bármikor futtatható a meglévő
   fingerprint-suiteon, végső széles ellenőrzésként.

## Ellenőrzés
- `cd /home/zoli/Projects/OMNET/inet/tests/unit && ./runtest serializer.test` —
  a pcap round-trip minden mintafájlon zöld; a régi 5 pcap továbbra is átmegy.
- A lefedettség-riport listázza a teszteletlen serializereket (`%contains`-szal
  vagy külön stdout-ellenőrzéssel rögzíthető a lista).
- A per-protokoll `.test`-ek a közös `roundTrip` után is átmennek (regresszió-check).
- Fuzz: fix seeddel determinisztikus futás, nincs abort/hang; a talált valódi
  bugok külön triage-listára.
- Ha van `tshark`: a normalizálás miatti eltérések automatikusan „nem-bug"-ként
  szűrődnek; hiánya esetén a keret skippel.
- Fingerprint-háló: a fingerprint-suite `~tND` hozzávalókkal futtatva ugyanazt a
  `tplx` fingerprintet adja, mint a sima `tplx` futás (ugyanazon a lokális
  buildon összevetve); eltérés vagy serialize-kivétel = serializer-hiba.
