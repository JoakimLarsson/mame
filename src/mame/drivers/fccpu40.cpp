// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-40 VME SBC drivers
 *
 *  01/11/2016
 *
 * Thanks to Al Kossow and his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 *
 *       ||
 * ||    ||  CPU-40 
 * ||||--||_____________________________________________________________
 * ||||--||      +--------------+ +--------------+                      |
 * ||    ||      | J42          | |J44           |                      |__
 *       ||      |              | |              |                     |   |
 * RST O===      |              | |              |                     |   |
 *       ||      |              | |              |                     |   |
 * ABT O===      |              | |              |                     |   |
 RUN/HLT C|      |              | |              |                     |   |
 BUS MST C|      +--------------+ +--------------+                     |   |
 *       ||                                                            |VME|
 *       ==                            +------------------+            |   |
 *  Ser  ==      +--------------+      |J41               |            |P1 |
 *   #4  ==      |J43           |      |                  |            |   |
 *       ==      |              |      |                  |            |   |
 *       ||      |              |      |                  |            |   |
 *       ==      |              |      |                  |            |   |
 *  Ser  ==      |              |      |                  |            |   |
 *   #3  ==      |              |      |                  |            |   |
 *       ==      +--------------+      +------------------+            |   |
 *       ||                                                            |___|
 *       ==                                                            |
 *  Ser  ==            +------------------------+                      |
 *   #2  ==            |J40                     |                      |
 *       ==            |                        |                      |
 *       ||            |      MC68040           |                      |
 *       ==            |                        |                      |
 *  Ser  ==            |                        |                      |
 *   #1  ==            |                        |                      |
 *       ==            |                        |                      |___
 *       ||            |                        |                      |   |
 * rot   [O            |                        |                      |   |
 * swtshs||            |                        |                      |   |
 *       [O            +------------------------+                      |   |
 *       ||                                                            |   |
 *       +|        +----------++----------+                            |VME|
 *        |        |J35       ||J36       |                            |   |
 * Eagle  |        |  PIT #1  ||  PIT #2  |                            |P2 |
 *        |        |          ||          |                            |   |
 *        |        |          ||          |                            |   |
 * board  |        +----------++----------+                            |   |
 *        |        |J24       |                                        |   |
 *        |        | DUSCC #2 |                                        |   |
 * module |        |          |                                        |   |
 *        |        |          |                                        |   |
 *        |        +----------+                                        |   |
 * front  |        |J19       |                                        |   |
 *       +|        | DUSCC #1 |                                        |___|
 * ||    ||        |          |                                          +
 * ||||--||        |          |                                          |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Force Computers
 *---------------------------
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * http://bitsavers.informatik.uni-stuttgart.de/pdf/forceComputers/CPU40/Force_SYS68K_CPU-40_41_Users_Manual_Feb92.pdf
 *
 * CPU-40 has the following feature set
 * ------------------------------------------------
 * - 68040 microprocessor: 25.0 MHz on CPU-40B/41B/x
 * - 68040 microprocessor: 33.0 MHz on CPU-40D/41D/x
 * - Shared DRAM Module: 
 *   - 4 Mbyte DRAM with Burst Read/Write and Parity Generation and Checking (DRM-01/4)
 *   - 16 Mbyte DRAM with Burst Read/Write and Parity Generation and Checking (DRM-01/16)
 * - Shared SRAM Module: 
 *   - 4 Mbyte SRAM with Burst Read/Write (SRM-01/4) 
 *   - 8 Mbyte SRAM with Burst Read/Write (SRM-01/8)
 * - 32-bit high speed DMA controller for data transfers to/from the shared RAM, VMEbus memory and
 *   EAGLE modules; DMA controller is installed in the FGA-002.
 * - Two system EPROM devices supporting 40-pin devices. Access from the 68040 using a 32-bit data path
 * - One boot EPROM for local booting, initialization of the I/O chips and configuration of the FGA-002
 * - 128 Kbyte SRAM with on-board battery backup
 * - 128 Kbyte FLASH EPROM
 * - FLXi interface for installation of one EAGLE module
 * - Four Serial I/O interfaces, configurable as RS232/RS422/RS485, available on the front panel
 * - 8-bit parallel interface with 4-bit handshake
 * - Two 24-bit timers with 5-bit prescaler
 * - One 8-bit timer
 * - Real Time Clock with calendar and on-board battery backup
 * - Full 32-bit VMEbus master/slave interface, supporting the following data transfer types:
 *   - A32, A24, A16 : D8, D16, D32 - Master
 *   - A32, A24 : D8, D16, D32 - Slave
 *   - UAT, RMW, ADO
 *   - Four-level VMEbus arbiter
 *   - SYSCLK driver
 *   - VMEbus interrupter (IR 1-7)
 *   - VMEbus interrupt handler (IH 1-7)
 *   - Support for ACFAIL* and SYSFAIL
 *   - Bus timeout counters for local and VMEbus access (15 ¦Ìsec)
 *   - VMEPROM, Real Time Multitasking Kernel with monitor, file manager and debugger
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
 * 00000000-00xFFFFF        Shared DRAM D8-D32 x=3:4MB x=7:8MB x=f:16Mb
 * 00y00000-F9FFFFFF        VME A32 D8-D32     y=x+1
 * FA000000-FAFFFFFF		Message Broadcast Area
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 * FD000000-FEFFFFFF        Reserved
 * FF000000-FF7FFFFF        System EPROM
 * FF800000-FF800BFF        Reserved
 * FF800C00-FF800DFF        PIT1 D8 (68230)
 * FF800E00-FF800FFF        PIT2 D8 (68230)
 * FF801000-FF801FFF        Reserved
 * FF802000-FF8021FF        DUSCC1 D8 (SCN68562)
 * FF802200-FF8023FF        DUSCC2 D8 (SCN68562)
 * FF802400-FF802FFF        Reserved
 * FF803000-FF8031FF        RTC (72423) D8
 * FF803200-FF8033FF        Reserved
 * FFC00000-FFCFFFFF        Local SRAM
 * FFD00000-FFDFFFFF        FGA-002 Gate Array D8-D32
 * FFE00000-FFEFFFFF        Boot PROM D8-D32
 * FFF00000-FFFFFFFF        Reserved
 * --------------------------------------------------------------------------
 *
 * PIT #1 hardware wiring
 * ----------------------------------------------------------
 * PA0-PA3  Rotary Switch 1 input
 * PA4-PA7  Rotary Switch 2 input
 * H1-H4    Reserved
 * PB0-PB7  A24-A31 Control for Slave Access  
 * PC0-PC2  Reserved
 * PC3      Timer interrupt request
 * PC4      Lock Cycles
 * PC5-PC7  Reserved
 *
 * PIT #2 hardware setup wiring
 * ----------------------------------------------------------
 * PA0-PA7  User I/O via optional B12
 * H1-H4    User I/O via optional B12
 * PB0-PB2  Memory Size
 * PB3-PB7  Board ID
 * PC0		MODLOW
 * PC1  	Reserved
 * PC2      RAMTYP
 * PC3      Timer Interrupt request
 * PC4      BURST
 * PC5      Port IRQ
 * PC6      PARITY
 * PC7      ENA24
 *
 *
 *  TODO:
 *  - Add RTC
 *  - Add VMEbus arbiter register
 *  - Write VME device
 *  - Add variants of boards
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/scnxx562.h"
#include "machine/68230pit.h"
#include "machine/fga002.h"
#include "machine/nvram.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
//#include "machine/timekpr.h"

#define VERBOSE 2

#define LOGPRINT(x)  { do { if (VERBOSE) logerror x; } while (0); }
#define LOG(x) LOGPRINT(x)
#define LOGR(x)
#define LOGSETUP(x) LOGPRINT(x)
#define LOGINT(x) LOGPRINT(x)
#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define DUSCC_CLOCK XTAL_14_7456MHz /* Needs verification */

class fccpu40_state : public driver_device
{
public:
fccpu40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
		, m_dusccterm(*this, "duscc")
		, m_pit1 (*this, "pit1")
		, m_pit2 (*this, "pit2")
		, m_fga002 (*this, "fga002")
	{
	}

	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);

	/* Interrupt  support */
	//  IRQ_CALLBACK_MEMBER(maincpu_iack_callback);
	DECLARE_WRITE_LINE_MEMBER(fga_irq_callback);
	uint8_t fga_irq_state;
	//  int fga_irq_vector;
	int fga_irq_level;

	/* Rotary switch PIT input */
	DECLARE_READ8_MEMBER (rotary_rd);
	DECLARE_READ8_MEMBER (flop_dmac_r);
	DECLARE_WRITE8_MEMBER (flop_dmac_w);
	DECLARE_READ8_MEMBER (pit1c_r);
	DECLARE_WRITE8_MEMBER (pit1c_w);
	DECLARE_READ8_MEMBER (pit2a_r);
	DECLARE_WRITE8_MEMBER (pit2a_w);
	DECLARE_READ8_MEMBER (board_mem_id_rd);
	DECLARE_READ8_MEMBER (pit2c_r);
	DECLARE_WRITE8_MEMBER (pit2c_w);

	/* VME bus accesses */
	//DECLARE_READ16_MEMBER (vme_a24_r);
	//DECLARE_WRITE16_MEMBER (vme_a24_w);
	//DECLARE_READ16_MEMBER (vme_a16_r);
	//DECLARE_WRITE16_MEMBER (vme_a16_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;
protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;

	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;

	required_device<fga002_device> m_fga002;

	// Helper functions
	void update_irq_to_maincpu();

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t  *m_sysrom;
	uint32_t  m_sysram[2];
};

static ADDRESS_MAP_START (fccpu40_mem, AS_PROGRAM, 32, fccpu40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* 4 Mb RAM */
	AM_RANGE (0xff000000, 0xff7fffff) AM_ROM AM_REGION("roms", 0x000000)
	AM_RANGE (0xff800c00, 0xff800dff) AM_DEVREADWRITE8("pit1", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xff800e00, 0xff800fff) AM_DEVREADWRITE8("pit2", pit68230_device, read, write, 0xffffffff)
	AM_RANGE (0xff802000, 0xff8021ff) AM_DEVREADWRITE8("duscc", duscc68562_device, read, write, 0xffffffff) /* Port 1&2 - Dual serial port DUSCC   */
	AM_RANGE (0xff802200, 0xff8023ff) AM_DEVREADWRITE8("duscc2", duscc68562_device, read, write, 0xffffffff) /* Port 3&4 - Dual serial port DUSCC   */
//	AM_RANGE (0xff803000, 0xff80300f) AM_DEVREADWRITE8("rtc", ep72423_device, read, write, 0x0f0f0f0f) /* Port 3&4 - Dual serial port DUSCC   */
//	AM_RANGE (0xff803e00, 0xff803fff) AM_READWRITE(arbiter_r, arbiter_w) /* VMEbus arbiter */
	AM_RANGE (0xffc00000, 0xffc7ffff) AM_RAM AM_SHARE ("nvram") /* On-board SRAM with battery backup (nvram) */
//	AM_RANGE (0xffc80000, 0xffcfffff) AM_ROM AM_REGION("roms", 0x800000) /* Local Flash EPROM */
	AM_RANGE (0xffd00000, 0xffdfffff) AM_DEVREADWRITE8("fga002", fga002_device, read, write, 0xffffffff)  /* FGA-002 Force Gate Array */
	AM_RANGE (0xffe00000, 0xffefffff) AM_ROM AM_REGION("roms", 0x800000)

	//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
	//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (fccpu40)
INPUT_PORTS_END

/* Start it up */
void fccpu40_state::machine_start ()
{
	LOG(("--->%s\n", FUNCNAME));

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

void fccpu40_state::machine_reset ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base () + 0x800000);
}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (fccpu40_state::bootvect_r){
	return m_sysrom[offset];
}

WRITE32_MEMBER (fccpu40_state::bootvect_w){
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

/*
 * Rotary Switches - to configure the board
 *
 * Table 25: PI/T #1 Interface Signals
 * Pin     Function  In/Out
 * PA0-PA3   SW1      In
 * PA4 PA7   SW2      In
 *
 * Table 38: Upper Rotary Switch (SW2)
 * Bit 3: This bit indicates whether the RAM disk should be initialized after reset. If this bit is set to "0" (settings 0-7),
 *  the RAM disk is initialized as defined by bit 0 and 1. When the disk is initialized, all data on the disk is lost.
 * Bit 2: This bit defines the default data size on the VMEbus. If the bit is set to "0", 16 bits are selected, if it is set
 *  to "1", 32 bits are selected.
 * Bit 1 and Bit 0: These two bits define the default RAM disk. See Table 40, "RAM Disk Usage," a detailed description.
 *  If AUTOBOOT is set by bit 2 and 3 of SW1, bit 1 and 0 of SW2 define which operating system will be booted. See Table 42,
 *  "Boot an Operating System (if AUTOBOOT is selected)," on page 129 for detailed description.
 *
 * Table 39: Lower Rotary Switch (SW1)
 * Bit 3 and Bit 2: These two bits define which program is to be invoked after reset. Please refer
 *  to Table 41, "Program After Reset," on page 129 for a detailed description.
 * Bit 1: If this switch is "0" (settings 0,1,4,5,8,9,C,D), VMEPROM tries to execute a start-up file after reset. The default
 *  filename is SY$STRT. If the bit is "1", VMEPROM comes up with the default banner.
 * Bit 0: If this switch is set to "0" (settings 0,2,4,6,8,A,C,E), VMEPROM checks the VMEbus for available hardware after reset.
 *  In addition VMEPROM waits for SYSFAIL to disappear from the VMEbus. The following hardware can be detected:
 *  - Contiguous memory
 *  - ASCU-1/2
 *  - ISIO-1/2
 *  - SIO-1/2
 *  - ISCSI-1
 *  - WFC-1
 *
 * Table 40: RAM Disk Usage
 * Bit 1 Bit 0 Upper Switch (SW 2) selected on
 *  1     1     RAM DISK AT TOP OF MEMORY (32 Kbytes) 3,7,B,F
 *  1     0     RAM DISK AT 0xFC80 0000 (512 Kbytes) 2,6,A,E
 *  0     1     RAM DISK AT 0x4070 0000 (512 Kbytes) 1,5,9,D
 *  0     0     RAM DISK AT 0x4080 0000 (512 Kbytes) 0,4,8,C
 *
 * Table 41: Program After Reset
 * Bit 3 Bit 2 Lower Switch (SW 1)          selected on
 *  1     1     VMEPROM                         C,D,E,F
 *  1     0     USER PROGRAM AT 0x4070 0000     8,9,A,B
 *  0     1     AUTOBOOT SYSTEM                 4,5,6,7
 *  0     0     USER PROGRAM AT 4080.000016     0,1,2,3
 *
 * Table 42: Boot an Operating System (if AUTOBOOT is selected)
 * Bit 1 Bit 0 Upper Switch (SW 2)          selected on
 *  1     1     reserved                        3,7,B,F
 *  1     0     Boot UNIX/PDOS 4.x              2,6,A,E
 *  0     1     Boot another operating system   1,5,9,D
 *  0     0     Setup for UNIX mailbox driver   0,4,8,C
 *
 * "To start VMEPROM, the rotary switches must both be set to 'F':" Hmm...
 */
READ8_MEMBER (fccpu40_state::rotary_rd){
	LOG(("%s\n", FUNCNAME));
	return 0xff; // TODO: make this configurable from commandline or artwork
}

// PIT#1 Port B TODO: implement floppy and dma control
READ8_MEMBER (fccpu40_state::flop_dmac_r){
	LOG(("%s\n", FUNCNAME));
	return 0xff;
}

WRITE8_MEMBER (fccpu40_state::flop_dmac_w){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

// PIT#1 Port C TODO: implement timer+port interrupts and 68882 sense
// TODO: Connect PC0, PC1, PC4 and PC7 to B5 and/or P2 connector
READ8_MEMBER (fccpu40_state::pit1c_r){
	LOG(("%s\n", FUNCNAME));
	return 0xff;
}

WRITE8_MEMBER (fccpu40_state::pit1c_w){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

// PIT#2 Port A TODO: Connect to B5 and /or P2 connector
READ8_MEMBER (fccpu40_state::pit2a_r){
	LOG(("%s\n", FUNCNAME));
	logerror("Unsupported user i/o on PIT2 port A detected\n");
	return 0xff;
}

WRITE8_MEMBER (fccpu40_state::pit2a_w){
	LOG(("%s(%02x)\n", FUNCNAME, data));
	logerror("Unsupported user i/o on PIT2 port A detected\n");
}

/*
 * PI/T #2 Factory settings
 * B2 B1 B2 Shared Memory Size - From these lines, the on-board Shared RAM capacity can be read in by software.
 *  0  0  0 32 Mb
 *  0  0  1 16 Mb
 *  0  1  0  8 Mb
 *  0  1  1  4 Mb
 *  1  x  x  Reserved
 *
 * B7 B6 B5 B4 B3 Board ID(s) -  From these lines, the CPU board identification number can be read in by
 *  0  0  1  0  0 CPU-40/41  software. Every CPU board has a unique number. Different versions of
 * (fill in more)       one CPU board (i.e. different speeds, capacity of memory, or modules)
 *                      contain the same identification number. In the case of the CPU-40/41, the
 *                      number is 0x10 (shifted righ + 4Mb memory yields 0x83 
 */
READ8_MEMBER (fccpu40_state::board_mem_id_rd){
	LOG(("%s\n", FUNCNAME));
	// FGA prom subtracts 0x14 and shift the result three bits right to get the ID. 
	// ID:s checked for are: 
	// 0x15 (CPU-40/41) 0x1E, 0x21, 0x27 (bp 0xffe01e66)
	// 0x14                              (bp 0xffe01e76)
	// 0x28                              (bp 0xffe01e80)
	// default                           (bp 0xffe01e8a)
	return ((0x15 - 0x14) << 3) + 3; 
}

// PIT#2 Port C 
READ8_MEMBER (fccpu40_state::pit2c_r){
	LOG(("%s\n", FUNCNAME));
	return 0x0f;
}

WRITE8_MEMBER (fccpu40_state::pit2c_w){
	LOG(("%s(%02x)\n", FUNCNAME, data));
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (fccpu40_state::vme_a24_r){
	LOG (logerror ("vme_a24_r\n"));
	return (uint16_t) 0;
}

WRITE16_MEMBER (fccpu40_state::vme_a24_w){
	LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (fccpu40_state::vme_a16_r){
	LOG (logerror ("vme_16_r\n"));
	return (uint16_t) 0;
}

WRITE16_MEMBER (fccpu40_state::vme_a16_w){
	LOG (logerror ("vme_a16_w\n"));
}
#endif

WRITE_LINE_MEMBER(fccpu40_state::fga_irq_callback)
{
	LOGINT(("%s(%02x)\n", FUNCNAME, state));

	fga_irq_state = state;
	fga_irq_level = m_fga002->get_irq_level();
	LOGINT((" - FGA irq level  %02x\n", fga_irq_level));
	update_irq_to_maincpu();
}

void fccpu40_state::update_irq_to_maincpu()
{
	LOGINT(("%s()\n", FUNCNAME));
	LOGINT((" - fga_irq_level: %02x\n", fga_irq_level));
	LOGINT((" - fga_irq_state: %02x\n", fga_irq_state));
	switch (fga_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, fga_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, fga_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, fga_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, fga_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, fga_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, fga_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, fga_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fccpu40, fccpu40_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68040, XTAL_33MHz)
	MCFG_CPU_PROGRAM_MAP (fccpu40_mem)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("fga002", fga002_device, iack)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Terminal Port config */
	/* Force CPU40 series of boards has up to four serial ports, p1-p4, the FGA boot uses p4 as console and subsequent
	   firmware uses p1 as console and in an operating system environment there may be user login shells on the other.

	   In order to use more than just one terminal MAME supports serial socket servers to which it is possible to
	   connect a telnet terminal to. The general syntax to open a socket server from the command prompts is:

	   mame  fccpu40 -window -rs232p4 null_modem -bitbngr socket.127.0.0.1:1001

	   At the opening screen, before the board starts to execute code, start up the telnet client and give 127.0.0.1:1001 as host
	   It is also possible to enumerate more than one terminal server in order to have several terminal session attached.

	   mame  fccpu40 -window -rs232p4 null_modem -bitbngr1 socket.127.0.0.1:1001 -rs232p1 null_modem -bitbngr2 socket.127.0.0.1:1002

	   Now just start up the telnet clients with 127.0.0.1:1001 and 127.0.0.1:1002 as hosts and you have control of input for each port.
	*/

#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"
#define RS232P3_TAG      "rs232p3"
#define RS232P4_TAG      "rs232p4"

	MCFG_DUSCC68562_ADD("duscc", DUSCC_CLOCK, DUSCC_CLOCK, 0, DUSCC_CLOCK, 0 )
	/* Port 1 on Port B */
	MCFG_DUSCC_OUT_TXDB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSB_CB(DEVWRITELINE(RS232P1_TAG, rs232_port_device, write_rts))
	/* Port 4 on Port A */
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE(RS232P4_TAG, rs232_port_device, write_rts))
	/* DUSCC1 interrupt signal REQN is connected to LOCAL IRQ4 of the FGA-002 and level is programmable */
	MCFG_DUSCC_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq4_w))

	MCFG_DUSCC68562_ADD("duscc2", DUSCC_CLOCK, 0, 0, 0, 0 )
	/* Port 2 on Port A */
	MCFG_DUSCC_OUT_TXDA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(DEVWRITELINE(RS232P2_TAG, rs232_port_device, write_rts))
	/* Port 3 on Port B */
	MCFG_DUSCC_OUT_TXDB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSB_CB(DEVWRITELINE(RS232P3_TAG, rs232_port_device, write_rts))
	/* DUSCC2 interrupt signal REQN is connected to LOCAL IRQ5 of the FGA-002 and level is programmable */
	MCFG_DUSCC_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq5_w))

	MCFG_RS232_PORT_ADD (RS232P1_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsb_w))

	MCFG_RS232_PORT_ADD (RS232P2_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, ctsa_w))

	MCFG_RS232_PORT_ADD (RS232P3_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc2", duscc68562_device, ctsb_w))

	MCFG_RS232_PORT_ADD (RS232P4_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("duscc", duscc68562_device, ctsa_w))

	/* PIT Parallel Interface and Timer device, assumed strapped for on board clock */
	MCFG_DEVICE_ADD ("pit1", PIT68230, XTAL_16MHz / 2) // The PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu40_state, rotary_rd))
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu40_state, flop_dmac_r))
	MCFG_PIT68230_PB_OUTPUT_CB(WRITE8(fccpu40_state, flop_dmac_w))
	MCFG_PIT68230_PC_INPUT_CB(READ8(fccpu40_state, pit1c_r))
	MCFG_PIT68230_PC_OUTPUT_CB(WRITE8(fccpu40_state, pit1c_w))
