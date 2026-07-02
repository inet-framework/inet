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

### Állapot (mi van kész)
- **WP0 + WP1 kész és zöld:** a `serializer.test` az új, generikus motorra állt
  (`PcapReader` + `PacketDissector`/`ChunkBuilder`, közös `tests/unit/lib/SerializerTestLib`).
  Mind az 5 meglévő pcap round-trippel (`opp_test` PASS).
- **PcapReader src-fixek** (külön commit): fejléc a tagba, veszteségmentes
  `PcapRecordTime` időbélyeg, tiszta EOF, csak-ha-kért névadás.
- **TcpHeaderSerializer** végtelen-ciklus javítva (`!isReadBeyondEnd()` guard) —
  menet közben talált valós robusztussági bug; csonkolt/sérült TCP-fejléc is
  kiváltja.
- Commitok a `topic/bz/serializertest` branchen: plan, PcapReader-fix,
  serializer.test-átírás, TCP-fix.

Következő: **WP2 (lefedettség + nyers-bájt riport)**, majd a wire-helyesség
egyszeri-hitelesített kanonikus snapshotja (lásd WP3 átdolgozva).

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

### 1. munkacsomag — PCAP-készlet alapú round-trip (KÉSZ)
- `PcapReader`-re épül (a kézi pcap-parser eldobva). A linktype→`Protocol` leképzést
  és a `PacketProtocolTag`-et már a `PcapReader` adja (nem kellett külön getter, a
  `fileHeader`-bug javítása aktiválta).
- A kézi `deserialize()` láncot **`PacketDissector` + `ChunkBuilder`** váltja ki
  → egy kódból minden regisztrált protokollra megy.
- Folyamat rekordonként: `PcapReader::readPacket` → (FCS-mentes ethernetnél számított
  FCS hozzáfűzése) → `dissectPacket` (a tag protokolljával) → `ChunkBuilder`
  visszaépíti a tartalmat → `peekAllAsBytes()` → bájt-összevetés; eltérésnél az
  **első eltérő offset** + környezet kiírása. Csonkolt (incomplete) rekord kihagyva.

