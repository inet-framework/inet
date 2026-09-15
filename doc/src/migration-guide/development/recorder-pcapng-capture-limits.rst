PCAPng Capture Limits
~~~~~~~~~~~~~~~~~~~~~

PCAPng captures now obey the configured capture length limit, ``snaplen``, in bytes. Before this change, ``PcapngWriter`` wrote the complete selected range regardless of a finite limit. With a finite limit, the writer truncates the captured bytes but keeps the original length in the record. A zero limit means unlimited for PCAPng.

For example, a selected range of six bytes with ``snaplen = 4`` produces four captured bytes and an original length of six. Set a sufficiently large limit or zero if your PCAPng consumers require complete records. The PCAPng block can contain alignment padding after the captured bytes. That padding does not count toward the captured length or the original length.