// MCFG_PIT68230_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq2_w)) // Interrupts not yet supported by 68230

	MCFG_DEVICE_ADD ("pit2", PIT68230, XTAL_16MHz / 2) // Th PIT clock is not verified on schema but reversed from behaviour
	MCFG_PIT68230_PB_INPUT_CB(READ8(fccpu40_state, board_mem_id_rd))
	MCFG_PIT68230_PA_INPUT_CB(READ8(fccpu40_state, pit2a_r))
	MCFG_PIT68230_PA_OUTPUT_CB(WRITE8(fccpu40_state, pit2a_w))
	MCFG_PIT68230_PC_INPUT_CB(READ8(fccpu40_state, pit2c_r))
	MCFG_PIT68230_PC_OUTPUT_CB(WRITE8(fccpu40_state, pit2c_w))
// MCFG_PIT68230_OUT_INT_CB(DEVWRITELINE("fga002", fga002_device, lirq3_w)) // Interrupts not yet supported by 68230

	/* FGA-002, Force Gate Array */
	MCFG_FGA002_ADD("fga002", 0)
	MCFG_FGA002_OUT_INT_CB(WRITELINE(fccpu40_state, fga_irq_callback))
	MCFG_FGA002_OUT_LIACK4_CB(DEVREAD8("duscc",  duscc_device, iack))
	MCFG_FGA002_OUT_LIACK5_CB(DEVREAD8("duscc2",  duscc_device, iack))
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu40)
	ROM_REGION32_BE(0x900000, "roms", 0)

	ROM_LOAD16_BYTE("176921-0001.BIN",  0x000001, 0x40000, CRC (96cb3eb1) SHA1 (21c9793d2a4d04b04e649b215a0bf103b97f4eeb))
	ROM_LOAD16_BYTE("176921-0002.BIN",  0x000000, 0x40000, CRC (6b688b86) SHA1 (5b5d31c602e33eaf4acb6c1b9967efe9ea93a93d))

