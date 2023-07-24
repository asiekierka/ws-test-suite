# ws-test-suite

My own assortment of various WonderSwan tests, designed to help understand the hardware.

Licensed under the MIT license.

Tests are split into two main categories:

* `mono` - WS/WSC-compatible tests,
* `color` - WSC-exclusive tests,
* `tools` - various tools (rough documentation below).

Other useful tests:

 * [WSTimingTest](https://github.com/FluBBaOfWard/WSTimingTest) - covers NEC V30MZ CPU instruction timing
 * [WSHWTest](https://github.com/FluBBaOfWard/WSHWTest) - covers some aspects of interrupt and timer handling
 * [rtctest](https://forums.nesdev.org/viewtopic.php?t=21513) - covers the 2003 mapper's S-3511 RTC
 * [Robert Peip's test ROMs](https://github.com/MiSTer-devel/WonderSwan_MiSTer/tree/main/testroms) - tools for manual sprite priority and window testing

## Tools

### timing-validator

Console timing/behaviour valiation tool, split into three tabs: (T)iming, (M)emory, (I)/O ports.

* Toggle tabs with START
* X1/X2/X3/X4 to move around in all tabs
* (T)iming: used to define IRQ handlers using simple micro-operations; Y1/Y3 to change options, A to apply
* (M)emory: simple memory editor; A/B to switch 0x80-sized pages, Y1/Y3 to change values
* (I)/O ports: simple I/O port editor; Y4/Y2 to switch pages, Y1/Y3 to change values, A to apply, B to refresh
