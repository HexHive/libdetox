# TRuE/Lockdown

**TRuE/Lockdown** is a secure runtime environment that enables the safe execution of untrusted (but not
malicious) code.  Untrusted code (e.g., the Apache server) is dynamically analyzed and secured against
different forms of control-flow based attacks like code injection, control-flow redirection, and return
oriented programming. This fine-grained security layer detects attacks right when they happen (i.e., before the
control flow is executed) and the program is terminated. A second layer of protection uses a system-call
interposition layer to validate each executed system call against a vigorous system call policy. TRuE uses
several components to both extract information from the application and to secure the application:

* A **secure loader** replaces the standard loader (ld.so). The secure loader is a part of the trusted domain and
  extracts information about each used shared object (the executable and all loaded shared libraries). The
  information contains details about individual memory regions and the locations of symbols and relocation slots.
  Details about the secure loader are published in the TRuE paper. The information is then used in the binary
  translator to weave control flow checks into the translated application code.
* **libdetox** is the virtualization system that separates user-space into two execution domains: the trusted
  sandbox domain that contains the binary translator and the untrusted application domain. The sandbox domain
  uses a separate stack from the application stack and makes sure that no pointers leak to the application. All
  application code is translated using the binary translator. In addition, all system calls are redirected from
  the application domain to the sandbox domain where they are handled (interposed, redirected, or executed). The
  libdetox system is published at VEE'11, 27c3'11, and 26c3'09. The binary translator is also extended with
  low-level domain specific aspect language that makes code generation easier, see DSAL'12 for more information.
* The binary translator **fastBT** uses a table-based translation scheme and thread-local code caches to
  dynamically translate all application code. Trampolines are used to translate individual indirect control flow
  transfers (indirect jumps, indirect calls, and function returns). The translation tables cover the complete set
  of x86-ia32 instructions and a table generator can be used to specify high-level code transformations (the API
  is similar to PIN with the difference that fastBT offers the high-level API at compile time while PIN offers
  the API at runtime). fastBT is published at SYSTOR'10 and AMAS-BT'09.

For a complete description and evaluation of TRuE look at Mathias Payer's PhD thesis. If you are interested in
a quick overview watch the Google TechTalk for an overview of fastBT, or the 27c3 talk and the 26c3 talk for an
overview of libdetox. See the [HexHive group homepage](http://nebelwelt.net/group) for publications and vidoes.

TRuE has the following runtime requirements: a machine with an IA-32 (x86) CPU, a Linux kernel 2.6 or higher,
and GCC version 4.2 or higher. You can download a given software package below, unpack it in a directory, read
the INSTALL information, (optionally) adapt the translation tables, configure the optimizations (for both
additional security and performance settings) and let it run. fastBT and libdetox both use LD_PRELOAD to inject
the binary translator into the application while TRuE uses the secure loader to initialize the sandbox before
the application is loaded.
