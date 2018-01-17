// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-1/CPU-6 VME SBC drivers, initially based on the 68ksbc.c
 *
 *  13/06/2015
 *
 * The info found on the links below is for a later revisions of the board I have
 * but it is somewhat compatible so I got the system ROM up and running in terminal.
 * My CPU-1 board has proms from 1983 and the PCB has no rev markings so probably
 * the original or a very early design. The board real estate differs from the later
 * CPU-1:s I found pictures of but has the same main chips and functions.
 *
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/1988_Force_VMEbus_Products.pdf
 * http://www.artisantg.com/info/P_wUovN.pdf
 *
 * Some info from those documents:
 *
 * Address Map
 * ----------------------------------------------------------
 * Address Range     Description
 * ----------------------------------------------------------
 * 000 000 - 000 007 Initialisation vectors from system EPROM
 * 000 008 - 01F FFF Dynamic RAM on CPU-1 B
 * 000 008 - 07F FFF Dynamic RAM on CPU-1 D
 * 080 008 - 09F FFF SYSTEM EPROM Area
 * OAO 000 - OBF FFF USER EPROMArea
 * 0C0 041 - 0C0 043 ACIA (P3) Host
 * 0C0 080 - 0C0 082 ACIA (P4) Terminal
 * 0C0 101 - 0C0 103 ACIA (P5) Remote device (eg serial printer)
 * 0C0 401 - 0C0 42F RTC
 * OEO 001 - 0E0 035 PI/T (eg centronics printer)
 * OEO 200 - 0E0 2FF FPU
 * OEO 300 - 0E0 300 Reset Off
 * OEO 380 - 0E0 380 Reset On
 * 100 000 - FEF FFF VMEbus addresses (A24)
 * FFO 000 - FFF FFF VMEbus Short I/O (A16)
 * ----------------------------------------------------------
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 * ABORT                        Switch  7    31
 * Real Time Clock (RTC)        58167A  6    30
 * Parallel/Timer (PI/T)        68230   5    29
 * Terminal ACIA                6850    4    28
 * Remote ACIA                  6850    3    27
 * Host ACIA                    6850    2    26
 * ACFAIL, SYSFAIL              VME     5    29
 * Off board Sources (other VME boards)
 * 6 Port Serial I/O board      SIO     4    64-75  0xb00000
 * 8 Port Serial I/O board      ISIO    4    76-83  0x960000
 * Disk Controller              WFC     3    119    0xb01000
 * SCSI Controller              ISCSI   4    119    0xa00000
 * Slot 1 Controller Board      ASCU    7    31     0xb02000
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Finish 3 x ACIA6850, host and remote interface left, terminal works
 *  - Finish 1 x 68230 Motorola, Parallel Interface / Timer as required by ROM
 *    - Configure PIT to the Centronics device printer interface as
 *      supported by ROM (DONE)
 *  - Add 1 x Abort Switch
 *  - Add 1 x Reset Switch
 *  - Add 1 x Halt LED
 *  - Add a jumper field device as supported by PCB
 *  - Add configurable serial connector between ACIA:s and
 *    - Real terminal emulator, ie rs232 "socket"
 *    - Debug console
 *  - Add VME bus driver
 *
 ****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mm58167.h"
#include "machine/68230pit.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/clock.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_fcisio.h"
#include "bus/vme/vme_fcscsi.h"

