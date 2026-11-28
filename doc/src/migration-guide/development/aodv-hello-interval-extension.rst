AODV Hello interval encoding
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Hello RREPs now include six extension bytes. Packet-length assertions and raw-wire peers must allow that extension. The base RREP format is unchanged, and a received Hello without an extension retains the local configured interval as its fallback.

The nominal timer period is unchanged. Its advertised interval is rounded upward to whole milliseconds; advertised lifetime and Hello-related configuration minima cover that conservative interval. Configurations whose encoded interval or lifetime exceeds the uint32 millisecond wire range are rejected at startup. Malformed, duplicate or unsupported mandatory extensions do not renew neighbor observation.
