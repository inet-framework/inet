# Cellular architecture prototype

A dummy LTE/NR cellular network on the INET radio medium (`inet.prototype.cellular`). It is
meant for INET developers: it shows how a cellular system with carrier aggregation maps onto
INET's radio medium, and it puts a realistic load on the medium, so that the medium can be
measured and optimized against it. The physics, the scheduling and the error model are
deliberately trivial; the structure of the transmissions and of the events is realistic.

## What the model does

- **One radio per component carrier.** A base station sector and a UE have one `CellularRadio`
  per carrier under one MAC entity; that is carrier aggregation. TDD carriers use one frequency;
  FDD carriers tune the transmitter and the receiver apart.
- **One transmission per allocation.** Each slot of each carrier, a sector sends one downlink
  transport block to each of a few UEs, and the UEs send uplink transport blocks in the slots
  the sector granted. All cells start their allocations at the same symbol boundaries.
- **The signal is a power spectral density lattice.** A transmission's power is a
  `PeriodicallyInterpolated2DFunction` with one cell per (OFDM symbol, PRB) of the carrier, zero
  outside the allocation, so a 100 MHz carrier at 30 kHz has 273 PRBs and 12 or 14 symbols per
  transmission. Everything after the transmitter is INET's dimensional analog model.
- **Panels are shared, and beams are per allocation.** The radios of one band use one
  `PanelAntenna` through `SharedAntennaRef`; each downlink allocation uses the beam closest to
  its UE.
- **Delivery goes to the intended receiver only.** The listening filter of the medium asks the
  receiver, which accepts only allocations addressed to its radio; every other transmission is
  only interference.
- **Control is ideal.** Grants and SNIR reports are method calls through `IdealControlChannel`.

## Workarounds that mark missing INET features

The code labels each place where it works around a missing INET feature with the number of the
change that would remove the workaround:

- **C1:** antennas give only their current gain, so `CellularTransmission` replaces the gain
  snapshot with the gain of the allocation's beam.
- **C3:** a radio cannot share an antenna, so its antenna slot holds a `SharedAntennaRef`.
- **C6:** receivers have no noise figure, so the background noise includes it.
- **C14:** decision regions have no frequency dimension, so the error model integrates the SNIR
  function over the allocation's PRBs itself, and decides once per transport block.
- **C15:** the transmitter analog model gets no packet, so `CellularTransmitter` builds the
  analog model itself.
- **C16:** the receiver attempts a reception only if no other one is in progress, so
  `CellularReceiver` overrides the rule for the concurrent uplink receptions of a sector.
- **C17:** the analog model gives the direction between two antennas at the same position (the
  sectors of a site) as a NaN rotation, so `PanelAntenna` returns its minimum gain for it.

## Configurations

- `Tiny`: one site, one TDD carrier, 9 UEs; a smoke test.
- `Small`: 7 sites, a TDD carrier and an FDD carrier on two bands, 210 UEs.
- `SmallBitmap`: as `Small`, with interleaved RBG allocations in the downlink.
- `Large`: 19 sites, four carriers on three bands, 1140 UEs.
- `CarrierSweep`: 7 sites and 210 UEs with 1, 2 or 4 carriers, which isolates the cost of the
  number of radios.

## Benchmark

`run-benchmark.py` runs configurations in Cmdenv express mode and reports the wall time, the
event rate and the counters of the radio medium. Measure with the release build:

```
./run-benchmark.py Tiny Small
./run-benchmark.py --mode debug Tiny
```