/*
 * The baudrate on the Force68k CPU-1 to CPU-6 is generated by a Motorola 14411 bitrate generator
 * The CPU-6 documents matches the circuits that I could find on the CPU-1 board.
 *
 * From the documents:
 *
 * 3 RS232C interfaces, strap selectable baud rate from 110-9600 or 600-19200 baud (CPU-1B datasheet)
 *  or
 * 3 RS232C interfaces, strap selectable baud rate from 60-9600 or 240-38400 baud (CPU-6 Users manual)
 *
 * Default Jumper Settings of B7:
 * --------------------------------
 * GND           10 - 11 RSA input on 14411
 * F1 on 14411    1 - 20 Baud selector of the terminal port
 * F1 on 14411    3 - 18 Baud selector of the host port
 * F1 on 14411    5 - 16 Baud selector of the remote port
 *
 * The RSB input on the 14411 is kept high always so RSA=0, RSB=1 and a 1.8432MHz crystal
 * generates 153600 on the F1 output pin which by default strapping is connected to all
 * three 6850 acias on the board. These can be strapped separatelly to speedup downloads.
 *
 * The selectable outputs from 14411, F1-F16:
 * X16 RSA=0,RSB=1: 153600, 115200, 76800, 57600, 38400, 28800, 19200, 9600, 4800, 3200, 2153.3, 1758.8, 1200, 921600, 1843000
 * X64 RSA=1,RSB=1: 614400, 460800, 307200, 230400, 153600, 115200, 76800, 57600, 38400, 28800, 19200, 9600, 4800, 921600, 1843000
 *
 * However, the datasheet says baudrate is strapable for 110-9600 but the output is 153600
 * so the system rom MUST setup the acia to divide by 16 to generate the correct baudrate.
 *
 * There are multiple ways to achieve some of the baud rates and we have only seen a CPU-6 users manual so
 * emulation mimics a CPU-6 board at the moment until further information has been gathered.
 */

//**************************************************************************
//  CONFIGURABLE LOGGING
//**************************************************************************
//#define LOG_GENERAL (1U <<  0) // defined in logmacro.h
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

