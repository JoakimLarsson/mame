// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  KRON K-180 VGA terminal server
 *
 *  27/10/2015
 *
 * I baught this hardware on Ebay to have something with a Z80 CPU to play with.
 * The hardware is a serial terminal controller with VGA output and a PC keyboard 
 * and was manufactured mid 90:ies by a company from Vinnitsa,Ukraine called KRON. 
 * There is a character generator with support for both western and cyrilic characters.
 * The PCB is also filled with chips with cyrrilic characters on but thanks to this 
 * page I managed to translate most of them into western TTL logic names:
 *
 * http://ganswijk.home.xs4all.nl/chipdir/soviet/
 *
 * +-----||||||||||||||||||||-|||||||||||----|||||||||||||||||||-|||||||||-||||||||+
 * |     |     RS232C       | |Serial PN|    | CENTRONICS PN   | | VGA   | |KEYBRD||
 * |     +----------------XP1 +-------XP2    +---------------XS3 +-----XS2 +----XS1|
 * |                      +---DD27 +-----DD20 +-------DD34   +----DD3 +-------DD33 |
 * |                      | 1488|  |CD75189|  | 74299   |    | 7407 | | 74244.1 |  |
 * |                      +-----+  +-------+  +---------+    +------+ +---------+  |
 * |                                           +------DD17   +----DD9              |
 * |                               +-----DD35  | 74670   |   | 7403 |              |
 * |     +--------------------DD10 | 7474  |  ++------DD12  ++----DD14  +-----DD13 |
 * |     |   Z8018006PSC         | +-------+  | 74374    |  | 74151 |   | 74174 |  |
 * |     |   Z180 MPU            |            +----------+  +-------+   +-------+  |
 * |     +-----------------------+ +-----DD36                           +------DD6 |
 * |                               |7432.1 |  +-------DD2    +----DD7   | 7474  |  |
 * |                               +-------+  | 74374   |    |7474.1|   +-------+  |
 * |                                          +---------+    +------+              |
 * |                              +------DD15   +-----DD1    +----DD19             |
 * |                              | 74243  |    | 74166 |    |7474.2|              |
 * |+-------BQ1 +-------------DD4 +--------+    +-------+    +------+              |
 * ||XTAL     | |               |  +-----DD16   +-----DD18      +-DD28  ..-^-..    |
 * ||12.280MHz| | NM27C512      |  | 7432  |    | 74395 |   93C46CB1| /         \  |
 * |+---------+ +---------------+  +-----DD37  DD18-----+   EEPROM--+/  Beeper   \ |
 * |+---------+                    | 7400  |     +----DD32  +----DD23|     O     | |
 * || 74299   |                    +-------+     | 7474  |  | 74259 |\   BQ2     / |
 * |+---------+  +------------DD5                +-------+  +-------+ \         /  |
 * |+---------+  | HY6264A      |  +-----DD8     +----DD31   +---DD25  ''--_--''   |
 * || 74374   |  | 8Kb SRAM     |  | 7408  |     | 7474  |   | 7414 |           +XS4
 * |+------DD24  +--------------+  +-------+     +-------+   +------+           |P |
 * |                DD30-------+   +-----DD29 +---------BQ3  +---DD11           |W |
 * |                 | 74244   |   | 74393 |  |XTL 29.3MHz|  | 7404 |           |R |
 * +-----------------+---------+---+-------+--+-----------+--+------+--------------+
 *
 * 74299 - 8 bit bidirectional universal shift/storage register
 * 7407  - 6 open collector drivers
 * 74244 - Octal buffers/line drivers/line receivers
 * 74670 - 4 x 4 register file with 3-state outputs
 * 74151 - 1 of 8 demultiplexor
 * 74374 - octal D type flip flops
 * 74174 - 6 D type  flip flops with clear
 * 75189 - quadruple line receivers
 * 74395 - 4 bit universal shift registers
 * 74393 - dual 4 bit counters
 * 74259 - 8 bit adressable latches
 * 74166 - 8 bit shift register
 * 
 *
 * Keyboard interface
 * ------------------
 * XT: 1 start bit + 8 data bits @ ~2kbps
 * AT: 1 start bit + 8 data bits + 1 odd parity bit @ ~10kbps 
 *
 * Pin 1 CLK/CTS - 
 * Pin 2 RxD   (AT:+ TxD/RTS) - via R to GND + Pin 4 (out) 7407 + pin 11 (DS0) 74299 8 bit shift register
 *       u                                 Pin 3 (in)  7407 + pin 11 (INT2) CPU + pin 1 74299 
 *  1         3                            + pin 4 (*PRE) 7474.2 + pin 6 (*Q) 7474.2
 *    4  2  5                              Pin 3 (CLK) 7474.2 + pin 8 (4Y) 7414
 *                                         Pin 2 (D) 7474.2 + Pin 17 (Q7 serial out) 74299
 * Pin 3 Reset (AT:+ NC)
 * Pin 4 GND                  - GND
 * Pin 5 +5v                  - VCC
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
 * - $17B9 might be keyboard input routine
 * - indentified used OUT ports: $00, $02, $04, $07, $08, $0A, $0C, $0E, $0F, $40, $60  
 * - identified used IN ports: $10 (keyboard?), $30
 * - screen memory at 0x8600
 * - each position has 2 bytes <character> + <mode>
 * - mode 0x08 is double height
 * - characters seems to follow IBM PC Code page 437 for opening screen
 * - terminal defaults to cyrillic characterset possibly due to setting in EEPROM
 * - http://www.phantom.sannata.ru/forum/index.php?t=5200 - Kron-2 for sale
 * - http://f-picture.net/fp/3b2a0496b981437a9c3f90ed236363c9 - Picture of Kron-2
 * - http://www.kron.com.ua/ - the company has no info on legacy hardware unfortunality
 *
 */