**Nyers-bájt figyelmeztetés (fontos korlát):** amelyik régióhoz **nincs disszektor**,
azt a `DefaultProtocolDissector` egy `BytesChunk`-ként adja tovább, ami **triviálisan
round-trippel** → a „frame is the same" arra a régióra **vak-zöld** (nulla serializer-
logika futott). Tehát a **pcap-PASS nem jelent lefedettséget** — ezt a WP2 teszi
láthatóvá. (Levél-payload jogosan bájt; a „parse-olni kellett volna" fejléc a gond.)

- Korpusz `tests/unit/pcap/<protokoll>/` alá — forrásokról lásd a „PCAP-források" szakaszt.

### PCAP-források (a korpuszhoz — élő rögzítés nélkül)
Kimenő hálózat **elérhető** ebből a környezetből (letöltés működik) — de a valódi
korlát a **licenc/redisztribúció**, mert commitolt korpuszról van szó (INET = LGPL-3.0).
- **Elsődleges: `text2pcap` RFC-/kézi hexből** (telepítve). Self-authored → nincs
  harmadik feles licenc, teljesen újraterjeszthető, és **független** wire-igazság.
- **INET `PcapRecorder`** (a sim maga generálja): olcsó, offline, széles — de a bájtok
  az INET saját serializereiből jönnek → **csak self-consistency/regresszió**, nem
  wire-helyesség (körkörös), hacsak tsharkkal nem hitelesítjük.
- **Publikus capture** csak licenc-tiszta, INET-kompatibilis esetben (fájlonként).
- **Runtime-letöltés kizárva** — a CI-nek offline/determinisztikusnak kell lennie.
- Minden forrás `tshark`-kal validálható (független orákulum).

### 2. munkacsomag — Lefedettség-riport + nyers-bájt jelölés (a következő lépés)
Egyetlen műszerezés, ami a #1 (nyers-bájt) és a lefedettség kérdést is megoldja.
- **`src` módosítás:** `ChunkSerializerRegistry`-be publikus felsoroló
  (pl. `std::vector<std::type_index> getRegisteredTypes() const`) → a **regisztrált**
  halmaz.
- **Exercised-halmaz:** a dissection `visitChunk`-ja minden érintett chunk
  `typeid`-jét rögzíti → a **tesztelt** halmaz. A `BytesChunk`/`BitsChunk`-ként látott
  régiók = **nyers/nem tesztelt** (a #1 esete) — külön listázva, a top-level azonnali
  bájtra-esést kiemelve (gyanús), a mély levél-payloadot csak halkan jelezve.
- **Kimenet:** `tested serializers` lista + `untested serializers` lista
  (`regisztrált − tesztelt`) + nyers-bájt régiók → a run végén a stdout-ra.
- **Gate-forma (brittleness-kezelés):** a **teljes listát printeljük** (test.out),
  de hard `%contains`-gate csak **kurált baseline-ra / darabszámra** (coverage-ratchet:
  jelez, ha a lefedettség csökken), NEM a pontos listára — különben minden új
  serializer/pcap elmozdítaná. (A pontos-lista-gate opció, ha explicit kaput akarsz.)
- **Caveat:** a `visitChunk` a **top-level disszektált** chunkokat látja; az opció-/
  al-chunk serializerek (a fejléc `serialize`-én belül futnak) „untested"-ként
  jelenhetnek meg, pedig használódnak — a riport „top-level érintettség"-et mér,
  dokumentálni kell.

### 3. munkacsomag — Wire-helyesség: egyszer-hitelesített kanonikus snapshot
(`tshark` telepítve: 4.6.4.) A wire-helyesség orákuluma egy **befagyasztott,
kanonikus mező-dump snapshot** protokollonként/csomagonként, amit **egyszer**,
létrehozáskor hitelesítünk függetlenül. Ez leváltja a korábban tervezett drága,
karbantartandó tshark↔INET **mezőnév-tábla + érték-normalizáló** kódot.

Munkafolyamat:
1. **Kanonikus dump** az INET dekódjából: rekurzív descriptor→JSON (a WP6
   `ChunkJsonDump`-ja), NEM a formázás-érzékeny „szép" `PacketPrinter`-szöveg.
2. **Egyszeri, független hitelesítés (authoring-idő, nem CI):** a csomag bájtjait
   `tshark -T json`-nal is dekódoljuk, és **AI-val** vetjük össze az INET-dumppal —
   az AI szemantikusan hidalja a mezőnév-eltérést, így **nem kell névtáblát/normalizálót
   írni és karbantartani**. Egyezés → a snapshot elfogadva (blessed).
3. **Auditnyom:** a snapshot mellé mentjük a **tshark-kimenetet is** és egy
   `verified: <dátum/eszköz>` jelölőt → később bárki újraellenőrizheti; az „AI
   hitelesítette" ne legyen láthatatlan bizalom.
4. **CI:** a run csak az INET-dumpot veti a **befagyasztott** snapshothoz
   (`%contains`/fájl-diff) — gyors, offline, determinisztikus, és **már hitelesített**.
5. **Re-bless:** szándékos serializer-változásnál a snapshot diffel → az egyszeri
   AI+tshark hitelesítést újrafuttatjuk az új snapshotra → újrafagyasztás
   (approval-testing minta).

Ez tisztán **megtöri a körkörösséget** a létrehozáskor (tshark = független bíró),
utána viszont olcsó. Korlát: csak azt fedi, amit az INET disszektál — a nyers-bájt
régiók (WP2/#1) itt is nyersen jelennek meg.

Külön, olcsó felhasználás névtábla nélkül: **round-trip bájt-eltérés arbitrációja** —
ha a re-serialize bájtjai eltérnek, ugyanazt a két bájtsort tsharkon átfuttatva, ha a
dekódolt mezők egyeznek → jóindulatú normalizálás (nem bug); ha eltérnek → valódi
bug. (Ehhez nincs szükség INET-névtáblára, mindkét oldal tshark.)

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

**Egyesített pcap-pipeline (a hex-elsődlegű forma automatizálása skálán):**
ez összeköti a WP1 + WP3 + WP6-ot:
- pcap rekord = az elsődleges hex;
- INET deserialize → objektum → kanonikus descriptor→JSON-dump;
- a dumpot a **WP3 egyszer-hitelesített snapshotjához** diffeljük (CI-ben gyors,
  offline); a snapshot wire-helyességét **egyszer**, létrehozáskor a `tshark -T json`
  + **AI** összevetés adja → **nincs karbantartandó mezőnév-tábla/normalizáló** (ezt
  a korábbi „fő költséget" az egyszeri AI-hitelesítés váltja ki, lásd WP3);
- serialize vissza → az eredeti hex (round-trip a serialize-oldalra).
- **Snapshot vs független igazság:** a befagyasztott snapshot magában csak
  *regressziót* fog — ezért kötelező a **WP3 szerinti egyszeri független hitelesítés**
  (tshark+AI) + auditnyom, hogy a snapshot ne az INET esetleges hibáját rögzítse.

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

Kész (landolt):
- `tests/unit/lib/SerializerTestLib.h` + `.cc` — a pcap round-trip motor + FCS-kezelés
  (a lefedettség-gyűjtő és a nyers-bájt jelölés WP2-ben bővül majd).
- `tests/unit/lib/Makefile` — kiegészítve az új objektummal.
- `src/inet/common/packet/recorder/PcapReader.h/.cc` — `fileHeader` tárolás,
  veszteségmentes `PcapRecordTime`, tiszta EOF, csak-ha-kért névadás.
- `src/inet/queueing/source/PcapFilePacketProducer.cc` — a simtime-konverzió a
  `PcapRecordTime`-ból (a policy itt lakik).
- `src/inet/transportlayer/tcp_common/TcpHeaderSerializer.cc` — opció-ciklus guard.
- `tests/unit/serializer.test` — átírva a generikus motorra (5 pcap zöld).

Hátralévő:
- `src/inet/common/packet/serializer/ChunkSerializerRegistry.h/.cc` — publikus
  típus-felsoroló (WP2 + WP7 alapja).
- `tests/unit/lib/RandomPacketFiller.h` + `.cc` — descriptor + `@bit`/`@enum`/típus-
  tudatos kitöltés + repair-menet (WP4c); a WP7 meghajtó is ezt használja.
- `tests/unit/lib/GenericSerializerRoundTrip.h` + `.cc` — a WP7 generikus random meghajtó.
- `tests/unit/lib/ChunkJsonDump.h` + `.cc` — rekurzív descriptor→JSON kanonikus dump
  (WP3 snapshot + WP6 összevetés; polimorf altípusnév `__type__`-ként).
- `tests/unit/lib/verify_snapshots` — **authoring-idejű** eszköz: `tshark -T json` +
  AI-összevetés az egyszeri hitelesítéshez; a snapshot mellé tshark-kimenet + `verified`
  jelölő (WP3). NEM CI-lépés.
- `tests/unit/IPv6_serializers.test` + a többi per-protokoll `.test` — átállás a
  közös `roundTrip`-re; hosszú távon a kurált maradékká zsugorodnak (WP6, 0b).
- `tests/unit/generic_serializer_roundtrip.test` — a WP7 random meghajtót futtatja.
- `tests/unit/snapshots/<protokoll>.json` — befagyasztott, egyszer-hitelesített
  kanonikus mező-dumpok (+ a hozzájuk mentett tshark-kimenet és `verified` jelölő).
- `tests/unit/pcap/<protokoll>/*.pcap` — bővülő korpusz (forrás: WP1 „PCAP-források").

## Javasolt sorrend
1. ~~WP0 + WP1: közös lib + pcap round-trip motor, 5 pcap zöld.~~ **KÉSZ.**
2. **WP2 — lefedettség-riport + nyers-bájt jelölés (a következő lépés):** registry-
   felsoroló + `visitChunk` typeid-gyűjtő + tested/untested listák + bytes-régiók.
3. **Korpusz-bővítés** a WP2 riporttól vezérelve (célzottan a hiányzó protokollokra),
   `text2pcap` RFC-hexből az elsődleges forrás (lásd „PCAP-források").
4. WP3 — wire-helyesség: kanonikus JSON snapshot, egyszeri tshark+AI hitelesítéssel.
5. WP6 — hex-elsődlegű korrektség-motor a snapshotokra épülve.
6. WP4 (fuzz/robusztusság).
7. WP7 (generikus random meghajtó) — a WP2 típuslistára + WP4c fillerre épül.
8. WP5 (fingerprint `~tND` regressziós háló) — bármikor futtatható, végső háló.

## Ellenőrzés
- **pcap round-trip (kész):** `opp_test run -v -p serializer/out/clang-debug/serializer_dbg
  serializer.test` a `tests/unit`-ban → PASS; mind az 5 pcap átmegy, `test.err` üres.
  (A `runtest` script hiányzik ebből a munkamásolatból; a klasszikus `opp_test` flow-t
  használjuk: `make` a `lib`-ben, `opp_test gen`, `make` a `work/serializer`-ben, majd
  `opp_test run`.)
- **WP2:** a lefedettség-riport a stdout-ra kerül; hard gate a kurált baseline-ra/darabszámra.
- A per-protokoll `.test`-ek a közös `roundTrip` után is átmennek (regresszió-check).
- **WP3 snapshot:** CI-ben a kanonikus dump a befagyasztott snapshothoz diffel; a
  snapshot egyszer, authoring-időben lett tshark+AI-val hitelesítve (auditnyommal).
- Fuzz: fix seeddel determinisztikus futás, nincs abort/hang; a talált valódi bugok
  külön triage-listára.
- Fingerprint-háló: `~tND` futás ugyanazt a `tplx` fingerprintet adja, mint a sima
  `tplx` (ugyanazon a lokális buildon); eltérés vagy serialize-kivétel = serializer-hiba.