// Same binary for many boards, attempts to detect CPU speed etc, currently failing detection but boots system roms anyway
	ROM_LOAD       ("FGA-002_V4.14.bin",  0x800000, 0x10000, CRC(b210d731) SHA1(7509143105a786f21997724a8f0db3e246710240) )
ROM_END

/*
 * System ROM information 
 *
 * FGA-002 Bootprom version 4.14, 1992, coprighted by FORCE Computers Gmbh
 *
 * TODO: Investigate why init fails!!
 *
 * DUSCC #1 channel A setup 1 sequence FGA-002 firmware (polled i/o)
 * -----------------------------------------------------------------
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 00 <- 07 - Async mode
 *  A Reg 01 <- 38 - Normal polled or interrupt mode, no DMA
 *  A Reg 04 <- 7f - Tx 8 bits, CTS and RTS, 1 STOP bit
 *  A Reg 06 <- 1b - Rx RTS, 8 bits, no DCD, no parity
 *  A Reg 05 <- 3d - Tx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 07 <- 2d - Rx BRG 9600 (assuming a 14.7456 crystal)
 *  A Reg 0e <- 27 - TRxC = RxCLK 1x, RTxC is input, RTS, GPO2, crystal oscillator connected to X2
 *  A Reg 0b <- f1 - RTS low, OUT1 = OUT2 = high, RxRdy asserted on FIFO not empty
 *                   TxRdy asserted on FIFO not empty, Same Tx Residual Character Length as for REG_TPR
 *  A Reg 0f <- 00 - reset Tx Command
 *  A Reg 0f <- 40 - reset Rx Command
 *  A Reg 0f <- 02 - enable Tx Command
 *  A Reg 0f <- 42 - enable Rx Command
 *--- end of DUSCC setup sequence ---

 * Bootprom PIT setup sequence 
 * --------------------------------------------------------------------------------------------------------------
 * :pit1 Reg 10 -> ff PAAR - read port A of PIT1 without side effects
 * :pit2 Reg 10 -> 36 PBAR - read port B of PIT2 without side effects

// Currently the FGA init crashes and starts writing the wrong init from here on and the DUSCC that then will malfunction
 * :pit1 Reg 00 <- ff PGCR - Mode 3, H34:Enabled H12:Enabled Sense asserts H4:Hi, H3:Hi, H2:Hi, H1:Hi
 * :pit1 Reg 01 <- 80 PSSR - PC5 pin activated, PC5 pin supports no interrupts, H prio mode: 0  
 * :pit1 Reg 02 <- 0c PADDR - Port A pin 2 and 3 are outputs
 * :pit1 Reg 03 <- 01 PBDDR - Port B pin 1 is output
 * :pit1 Reg 08 <- 00 PADR pin 2 and 3 are low
 * :pit1 Reg 09 <- 5f PBDR pin 1 is high
 * :pit1 Reg 0a <- 00 PAAR read only register!
 * :pit1 Reg 0b <- 00 PBAR read only register!
 * :pit1 Reg 04 <- 00 PCDDR Port C all pins are inputs
 * :pit1 Reg 05 <- f0 PIVR interrupt vector
 * :pit1 Reg 06 <- 00 PACR - Submode 00 H2 input edge, no interrupts, H1 Interrupts/DMA disabled
 * :pit1 Reg 07 <- 0a PBCR - Submode 00 H4 input edge, no interrupts, H3 Interrupts/DMA enabled
 * :pit1 Reg 04 <- 00 PCDDR Port C all pins are inputs
 * :pit1 Reg 05 <- 0f PIVR interrupt vector
 * :pit1 Reg 06 <- 00 PACR - Submode 00 H2 input edge, no interrupts, H1 Interrupts/DMA disabled
 * :pit1 Reg 07 <- 8a PBCR - Submode 1X H4 input edge, no interrupts, H3 Interrupts/DMA enabled

 * :pit2 Reg 00 <- ff PGCR - Mode 3, H34:Enabled H12:Enabled Sense asserts H4:Hi, H3:Hi, H2:Hi, H1:Hi
 * :pit2 Reg 01 <- 80 PSSR - PC5 pin activated, PC5 pin supports no interrupts, H prio mode: 0  
 * :pit2 Reg 02 <- 0e PADDR - Port A pin 1 is input, all other pins are input
 * :pit2 Reg 03 <- 01 PBDDR - Port B pin 1 is output
 * :pit2 Reg 08 <- 00 PADR pin 2 and 3 are low
 * :pit2 Reg 09 <- 5f PBDR pin 1 is high
 * :pit2 Reg 0a <- 00 PAAR read only register!
 * :pit2 Reg 0b <- 00 PBAR read only register!
 * :pit2 Reg 04 <- 00 PCDDR Port C all pins are inputs
 * :pit2 Reg 05 <- f0 PIVR interrupt vector
 * :pit2 Reg 06 <- 00 PACR - Submode 00 H2 input edge, no interrupts, H1 Interrupts/DMA disabled
 * :pit2 Reg 07 <- 0a PBCR - Submode 00 H4 input edge, no interrupts, H3 Interrupts/DMA enabled
 * :pit2 Reg 04 <- 00 PCDDR Port C all pins are inputs
 *
 * And here the Duscc get reinited with the same setup as the PIT - Major wrong needs investigation!!
 *  :dusccA Reg 00 <- ff 
 *  :dusccA Reg 01 <- 80 
 *  :dusccA Reg 02 <- 20 
 *  :dusccA Reg 03 <- 01 
 *  :dusccA Reg 08 <- 00 
 *  :dusccA Reg 09 <- 5f 
 *  :dusccA Reg 0a <- 00 
 *  :dusccA Reg 0b <- 00 
 *  :dusccA Reg 04 <- 00 
 *  :dusccA Reg 05 <- f0 
 *  :dusccA Reg 06 <- 00 
 *  :dusccA Reg 07 <- 0a 
 *  :dusccA Reg 04 <- 00 
 *  :dusccA Reg 05 <- f0 
 *  :dusccA Reg 06 <- 00 
 *  :dusccA Reg 07 <- 8a 
 *  :duscc2A Reg 00 <- ff
 *  :duscc2A Reg 01 <- 80
 *  :duscc2A Reg 02 <- 22
 *  :duscc2A Reg 03 <- 01
 *  :duscc2A Reg 08 <- 00
 *  :duscc2A Reg 09 <- 5f
 *  :duscc2A Reg 0a <- 00
 *  :duscc2A Reg 0b <- 00
 *  :duscc2A Reg 04 <- 00
 *  :duscc2A Reg 05 <- f0
 *  :duscc2A Reg 06 <- 00
 *  :duscc2A Reg 07 <- 0a
 *  :duscc2A Reg 04 <- 00
 *
 */

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1992, fccpu40,      0,      0,       fccpu40,        fccpu40, driver_device, 0,   "Force Computers Gmbh",   "SYS68K/CPU-40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