#include "emu.h"
#include "cpu/z180/z180.h"
#include "cpu/z80/z80.h"

#define VERBOSE 0

#define LOGPRINT(x) do { if (VERBOSE) logerror x; } while (0)
#define LOG(x) {}
#define LOGSCAN(x) LOGPRINT(x)
#define LOGSCREEN(x) {}
#define RLOG(x) {}
#define LOGCS(x) {}

#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class kron180_state : public driver_device
{
public:
kron180_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device (mconfig, type, tag)
	,m_maincpu (*this, "maincpu")
	,m_videoram(*this, "videoram")
	{ }
	UINT8 *m_char_ptr;
	UINT8 *m_vram;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
protected:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
private:
	virtual void machine_start ();
};

static ADDRESS_MAP_START (kron180_mem, AS_PROGRAM, 8, kron180_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000, 0x07fff) AM_ROM /* 32 Kb of EPROM	*/
	AM_RANGE (0x08000, 0x085ff) AM_RAM 
	AM_RANGE (0x08600, 0x095ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE (0x09600, 0x09fff) AM_RAM 
ADDRESS_MAP_END

static ADDRESS_MAP_START( kron180_iomap, AS_IO, 8, kron180_state )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (kron180)
INPUT_PORTS_END

/* Video */
UINT32 kron180_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int vramad;
	UINT8 *chardata;
	UINT8 charcode;

	LOGSCREEN(("%s()\n", FUNCNAME));
	vramad = 0;
	for (int row = 0; row < 25 * 8; row += 8)
	{
		for (int col = 0; col < 80 * 8; col += 8)
		{
			/* look up the character data */
			charcode = m_vram[vramad];
			if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n %c at X=%d Y=%d: ", charcode, col, row));
			chardata = &m_char_ptr[(charcode * 8) + 8];
			/* plot the character */
			for (y = 0; y < 8; y++)
			{
				chardata--;
				if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n  %02x: ", *chardata));
				for (x = 0; x < 8; x++)
				{
					if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN((" %02x: ", *chardata));
					bitmap.pix16(row + (8 - y), col + (8 - x)) = (*chardata & (1 << x)) ? 1 : 0;
				}
			}
			vramad += 2;
		}
		if (VERBOSE && charcode != 0x20 && charcode != 0) LOGSCREEN(("\n"));
		vramad += 96; // Each row is aligned at a 128 byte boundary
	}

	return 0;
}

/* Start it up */
void kron180_state::machine_start ()
{
	LOG (logerror ("machine_start\n"));
	m_char_ptr  = memregion("chargen")->base();
	m_vram      = (UINT8 *)m_videoram.target();
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (kron180, kron180_state)
/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", Z180, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP (kron180_mem)
	MCFG_CPU_IO_MAP(kron180_iomap)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DRIVER(kron180_state, screen_update)
	MCFG_SCREEN_SIZE(81 * 8, 25 * 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 647, 0, 199)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (kron180)
	ROM_REGION (0x1000000, "maincpu", 0)

#if 0
// Full 64Kb EPROM but first half was just garbish so I trimmed away the first 32Kb from the EPROM
	ROM_LOAD ("kron.bin", 0x000000, 0x8000, CRC (ae0642ad) SHA1 (2c53a714de6af4b64e46fcd34bca6d4438511765))
#endif

// Last half moved from 0x8000 to 0x0000, works but need to trace A15 from EPROM, probably connected to GND.
	ROM_LOAD ("kron.bin", 0x000000, 0x8000, CRC (6beed65e) SHA1 (338d6b77349d4d50488a4393bcd4f5fe4190d510))

#if 0
	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "e100U506.bin", 0x0000, 0x0800, CRC(fff9f288) SHA1(2dfb3eb551fe1ef67da328f61ef51ae8d1abdfb8) )
#else
	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD( "cga.chr", 0x0000, 0x1000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd) )
#endif
ROM_END

/* Driver */
/*	  YEAR	NAME		  PARENT  COMPAT   MACHINE		   INPUT	 CLASS			INIT COMPANY				  FULLNAME			FLAGS */
COMP (1995, kron180,	  0,	  0,	   kron180,		   kron180, driver_device, 0,	"Kron Ltd",				"Kron K-180", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