// #define LOG(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__) // defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class force68k_state : public driver_device
{
public:
	force68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_pit(*this, "pit")
		, m_brg(*this, "brg")
		, m_aciahost(*this, "aciahost")
		, m_aciaterm(*this, "aciaterm")
		, m_aciaremt(*this, "aciaremt")
		, m_centronics(*this, "centronics")
		, m_centronics_ack(0)
		, m_centronics_busy(0)
		, m_centronics_perror(0)
		, m_centronics_select(0)
		, m_serial_brf(*this, "SERIAL_BRF")
		, m_serial_p3(*this, "SERIAL_P3")
		, m_serial_p4(*this, "SERIAL_P4")
		, m_serial_p5(*this, "SERIAL_P5")
		, m_cart(*this, "exp_rom1")
	{
	}

	DECLARE_READ16_MEMBER (bootvect_r);
	DECLARE_READ16_MEMBER (vme_a24_r);
	DECLARE_WRITE16_MEMBER (vme_a24_w);
	DECLARE_READ16_MEMBER (vme_a16_r);
	DECLARE_WRITE16_MEMBER (vme_a16_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;

	// Clocks
	void write_acia_clocks(int id, int state);
	DECLARE_WRITE_LINE_MEMBER (write_f1_clock){ write_acia_clocks(mc14411_device::TIMER_F1, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f3_clock){ write_acia_clocks(mc14411_device::TIMER_F3, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f5_clock){ write_acia_clocks(mc14411_device::TIMER_F5, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f7_clock){ write_acia_clocks(mc14411_device::TIMER_F7, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f8_clock){ write_acia_clocks(mc14411_device::TIMER_F8, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f9_clock){ write_acia_clocks(mc14411_device::TIMER_F9, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f11_clock){ write_acia_clocks(mc14411_device::TIMER_F11, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f13_clock){ write_acia_clocks(mc14411_device::TIMER_F13, state); }
	DECLARE_WRITE_LINE_MEMBER (write_f15_clock){ write_acia_clocks(mc14411_device::TIMER_F15, state); }

	// Centronics printer interface
	DECLARE_WRITE_LINE_MEMBER (centronics_ack_w);
	DECLARE_WRITE_LINE_MEMBER (centronics_busy_w);
	DECLARE_WRITE_LINE_MEMBER (centronics_perror_w);
	DECLARE_WRITE_LINE_MEMBER (centronics_select_w);

	// User EPROM/SRAM slot(s)
	image_init_result force68k_load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER (exp1_load) { return force68k_load_cart(image, m_cart); }
	DECLARE_READ16_MEMBER (read16_rom);

	void fccpu1_eprom_sockets(machine_config &config);
	void fccpu1(machine_config &config);
private:
	required_device<cpu_device> m_maincpu;
	required_device<mm58167_device> m_rtc;
	required_device<pit68230_device> m_pit;
	required_device<mc14411_device> m_brg;
	required_device<acia6850_device> m_aciahost;
	required_device<acia6850_device> m_aciaterm;
	required_device<acia6850_device> m_aciaremt;
	optional_device<centronics_device> m_centronics;

	int32_t m_centronics_ack;
	int32_t m_centronics_busy;
	int32_t m_centronics_perror;
	int32_t m_centronics_select;

	// fake inputs for hardware configuration and things that need rewiring
	required_ioport             m_serial_brf;
	required_ioport             m_serial_p3;
	required_ioport             m_serial_p4;
	required_ioport             m_serial_p5;

	// Pointer to System ROMs needed by bootvect_r
	uint16_t  *m_sysrom;
	uint16_t  *m_usrrom;

	required_device<generic_slot_device> m_cart;
};

static ADDRESS_MAP_START (force68k_mem, AS_PROGRAM, 16, force68k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Vectors mapped from System EPROM */
	AM_RANGE (0x000008, 0x01ffff) AM_RAM /* DRAM CPU-1B */
//AM_RANGE (0x020000, 0x07ffff) AM_RAM /* Additional DRAM CPU-1D */
	AM_RANGE (0x080000, 0x083fff) AM_ROM /* System EPROM Area 16Kb DEBUGGER supplied as default on CPU-1B/D     */
	AM_RANGE (0x084000, 0x09ffff) AM_ROM /* System EPROM Area 112Kb additional space for System ROM     */
//AM_RANGE (0x0a0000, 0x0bffff) AM_ROM /* User EPROM/SRAM Area, max 128Kb mapped by a cartslot  */
	AM_RANGE (0x0c0040, 0x0c0043) AM_DEVREADWRITE8 ("aciahost", acia6850_device, read, write, 0x00ff)
	AM_RANGE (0x0c0080, 0x0c0083) AM_DEVREADWRITE8 ("aciaterm", acia6850_device, read, write, 0xff00)
	AM_RANGE (0x0c0100, 0x0c0103) AM_DEVREADWRITE8 ("aciaremt", acia6850_device, read, write, 0x00ff)
	AM_RANGE (0x0c0400, 0x0c042f) AM_DEVREADWRITE8 ("rtc", mm58167_device, read, write, 0x00ff)
	AM_RANGE (0x0e0000, 0x0e0035) AM_DEVREADWRITE8 ("pit", pit68230_device, read, write, 0x00ff)
//AM_RANGE(0x0e0200, 0x0e0380) AM_READWRITE(fpu_r, fpu_w) /* optional FPCP 68881 FPU interface */
	AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) */
	AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (force68k)

	PORT_START("SERIAL_BRF")
	PORT_CONFNAME(0x80 , 0x00 , "Baud Rate Factor") // RSA pin on MC14411
	PORT_CONFSETTING(0x00, "1x (Lo)")
	PORT_CONFSETTING(0x80, "4x (Hi)")

	PORT_START("SERIAL_P3")
	PORT_CONFNAME(0x0F , 0x00 , "P3 Host Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")
	PORT_CONFSETTING(mc14411_device::TIMER_F15, "60/240")

	PORT_START("SERIAL_P4")
	PORT_CONFNAME(0x0F , 0x00 , "P4 Terminal Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")
	PORT_CONFSETTING(mc14411_device::TIMER_F15, "60/240")

	PORT_START("SERIAL_P5")
	PORT_CONFNAME(0x0F , 0x00 , "P5 Remote Baud Lo/Hi") // F1-Fx pins on MC14411
	PORT_CONFSETTING(mc14411_device::TIMER_F1,  "9600/38400") // RSA=1x/16x
	PORT_CONFSETTING(mc14411_device::TIMER_F3,  "4800/19200")
	PORT_CONFSETTING(mc14411_device::TIMER_F5,  "2400/9600")
	PORT_CONFSETTING(mc14411_device::TIMER_F7,  "1200/4800")
	PORT_CONFSETTING(mc14411_device::TIMER_F8,  "600/2400")
	PORT_CONFSETTING(mc14411_device::TIMER_F9,  "300/1200")
	PORT_CONFSETTING(mc14411_device::TIMER_F11, "150/600")
	PORT_CONFSETTING(mc14411_device::TIMER_F13, "110/440")
	PORT_CONFSETTING(mc14411_device::TIMER_F15, "60/240")

INPUT_PORTS_END

/*
 *  Centronics support
 *
 *  The system ROMs has support for a parallel printer interface but the signals are just routed to row A
 *  of the VME P2 connector so no on board Centronics connector is available but assumed to be added on a
 *  separate I/O board. After some detective work I found that the ROM works as follows:
 *
 *  The 'PA' (Printer Attach) command issues a <cr> on Port A and sends a strobe on H2 it then loops over
 *  the select signal, bit 0 on Port B, and the ack signal on HS1, both to be non zero. The support is really
 *  flawed as the strobe signal goes high instead of low ( this might assume an inverting driver on the
 *  P2 board ) and the busy signal is not checked at all. Or I might have assumed it all wrong, but it now
 *  works with the generic centronics printer driver. Need the printer board documentation to improve further.
 *
 *  When the 'PA' command is successful everything printed to screen is mirrored on the printer. Use the
 *  'NOPA' command to stop mirroring. I had no printer ROMs so could not test it with a "real" printer.
 *
 *  Force CPU-1 init sequence for MC68230 PIT
 *  -----------------------------------------
 *  0801E6 0E0000 W 00 -> PGCR  Mode 0 (uni8), H34 dis, H12 dis, H1234 HZ
 *  0801E6 0E0002 W 00 -> PSRR  PC4, PC5, H1S>H2S>H3S>H4S
 *  0801E6 0E0004 W FF -> PADDR Port A all Outputs
 *  0801E6 0E0006 W 00 -> PBDDR Port B all Inputs
 *  0801EA 0E000C W 60 -> PACR  Port A Mode 01, pin def, dbfr H1 data rec, H2 status/int, H2 output neg, H2S clrd
 *  0801F0 0E000E W A0 -> PBCR  Port B mode 1x, H4 output neg, H4S clrd, H3 int dis, H3 edg input, H3S set by assrt edg
 *  0801F6 0E0000 W 30 -> PGCR  H34 enable, H12enable
 *  0801FC 0E000E W A8 -> PBCR  +H4 asserted
 *  08020A 0E000E W A0 -> PBCR  +H4 negated
 *
 *  Upon PA (Printer Attach) command enabling the Centronics printer mode
 *  ---------------------------------------------------------------------
 *  081DB4 0E0011 W D0 -> PADR  Data to Port A
 *  081DB8 0E000D W 68 -> PACR  H2 output asserted Centronics Strobe
 *  081DC0 0E000D W 60 -> PACR  H2 output negated
 *  081DD0 0E0013 R 00 <- PBDR  Port B polled for 01 (data) & 03 (mask)
 *
 */

/* Centronics ACK handler
 * The centronics ack signal is expected by the ROM to arrive at H1 input line
 */
WRITE_LINE_MEMBER (force68k_state::centronics_ack_w)
{
		LOG("%s(%d)\n", FUNCNAME, state);
		m_centronics_ack = state;
		m_pit->h1_set (state);
}

/* Centronics BUSY handler
 * The centronics busy signal is not used by the ROM driver afaik
 */
WRITE_LINE_MEMBER (force68k_state::centronics_busy_w){
		LOG("%s(%d)\n", FUNCNAME, state);
		m_centronics_busy = state;
}

/* Centronics PERROR handler
 * The centronics perror signal is not used by the ROM driver afaik
 */
WRITE_LINE_MEMBER (force68k_state::centronics_perror_w){
		LOG("%s(%d)\n", FUNCNAME, state);
		m_centronics_perror = state;
}

/* Centronics SELECT handler
 * The centronics select signal is expected by the ROM on Port B bit 0
 */
WRITE_LINE_MEMBER (force68k_state::centronics_select_w){
		LOG("%s(%d)\n", FUNCNAME, state);
		m_centronics_select = state;
		m_pit->portb_setbit (0, state);
}

/* Start it up */
void force68k_state::machine_start ()
{
	LOG("%s\n", FUNCNAME);

	save_item (NAME (m_centronics_busy));
	save_item (NAME (m_centronics_ack));
	save_item (NAME (m_centronics_select));
	save_item (NAME (m_centronics_perror));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint16_t*)(memregion ("maincpu")->base () + 0x080000);

	/* Map user ROM/RAM socket(s) */
	if (m_cart->exists())
	{
		m_usrrom = (uint16_t*)m_cart->get_rom_base();
#if 0 // This should be the correct way but produces odd and even bytes swapped
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa0000, 0xbffff, read16_delegate(FUNC(generic_slot_device::read16_rom), (generic_slot_device*)m_cart));
#else // So we installs a custom very ineffecient handler for now until we understand hwp to solve the problem better
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xa0000, 0xbffff, read16_delegate(FUNC(force68k_state::read16_rom), this));
#endif
	}
}

/* Reset it */
void force68k_state::machine_reset ()
{
	LOG("%s\n", FUNCNAME);

	// Set up the BRG divider. RSA is a jumper setting and RSB is always set High
	m_brg->rsa_w( m_serial_brf->read() == 0x80 ? ASSERT_LINE : CLEAR_LINE );
	m_brg->rsb_w( ASSERT_LINE);

	// Disable all configured timers, only enabling the used ones
	m_brg->timer_disable_all();
	m_brg->timer_enable((mc14411_device::timer_id) m_serial_p3->read(), true);
	m_brg->timer_enable((mc14411_device::timer_id) m_serial_p4->read(), true);
	m_brg->timer_enable((mc14411_device::timer_id) m_serial_p5->read(), true);
}

/* A very ineffecient User cart emulation of two 8 bit sockets (odd and even) */
READ16_MEMBER (force68k_state::read16_rom){
	offset = offset % m_cart->common_get_size("rom"); // Don't read outside buffer...
	return ((m_usrrom [offset] << 8) & 0xff00) | ((m_usrrom [offset] >> 8) & 0x00ff);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0x80000 to 0x0 */
READ16_MEMBER (force68k_state::bootvect_r){
	return m_sysrom [offset];
}

/* 10. The VMEbus (text from board documentation)
 * ---------------
 * The implemented VMEbus Interface includes 24 address, 16 data,
 * 6 address modifier and the asynchronous control signals.
 * A single level bus arbiter is provided to build multi master
 * systems. In addition to the bus arbiter, a separate slave bus
 * arbitration allows selection of the arbitration level (0-3).
 *
 * The address modifier range .,Short 110 Access can be selected
 * via a jumper for variable system generation. The 7 interrupt
 * request levels of the VMEbus are fully supported from the
 * SYS68K1CPU-1 B/D. For multi-processing, each IRQ signal can be
 * enabled/disabled via a jumper field.
 *
 * Additionally, the SYS68K1CPU-1 B/D supports the ACFAIL, SYSRESET,
 * SYSFAIL and SYSCLK signal (16 MHz).
 */

/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (force68k_state::vme_a24_r){
		LOG("%s\n", FUNCNAME);
		return (uint16_t) 0;
}

WRITE16_MEMBER (force68k_state::vme_a24_w){
		LOG("%s\n", FUNCNAME);
}

READ16_MEMBER (force68k_state::vme_a16_r){
		LOG("%s\n", FUNCNAME);
		return (uint16_t) 0;
}

WRITE16_MEMBER (force68k_state::vme_a16_w){
		LOG("%s\n", FUNCNAME);
}

/*
 * Serial port clock sources can all be driven by different or the same output(s) of the MC14411
 */
void force68k_state::write_acia_clocks(int id, int state)
{
	if (id == m_serial_p3->read())
	{
		m_aciahost->write_txc(state);
		m_aciahost->write_rxc(state);
	}
	if (id == m_serial_p4->read())
	{
		m_aciaterm->write_txc(state);
		m_aciaterm->write_rxc(state);
	}
	if (id == m_serial_p5->read())
	{
		m_aciaremt->write_txc(state);
		m_aciaremt->write_rxc(state);
	}
}

/*
 * 4. The USER Area (Text from the board manual)
  The USER area contains two 28 pin sockets with JEDEC compatible pin out.
   To allow the usage of static RAM's, the access to the USER area is byte
   oriented. Table 3. lists the usable device types.

   Bits   Bytes    EPROM SRAM
   --------------------------
   2Kx16   4 Kbyte 2716  6116
   4Kx16   8 Kbyte 2732
   8Kx16  16 Kbyte 2764  6264
   16Kx16 32 Kbyte 27128
   32Kx16 64 Kbyte 27256
   --------------------------
*/
// Implementation of static 2 x 64K EPROM in sockets J10/J11 as 16 bit wide cartridge for easier
// software handling. TODO: make configurable according to table above.
MACHINE_CONFIG_START(force68k_state::fccpu1_eprom_sockets)
	MCFG_GENERIC_CARTSLOT_ADD("exp_rom1", generic_plain_slot, "fccpu1_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
	MCFG_GENERIC_ENDIAN(ENDIANNESS_BIG)
	MCFG_GENERIC_LOAD(force68k_state, exp1_load)
//  MCFG_SOFTWARE_LIST_ADD("cart_list", "fccpu1_cart")
MACHINE_CONFIG_END

/***************************
   Rom loading functions
****************************/
image_init_result force68k_state::force68k_load_cart(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	if (size > 0x20000) // Max 128Kb
	{
		LOG("Cartridge size exceeding max size (128Kb): %d\n", size);
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Cartridge size exceeding max size (128Kb)");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


static SLOT_INTERFACE_START(fccpu1_vme_cards)
	SLOT_INTERFACE("fcisio", VME_FCISIO1)
	SLOT_INTERFACE("fcscsi", VME_FCSCSI1)
SLOT_INTERFACE_END

/*
 * Machine configuration
 */
MACHINE_CONFIG_START(force68k_state::fccpu1)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68000, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP (force68k_mem)

	/* P3/Host Port config
	 * LO command causes ROM monitor to expect S-records on HOST port by default
	 * Implementation through nullmodem currently does not support handshakes so
	 * the ROM momitor is over-run while checking for checksums etc if used with
	 * UI mount <file> feature.
	 */
	MCFG_DEVICE_ADD ("aciahost", ACIA6850, 0)

	MCFG_ACIA6850_TXD_HANDLER (DEVWRITELINE ("rs232host", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER (DEVWRITELINE ("rs232host", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232host", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("aciahost", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("aciahost", acia6850_device, write_cts))

	/* P4/Terminal Port config */
	MCFG_DEVICE_ADD ("aciaterm", ACIA6850, 0)

	MCFG_ACIA6850_TXD_HANDLER (DEVWRITELINE ("rs232trm", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER (DEVWRITELINE ("rs232trm", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("aciaterm", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("aciaterm", acia6850_device, write_cts))

	/* P5/Remote Port config */
	MCFG_DEVICE_ADD ("aciaremt", ACIA6850, 0)

	/* Bit Rate Generator */
	MCFG_MC14411_ADD ("brg", XTAL_1_8432MHz)
	MCFG_MC14411_F1_CB(WRITELINE (force68k_state, write_f1_clock))
	MCFG_MC14411_F3_CB(WRITELINE (force68k_state, write_f3_clock))
	MCFG_MC14411_F5_CB(WRITELINE (force68k_state, write_f5_clock))
	MCFG_MC14411_F7_CB(WRITELINE (force68k_state, write_f7_clock))
	MCFG_MC14411_F8_CB(WRITELINE (force68k_state, write_f8_clock))
	MCFG_MC14411_F9_CB(WRITELINE (force68k_state, write_f9_clock))
	MCFG_MC14411_F11_CB(WRITELINE (force68k_state, write_f11_clock))
	MCFG_MC14411_F13_CB(WRITELINE (force68k_state, write_f13_clock))
	MCFG_MC14411_F15_CB(WRITELINE (force68k_state, write_f15_clock))

	/* RTC Real Time Clock device */
	MCFG_DEVICE_ADD ("rtc", MM58167, XTAL_32_768kHz)

	/* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	MCFG_DEVICE_ADD ("pit", PIT68230, XTAL_16MHz / 2)
	MCFG_PIT68230_PA_OUTPUT_CB (DEVWRITE8 ("cent_data_out", output_latch_device, write))
	MCFG_PIT68230_H2_CB (DEVWRITELINE ("centronics", centronics_device, write_strobe))

	// Centronics
	MCFG_CENTRONICS_ADD ("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER (WRITELINE (force68k_state, centronics_ack_w))
	MCFG_CENTRONICS_BUSY_HANDLER (WRITELINE (force68k_state, centronics_busy_w))
	MCFG_CENTRONICS_PERROR_HANDLER (WRITELINE (force68k_state, centronics_perror_w))
	MCFG_CENTRONICS_SELECT_HANDLER (WRITELINE (force68k_state, centronics_select_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD ("cent_data_out", "centronics")

	// EPROM sockets
	MCFG_FRAGMENT_ADD(fccpu1_eprom_sockets)

	// VME interface
	MCFG_VME_DEVICE_ADD("vme")
	MCFG_VME_SLOT_ADD ("vme", 1, fccpu1_vme_cards, nullptr)
MACHINE_CONFIG_END

#if 0 /*
       * CPU-6 family is device and adressmap compatible with CPU-1 but with additions
       * such as an optional 68881 FPU
       */
MACHINE_CONFIG_START (force68k_state::fccpu6)
	MCFG_CPU_ADD ("maincpu", M68000, XTAL_8MHz)         /* Jumper B10 Mode B */
	MCFG_CPU_PROGRAM_MAP (force68k_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START (force68k_state::fccpu6a)
	MCFG_CPU_ADD ("maincpu", M68000, XTAL_12_5MHz)        /* Jumper B10 Mode A */
	MCFG_CPU_PROGRAM_MAP (force68k_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START (force68k_state::fccpu6v)
	MCFG_CPU_ADD ("maincpu", M68010, XTAL_8MHz)         /* Jumper B10 Mode B */
	MCFG_CPU_PROGRAM_MAP (force68k_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START (force68k_state::fccpu6va)
	MCFG_CPU_ADD ("maincpu", M68010, XTAL_12_5MHz)        /* Jumper B10 Mode A */
	MCFG_CPU_PROGRAM_MAP (force68k_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START (force68k_state::fccpu6vb)
	MCFG_CPU_ADD ("maincpu", M68010, XTAL_12_5MHz)        /* Jumper B10 Mode A */
	MCFG_CPU_PROGRAM_MAP (force68k_mem)
MACHINE_CONFIG_END
#endif

/* ROM definitions */
ROM_START (fccpu1)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_DEFAULT_BIOS("forcemon-1.0l")

ROM_SYSTEM_BIOS(0, "forcemon-1.0l", "Force Computers SYS68K/CPU-1 Force Monitor 1.0L")
ROMX_LOAD ("fccpu1V1.0L.j8.bin", 0x080001, 0x2000, CRC (3ac6f08f) SHA1 (502f6547b508d8732bd68bbbb2402d8c30fefc3b), ROM_SKIP(1) | ROM_BIOS(1))
ROMX_LOAD ("fccpu1V1.0L.j9.bin", 0x080000, 0x2000, CRC (035315fb) SHA1 (90dc44d9c25d28428233e6846da6edce2d69e440), ROM_SKIP(1) | ROM_BIOS(1))

ROM_SYSTEM_BIOS(1, "forcebug-1.1", "Force Computers SYS68K/CPU-1 Force Debugger 1.1")
ROMX_LOAD ("fccpu1V1.1.j8.bin", 0x080001, 0x4000, CRC (116dcbf0) SHA1 (6870b71606933f84afe27ad031c651d201b93f99), ROM_SKIP(1) | ROM_BIOS(2))
ROMX_LOAD ("fccpu1V1.1.j9.bin", 0x080000, 0x4000, CRC (aefd5b0b) SHA1 (1e24530a6d5dc4fb77fde67acae08d371e59fc0f), ROM_SKIP(1) | ROM_BIOS(2))

/*
 * System ROM terminal commands
 *
 * COMMAND SUMMARY DESCRIPTION (From CPU-1B datasheet, ROMs were dumped
 * from a CPU-1 board so some features might be missing or different)
 * ---------------------------------------------------------------------------
 * BF <address1> <address2> <data> <CR>        Block Fill memory - from addr1 through addr2 with data
 * BM <address1> <address2> <address 3> <CR>   Block Move  - move from addr1 through addr2to addr3
 * BR [<address> [; <count>] ... ] <CR>        Set/display Breakpoint
 * BS <address1> <address2> <data> <CR>        Block Search - search addr1 through addr2 for data
 * BT <address1> <address2> <CR>               Block Test of memory
 * DC <expression> <CR>                        Data Conversion
 * DF <CR>                                     Display Formatted registers
 * DU [n] <address1> <address2>[<string>] <CR> Dump memory to object file
 * GO or G [<address] <CR>                     Execute program.
 * GD [<address] <CR>                          Go Direct
 * GT <address> <CR>                           Exec prog: temporary breakpoint
 * HE<CR>                                      Help; display monitor commands
 * LO [n] [;<options] <CR>                     Load Object file
 * MD <address> [<count>] <CR>                 Memory Display
 * MM or M <address> [<data<][;<options>] <CR> Memory Modify
 * MS <address> <data1 > <data2> < ... <CR>    Memory Set - starting at addr with data 1. data 2 ...
 * NOBR [<address> ... ] <CR>                  Remove Breakpoint
 * NOPA <CR>                                   Printer Detach (Centronics on PIT/P2)
 * OF <CR>                                     Offset
 * PA <CR>                                     Printer Attach (Centronics on PIT/P2)
 * PF[n] <CR>                                  Set/display Port Format
 * RM <CR>                                     Register Modify
 * TM [<exit character>] <CR>                  Transparent Mode
 * TR OR T [<count] <CR>                       Trace
 * TT <address> <CR>                           Trace: temporary breakpoint
 * VE [n] [<string] <CR>                       Verify memory/object file
 * ----------------------------------------------------------------------------
 * .AO - .A7 [<expression] <CR>                Display/set address register
 * .00 - .07 [<expression] <CR>                Display/set data register
 * .RO - .R6 [<expression] <CR>                Display/set offset register
 * .PC [<expression] <CR>                      Display/set program counter
 * .SR [<expression] <CR>                      Display/set status register
 * .SS [<expression] <CR>                      Display/set supervisor stack
 * .US [<expression] <CR>                      Display/set user stack
 * ----------------------------------------------------------------------------
 * MD <address> [<count>]; DI <CR>             Disassemble memory location
 * MM <address>; DI <CR>                       Disassemble/Assemble memory location
 * ----------------------------------------------------------------------------
 * Undocumented commands found in ROM table at address 0x80308
 * .*                                          No WHAT message displayed, no action seen.
 */
ROM_END

/*
 * CPU-6 ROMs were generally based om VMEPROM which contained the PDOS RTOS from Eyring Research.
 * I don't have these but if anyone can dump them and send to me I can verify that they work as expected.
 */
#if 0
ROM_START (fccpu6)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_END

ROM_START (fccpu6a)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_END

ROM_START (fccpu6v)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_END

ROM_START (fccpu6va)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_END

ROM_START (fccpu6vb)
ROM_REGION (0x1000000, "maincpu", 0)
ROM_END
#endif

/* Driver */
/*    YEAR  NAME      PARENT  COMPAT  MACHINE      INPUT     CLASS            INIT  COMPANY                  FULLNAME          FLAGS */
COMP( 1983, fccpu1,   0,      0,      fccpu1,      force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-1",   MACHINE_NO_SOUND_HW )
//COMP( 1989, fccpu6,   0,      0,      fccpu6,      force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-6",   MACHINE_IS_SKELETON )
//COMP( 1989, fccpu6a,  0,      0,      fccpu6a,     force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-6a",  MACHINE_IS_SKELETON )
//COMP( 1989, fccpu6v,  0,      0,      fccpu6v,     force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-6v",  MACHINE_IS_SKELETON )
//COMP( 1989, fccpu6va, 0,      0,      fccpu6va,    force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-6va", MACHINE_IS_SKELETON )
//COMP( 1989, fccpu6vb, 0,      0,      fccpu6vb,    force68k, force68k_state,  0,    "Force Computers GmbH",  "SYS68K/CPU-6vb", MACHINE_IS_SKELETON )
