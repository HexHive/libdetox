# Lockdown README

This is the first research prototype release of Lockdown. Lockdown is
a run-time environment for x86 32bit ELF binaries that transparently
hardens binary applications at run-time against the exploitation of
memory corruption vulnerabilities. Lockdown implements a fine-grained
CFI policy for jumps and calls. Return instructions are secured over
a practical shadow stack. Furthermore Lockdown has its own ELF dynamic
loader that loads ELF binaries (executables and shared libraries).

Lockdown is a flavor of TRuE (Trusted Run-TimE) which itself is based
on fastBT, secureLoader and libdetox. This release contains a snapshot
of all these components including the configuration for Lockdown.


## Compatibility

As a research prototype run-time for binaries, binary compatibility is
not always guaranteed. Lockdown has its own ELF loader implementation
that strongly depends on the libc in use. There are datastructures that
are shared between the loader and libc. A productive implementation
should make it easy to export this libc specific information such
that all libc versions can easily be supported.
This research prototype implementation further uses a small whitelist
of control tranfers for some low-level run-time libraries. The whitelist
is small and could easily be generated automatically. A productive
implementation should do more static and dynamic analysis to also catch
these transfers.
This Lockdown version therefore will only work with the specific system
it was tested with namely Ubuntu 12.04 for 32bit x86.

Tested with Ubuntu 12.04.5 LTS (codename: precise) and gcc/g++ 4.6.3-1ubuntu5


## Build

make all


## Run

bin/trustedRT <ELF executable>
Example: "bin/trustedRT /bin/ls" or "bin/trustedRT /bin/nano"


## Build and run tests

make test
test/runtests.rb -runtime

Note: you have to run the runtests.rb script from the top level
directory (not within the test/ directory).


## References

Fine-Grained Control-Flow Integrity through Binary Hardening
Mathias Payer, Antonio Barresi, and Thomas R. Gross.
In DIMVA'15: 12th Conference on Detection of Intrusions and Malware and Vulnerability Assessment, 2015.
https://nebelwelt.net/publications/files/15DIMVA.pdf
See the [HexHive group homepage](http://nebelwelt.net/group) for publications and vidoes.
