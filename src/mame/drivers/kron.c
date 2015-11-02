// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  KRON K-180 VGA terminal server
 *
 *  27/10/2015
 *
 * I baught this hardware on Ebay to have something with a Z80 CPU to play with.
 * The hardware is a serial terminal with VGA output and a PC keyboard and was
 * manufactured mid 90:ies by a company from Vinnitsa,Ukraine called KRON. There
 * is a character generator with support for both western and cyrilic characters.
 * The PCB is also filled with chips with cyrrilic characters on but thanks to
 * this page I managed to translate most of them into western TTL logic names:
 *
 * http://ganswijk.home.xs4all.nl/chipdir/soviet/
 *
 * +-----||||||||||||||||||||-|||||||||||----|||||||||||||||||||-|||||||||-||||||||+
 * |     |     RS232C       | |Serial PN|    | CENTRONICS PN   | | VGA   | |KEYBRD||
 * |     +------------------+ +---------+    +-----------------+ +-------+ +------+|
 * |                      +-----+  +-------+  +---------+    +------+ +---------+  |
 * |                      | 1488|  |CD75189|  | 74299   |    | 7407 | | 74244   |  |
 * |                      +-----+  +-------+  +---------+    +------+ +---------+  |
 * |                                           +---------+   +------+              |
 * |                               +-------+   | 74670   |   | 7403 |              |
 * |     +-----------------------+ | 7474  |  ++---------+  ++------+   +-------+  |
 * |     |   Z8018006PSC         | +-------+  | 74374    |  | 74151 |   | 74174 |  |
 * |     |   Z180 MPU            |            +----------+  +-------+   +-------+  |
 * |     +-----------------------+ +-------+                            +-------+  |
 * |                               |74365? |  +---------+    +------+   | 7474  |  |
 * |                               +-------+  | 74374   |    | 7474 |   +-------+  |
 * |                                          +---------+    +------+              |
 * |                              +--------+    +-------+    +------+              |
 * |                              | 74243  |    | 74166 |    | 7474 |              |
 * |+---------+ +---------------+ +--------+    +-------+    +------+              |
 * ||XTAL     | |               |  +-------+    +-------+       +---+   ..-^-..    |
 * ||12.280MHz| | NM27C512      |  | 7432  |    | 74395 |   93C46CB1| /         \  |
 * |+---------+ +---------------+  +-------+    +-------+   EEPROM--+/  Beeper   \ |
 * |+---------+                    | 7400  |     +-------+  +-------+|     O     | |
 * || 74299   |                    +-------+     | 7474  |  | 74259 |\           / |
 * |+---------+  +--------------+                +-------+  +-------+ \         /  |
 * |+---------+  | HY6264A      |  +-------+     +-------+   +------+  ''--_--''   |
 * || 74374   |  | 8Kb SRAM     |  | 7408  |     | 7474  |   | 7414 |           +--+
 * |+---------+  +--------------+  +-------+     +-------+   +------+           |P |
 * |                 +---------+   +-------+  +-----------+  +------+           |W |
 * |                 | 74244   |   | 74393 |  |XTL 29.3MHz|  | 7404 |           |R |
 * +-----------------+---------+---+-------+--+-----------+--+------+--------------+
 *
 *
 * Identified chips
 * -----------------
 * Z180 MPU (Z8018006PCS)
 * NM27C512Q 64Kb EPROM
 * HY6264A 8Kb SRAM
 * 93C46B1 128 bytes EEPROM
 *
 * Misc findings
 * --------------
 * - screen memory at 0x8700
 * - each position has 2 bytes <character> + <mode>
 * - mode 0x08 is double height
 * - characters seems to follow IBM PC Code page 437 for opening screen
 * - terminal defaults to cyrillic characterset possibly due to setting in EEPROM
 * - http://www.phantom.sannata.ru/forum/index.php?t=5200 - Kron-2 for sale
 * - http://f-picture.net/fp/3b2a0496b981437a9c3f90ed236363c9 - Picture of Kron-2
 *
 */

#include "emu.h"
#include "cpu/z180/z180.h"
#include "cpu/z80/z80.h"

#define LOG(x) /* x */

class kron180_state : public driver_device
{
public:
kron180_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu"){ }

	virtual void machine_start ();

	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START (kron180_mem, AS_PROGRAM, 8, kron180_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000, 0x07fff) AM_ROM /* 32 Kb of EPROM	*/
	AM_RANGE (0x08000, 0x09fff) AM_RAM /*  8 Kb of SRAM		*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( kron180_iomap, AS_IO, 8, kron180_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (kron180)
INPUT_PORTS_END

/* Start it up */
void kron180_state::machine_start ()
{
	LOG (logerror ("machine_start\n"));
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (kron180, kron180_state)
/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", Z180, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP (kron180_mem)
	MCFG_CPU_IO_MAP(kron180_iomap)
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (kron180)
	ROM_REGION (0x1000000, "maincpu", 0)

#if 0
// Full 64Kb EPROM but forst half was just garbish to trimmed away the first 32Kb from the EPROM
	ROM_LOAD ("kron.bin", 0x000000, 0x8000, CRC (ae0642ad) SHA1 (2c53a714de6af4b64e46fcd34bca6d4438511765))
#endif

// Last half moved from 0x8000 to 0x0000, works but need to trace A15 from EPROM, probably connected to GND.
	ROM_LOAD ("kron.bin", 0x000000, 0x8000, CRC (6beed65e) SHA1 (338d6b77349d4d50488a4393bcd4f5fe4190d510))
ROM_END

/* Driver */
/*	  YEAR	NAME		  PARENT  COMPAT   MACHINE		   INPUT	 CLASS			INIT COMPANY				  FULLNAME			FLAGS */
COMP (1995, kron180,	  0,	  0,	   kron180,		   kron180, driver_device, 0,	"Kron Ltd",				"Kron K-180", MACHINE_IS_SKELETON )
