// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Motorola MVME series of CPU boards: MVME-162
 *
 *  16/05/2016
 *
 * Thanks to Plamen Mihaylov and his site http://www.m88k.com/ I got the information
 * required to start the work with this driver.
 *
 *
 *       ||
 * ||    ||  MVME-162
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |   |
 *       ||                                                          | |   |
 *       C|                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P1 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |___
 *       ||                                                           _|   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P2 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 * ||    ||                                                              +
 * ||||--||                                                              |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Motorola VME division (https://en.wikipedia.org/wiki/VMEbus)
 *---------------------------------
 * When Motorola released the 68000 processor 1979 the ambition of the deisgners
 * was also to standardize a versatile CPU bus to be able to build computer
 * systems without constructing PCB:s from scratch. This become VersaBus but the
 * boards was really too big and the computer world already saw the systems shrink
 * in size. Motorola's design center in Munich proposed to use the smaller and
 * already used Euroboard form factor and call it Versabus-E. This later became
 * VME which was standardized in the VITA organization 1981
 *
 * Misc links about Motorola VME division and this board:
 * http://bitsavers.trailing-edge.com/pdf/motorola/_dataBooks/1987_Microcomputer_Systems_and_Components.pdf
 * http://www.m88k.com/mvme162.html
 *
 * Description(s)
 * -------------
 * MVME-162 has the following feature set
 *      - 25/33MHz MHzMC68040 or MC68LC040 Microprocessor
 *      - 1 or 4 MB of DRAM with parity protection on a mezzanine module, or 16 MB ECC DRAM on a mezzanine board
 *      - 128 KB of SRAM with battery backup, or 2 MB SRAM on a mezzanine board with battery backup
 *      - Four JEDEC standard 32-pin DIP PROM sockets
 *      - One Intel 28F008SA 1M x 8 Flash memory device with write protection.
 *      - Status LEDs for FAIL, RUN, SCON, and FUSES
 *      - 8K by 8 Non-Volatile RAM (NVRAM) and time of day (TOD) clock with battery backup
 *      - RESET and ABORT switches
 *      - Four 32-bit Tick Timers and Watchdog Timer (in the MCchip ASIC) for periodic interrupts
 *      - Two 32-bit Tick Timers and Watchdog Timer (in the VMEchip2 ASIC) for periodic interrupts
 *      - Eight software interrupts (for MVME162LX versions that have the VMEchip2)
 *      - Optional SCSI Bus interface with DMA
 *      - Four serial ports with EIA-232-D interface (serial port controllers are the Z85230s
 *      - Optional Ethernet transceiver interface with DMA Two IndustryPack interfaces
 * VMEbus interface
 *      - VMEbus system controller functions
 *      - VMEbus interface to local bus (A24/A32,
 *      - D8/D16/D32 (D8/D16/D32/D64 BLT) (BLT = Block Transfer)
 *      - Local bus to VMEbus interface (A16/A24/A32, D8/D16/D32)
 *      - VMEbus interrupter
 *      - VMEbus interrupt handler
 *      - Global CSR for interprocessor communications
 *      - DMA for fast local memory - VMEbus transfers (A16/A24/A32, D16/D32 (D16/D32/D64 BLT)
 *
 * NOTE: This driver currently mimics the MVME162-020A configuration: 25MHz 68040, 4Mb RAM, 2 Serial ports, no SCSI, no Ethernet
 *
 * Address Map
 * --------------------------------------------------------------------------
 *                          Decscription
 * --------------------------------------------------------------------------
 * 00000000-001FFFFF        Boot ROM until ROM0 bit is cleared
 * Programmable             DRAM on Parity Mezzanine D32 1-4MB
 * Programmable             DRAM on ECC Mezzanine D32 16MB
 * Programmable             On-board SRAM D32 128KB
 * Programmable             SRAM on Mezzanine D32 2MB
 * Programmable             VMEbus A32/A24 D32/D16
 * Programmable             IP_a Memory D32-D8 64KB-8MB
 * Programmable             IP_b Memory D32-D8 64KB-8MB
 * FF800000-FF9FFFFF        Flash/EPROM D32 2Mb
 * FFA00000-FFBFFFFF        EPROM/Flash D32 2Mb
 * FFC00000-FFDFFFFF        Not decoded
 * FFE00000-FFE1FFFF        On-board SRAM D32 128Kb
 * FFE80000-FFEFFFFF        Not decoded
 * ------------------------ Local I/O devices D8/D16/D32
 * FFF00000-FFF3FFFF        Reserved 256KB      
 * FFF40000-FFF400FF        VMEchip2 (LCSR) D32 256B
 * FFF40100-FFF401FF        VMEchip2 (GCSR) D32-D8 256B
 * FFF40200-FFF40FFF        Reserved 3.5KB             
 * FFF41000-FFF41FFF        Reserved 4KB               
 * FFF42000-FFF41FFF        MCchip D32-D8 4KB
 * FFF43000-FFF430FF        MCECC #1 D8 256B
 * FFF43100-FFF431FF        MCECC #2 D8 256B
 * FFF43200-FFF43FFF        MCECC:s mirrored
 * FFF44000-FFF44FFF        Reserved
 * FFF45000-FFF45800        SCC #1 (Z85230) D8 2Kb
 * FFF45801-FFF45FFF        SCC #2 (Z85230) D8 2Kb
 * FFF46000-FFF46FFF        LAN (82596CA) D32 4Kb
 * FFF47000-FFF47FFF        SCSI (53C710) D32-D8 4Kb
 * FFF48000-FFF57FFF        Reserved
 * FFF58000-FFF587FF        IPIC IP_* D32-D16
 * FFF58800-FFF58FFF        Reserved 
 * FFFBC000-FFFBC01F        IPIC Registers D32-D8
 * FFFBC800-FFFBFFFF        Reserved
 * FFFC0000-FFFC7FFF        MK48T08 (BBRAM, TOD Clock) D32-D8 32Kb
 * FFFC8000-FFFCBFFF        MK48T08 & Disable Flash writes D32-D8 16Kb
 * FFFC8000-FFFCBFFF        MK48T08 & Enable Flash writes D32-D8 16Kb
 * FFFD0000-FFFEFFFF        Reserved
 * FFFF0000-FFFFFFFF        VMEbux short I/O D16
 * --------------------------------------------------------------------------
 *
 * Interrupt sources MVME
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 *
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         MVME147
 * ----------------------------------------------------------
 *
 *
 *  TODO:
 *  - Configure SCC:s connected to a terminal
 *  - Pass 147bug bootup tests
 *  - Add VME bus driver
 *  - Add variants of boards, preferably as runtime configurations
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/z80scc.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/timekpr.h"

#define VERBOSE 1

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

/* from documentataion: http://www.m88k.com/Docs/147/147aih.pdf but crystal and divider not known */
/*Serial Communications Interface
The MVME162LX uses two Zilog Z85230 serial port controllers to implement the four serial communications 
interfaces. Each interface supports CTS, DCD, RTS, and DTR control signals; as well as the TXD and RXD 
transmit/receive data signals. Because the serial clocks are omitted in the MVME162LX implementation, 
serial communications are strictly asynchronous. The MVME162LX hardware supports serial baud rates of 
110b/s to 38.4Kb/s. The Z85230 supplies an interrupt vector during interrupt acknowledge cycles.
The vector is modified based upon the interrupt source within the Z85230. Interrupt request levels are 
programmed via the MCchip. The Z85230s are interfaced as DTE (data terminal equipment) with EIA-232-D
signal levels. The four serial ports are routed to four RJ45 telephone connectors on the MVME162LX front panel.*/

#define BAUDGEN_CLOCK XTAL_5MHz            /* Baud rate on the MVME162 is programmable but defaults to 8N1 9600 */
#define DIVIDER 1                          /* Need a divider here to communicate correctly with the RS232 terminal device (9600) */
#define SCC_CLOCK (BAUDGEN_CLOCK/DIVIDER)  /* .. or schematics to implement the clock circuitry correctly. */

class mvme162_state : public driver_device
{
public:
mvme162_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_sccterm(*this, "scc")
//		,m_sccterm2(*this, "scc2")
	{
	}

	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);
	/* PCC - Peripheral Channel Controller */
	//DECLARE_READ32_MEMBER (pcc32_r);
	//DECLARE_WRITE32_MEMBER (pcc32_w);
	//DECLARE_READ16_MEMBER (pcc16_r);
	//DECLARE_WRITE16_MEMBER (pcc16_w);
	//DECLARE_READ8_MEMBER (pcc8_r);
	//DECLARE_WRITE8_MEMBER (pcc8_w);
	//DECLARE_READ8_MEMBER (vmechip_r);
	//DECLARE_WRITE8_MEMBER (vmechip_w);
	//DECLARE_READ16_MEMBER (vme_a24_r);
	//DECLARE_WRITE16_MEMBER (vme_a24_w);
	//DECLARE_READ16_MEMBER (vme_a16_r);
	//DECLARE_WRITE16_MEMBER (vme_a16_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;
protected:

private:
	required_device<cpu_device> m_maincpu;
	required_device<scc85230_device> m_sccterm;
  //	required_device<scc85230_device> m_sccterm2;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	UINT32	*m_sysrom;
	UINT32	m_sysram[2];

	// PCC registers
	UINT8	m_genpurp_stat;

	// VME chip registers
	UINT8	m_vc_cntl_conf;
};

static ADDRESS_MAP_START (mvme162_mem, AS_PROGRAM, 32, mvme162_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)	     /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)	     /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* 4 Mb RAM */
	AM_RANGE (0xff800000, 0xff9fffff) AM_ROM AM_REGION("maincpu", 0xff800000) //AM_MIRROR(0x00780000) /* ROM/EEPROM bank 1 - 162bug */
	AM_RANGE (0xffa00000, 0xffbfffff) AM_ROM AM_REGION("maincpu", 0xffa00000) //AM_MIRROR(0x00780000) /* ROM/EEPROM bank 2 - unpopulated */

        /*  SGS-Thompson M48T18 RAM and clock chip, only 4088 bytes used,  and 8 bytes for the RTC, out of 8Kb though */
	AM_RANGE (0xfffe0000, 0xfffe0fff) AM_DEVREADWRITE8("m48t18", timekeeper_device, read, write, 0xffffffff)

       //AM_RANGE (0xfffe1000, 0xfffe100f) AM_READWRITE32(pcc32_r, pcc32_w, 0xffffffff) /* PCC 32 bits registers  - needs U64 cast defined to work */
//	AM_RANGE (0xfffe1010, 0xfffe1017) AM_READWRITE16(pcc16_r, pcc16_w, 0xffffffff) /* PCC 16 bits registers */
//	AM_RANGE (0xfffe1018, 0xfffe102f) AM_READWRITE8(pcc8_r,	  pcc8_w,  0xffffffff) /* PCC 8 bits registers */
//	AM_RANGE (0xfffe2000, 0xfffe201b) AM_READWRITE8(vmechip_r, vmechip_w, 0x00ff00ff) /* VMEchip 8 bits registers on odd adresses */

	AM_RANGE (0xfff45000, 0xfff457ff) AM_DEVREADWRITE8("scc",  scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xffffffff) /* Port 1&2 - Dual serial port Z80-SCC */
//AM_RANGE (0xfff45000, 0xfff45800) AM_DEVREADWRITE8("scc2", scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xffffffff) /* Port 3&4 - Dual serial port Z80-SCC */

	//AM_RANGE(0x100000, 0xfeffff)	AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
	//AM_RANGE(0xff0000, 0xffffff)	AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (mvme162)
INPUT_PORTS_END

/* Start it up */
void mvme162_state::machine_start ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT32*)(memregion ("maincpu")->base () + 0xff800000);
	m_genpurp_stat = 0x02; /* Indicate power up reset */
	m_vc_cntl_conf = 0x01; /* We are the system controller */
}

void mvme162_state::machine_reset ()
{
	LOG(("--->%s\n", FUNCNAME));

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (UINT32*)(memregion ("maincpu")->base () + 0xff800000);
	m_genpurp_stat &= 0xfe; /* Clear parity error bit - not used by MAME at this point so just for the record */
}

/* 
  Boot vector handler. Devices mapped at $FFF80000-$FFF9FFFF also appear at $00000000-$001FFFFF when the ROM0 bit 
  in the MCchip EPROM control register is high (ROM0=1). ROM0 is set to 1 after each reset. The ROM0 bit must be
  cleared before other resources (DRAM or SRAM) can be mapped in this range ($00000000 - $001FFFFF).
*/
READ32_MEMBER (mvme162_state::bootvect_r){
	return m_sysrom[offset];
}

WRITE32_MEMBER (mvme162_state::bootvect_w){
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

#if 0 // Disable all special chips to start with
/**** 
 * PCC - Periheral Channel Controller driver, might deserve its own driver but will rest here until another board wants it
 */
#if 0 /* Doesn't compile atm */
READ32_MEMBER (mvme162_state::pcc32_r){
	LOG(("--->%s[%04x]", FUNCNAME, offset));
	switch(offset)
	{
	default:
		LOG(("unsupported register\n"));
	}
	return 0x00;
}

WRITE32_MEMBER (mvme162_state::pcc32_w){
	LOG(("--->%s[%04x]= %08lx", FUNCNAME, offset, data));
	switch(offset)
	{
	default:
		LOG(("unsupported register\n"));
	}
}
#endif

#define P16BASE 0xfffe1010
#define P16_TIMER1_PRELOAD (P16BASE)
#define P16_TIMER1_COUNT   (P16BASE + 2)
#define P16_TIMER2_PRELOAD (P16BASE + 4)
#define P16_TIMER2_COUNT   (P16BASE + 6)

READ16_MEMBER (mvme162_state::pcc16_r){

	UINT16 ret = 0;

	LOG(("Call to %s[%04x]", FUNCNAME, offset));
	switch(offset)
	{
	case  P16_TIMER1_PRELOAD - P16BASE   : LOG((" -> %02x Timer 1 preload - not implemented\n", ret)); break; 
	case  P16_TIMER1_COUNT	 - P16BASE   : LOG((" -> %02x Timer 1 count - not implemented\n", ret)); break; 
	case  P16_TIMER2_PRELOAD - P16BASE   : LOG((" -> %02x Timer 2 preload - not implemented\n", ret)); break; 
	case  P16_TIMER2_COUNT	 - P16BASE   : LOG((" -> %02x Timer 2 count - not implemented\n", ret)); break; 
	default:
		LOG((" -> %02x unsupported register\n", ret));
	}
	return ret;
}

WRITE16_MEMBER (mvme162_state::pcc16_w){
  LOG(("Call to %s[%04x] <- %04x - ", FUNCNAME, offset, data));
  switch(offset)
  {
  case	P16_TIMER1_PRELOAD - P16BASE   : LOG(("Timer 1 preload - not implemented\n")); break; 
  case	P16_TIMER1_COUNT   - P16BASE   : LOG(("Timer 1 count - not implemented\n")); break; 
  case	P16_TIMER2_PRELOAD - P16BASE   : LOG(("Timer 2 preload - not implemented\n")); break; 
  case	P16_TIMER2_COUNT   - P16BASE   : LOG(("Timer 2 count - not implemented\n")); break; 
  default:
	LOG(("unsupported register\n"));
  }
}

#define P8BASE 0xfffe1018
#define P8_TIMER1_INT_CNTL  0xfffe1018
#define P8_TIMER1_CNTL	    0xfffe1019
#define P8_TIMER2_INT_CNTL  0xfffe101A
#define P8_TIMER2_CNTL	    0xfffe101B
#define P8_ACFAIL_INT_CNTL  0xfffe101C
#define P8_WDOG_TIMER_CNTL  0xfffe101D
#define P8_PRINTER_INT_CNTL 0xfffe101E
#define P8_PRINTER_CNTL	    0xfffe101F
#define P8_DMA_INT_CNTL	    0xfffe1020
#define P8_DMA_CNTL_STAT    0xfffe1021
#define P8_BUSERR_CNTL	    0xfffe1022
#define P8_DMA_STATUS	    0xfffe1023
#define P8_ABORT_INT_CNTL   0xfffe1024
#define P8_TABADD_FC_CNTL   0xfffe1025
#define P8_SERIAL_INT_CNTL  0xfffe1026
#define P8_GEN_PURP_CNTL    0xfffe1027
#define P8_LAN_INT_CNTL	    0xfffe1028
#define P8_GEN_PURP_STAT    0xfffe1029
#define P8_SCSI_INT_CNTL    0xfffe102A
#define P8_SLAVE_BASE_ADDR  0xfffe102B
#define P8_SWI_1_CNTL	    0xfffe102C
#define P8_INT_VECT_BASE    0xfffe102D
#define P8_SWI_2_CNTL	    0xfffe102E
#define P8_REVISION_LEVEL   0xfffe102F
#define P8_PRINTER_DATA	    0xfffe2800
#define P8_PRINTER_STATUS   0xfffe2800

READ8_MEMBER (mvme162_state::pcc8_r){

	UINT8 ret = 0;

	LOG(("Call to %s[%04x]	    ", FUNCNAME, offset));
	switch(offset + P8BASE)
	{
	case P8_TIMER1_INT_CNTL	 : LOG((" -> %02x    - Timer 1 Interrupt Control  - not implemented\n", ret)); break;
	case P8_TIMER1_CNTL	 : LOG((" -> %02x    - Timer 1 Control - not implemented\n", ret)); break;
	case P8_TIMER2_INT_CNTL	 : LOG((" -> %02x    - Timer 2 Interrupt Control - not implemented\n", ret)); break;
	case P8_TIMER2_CNTL	 : LOG((" -> %02x    - Timer 2 Control - not implemented\n", ret)); break;
	case P8_ACFAIL_INT_CNTL	 : LOG((" -> %02x    - AC Fail Interrupt Control Register - not implemented\n", ret)); break;
	case P8_WDOG_TIMER_CNTL	 : LOG((" -> %02x    - Watchdog Timer Control Register - not implemented\n", ret)); break;
	case P8_PRINTER_INT_CNTL : LOG((" -> %02x    - Printer Interrupt Control Register - not implemented\n", ret)); break;
	case P8_PRINTER_CNTL	 : LOG((" -> %02x    - Printer Control Register - not implemented\n", ret)); break;
	case P8_DMA_INT_CNTL	 : LOG((" -> %02x    - DMA Interrupt Control Register - not implemented\n", ret)); break;
	case P8_DMA_CNTL_STAT	 : LOG((" -> %02x    - DMA Control and Status Register - not implemented\n", ret)); break;
	case P8_BUSERR_CNTL	 : LOG((" -> %02x    - Bus Error Interrupt Control Register - not implemented\n", ret)); break;
	case P8_DMA_STATUS	 : LOG((" -> %02x    - DMA Status Register - not implemented\n", ret)); break;
	case P8_ABORT_INT_CNTL	 : LOG((" -> %02x    - Abort Interrupt Control Register - not fully implemented\n", ret)); 
	  /* Bit 3 When this bit is high, the interrupt is enabled. The interrupt is disabled when this bit is low. This bit is cleared by reset.
	     Bit 6 This bit indicates the current state of the ABORT switch. When this bit is low, the ABORT switch is not pressed. When this bit is
		   high, the ABORT switch is pressed.
	     Bit 7 When this bit is high, an abort interrupt is being generated at Level 7. This bit is edge sensitive and it is set on the leading
		   edge of interrupt enable and abort. This bit is cleared when a 1 is written to it or when the interrupt is disabled. When cleared,
		   it remains cleared until the next leading edge of interrupt enable and abort. This bit is cleared by reset. */
		ret = 0; /* Always return reset values for now */
		break;
	case P8_TABADD_FC_CNTL	 : LOG((" -> %02x    - Table Address Function Code Register - not implemented\n", ret)); break;
	case P8_SERIAL_INT_CNTL	 : LOG((" -> %02x    - Serial Port Interrupt Control Register - not implemented\n", ret)); break;
	case P8_GEN_PURP_CNTL	 : LOG((" -> %02x    - General Purpose Control Register - not implemented\n", ret)); break;
	case P8_LAN_INT_CNTL	 : LOG((" -> %02x    - LAN Interrupt Control Register - not implemented\n", ret)); break;
	case P8_GEN_PURP_STAT	 : LOG((" -> %02x    - General Purpose Status Register\n", ret)); 
		ret = m_genpurp_stat;
		break;
	case P8_SCSI_INT_CNTL	 : LOG((" -> %02x    - SCSI Port Interrupt Control Register - not implemented\n", ret)); break;
	case P8_SLAVE_BASE_ADDR	 : LOG((" -> %02x    - Slave Base Address Register - not implemented\n", ret)); break;
	case P8_SWI_1_CNTL	 : LOG((" -> %02x    - Software Interrupt 1 Control Register - not implemented\n", ret)); break;
	case P8_INT_VECT_BASE	 : LOG((" -> %02x    - Interrupt Vector Base - not implemented\n", ret)); break;
	case P8_SWI_2_CNTL	 : LOG((" -> %02x    - Software Interrupt 2 Control Register - not implemented\n", ret)); break;
	case P8_REVISION_LEVEL	 : LOG((" -> %02x    - PCC Revision Level Register - not implemented\n", ret)); break;
	case P8_PRINTER_STATUS	 : LOG((" -> %02x    - Printer Status Register - not implemented\n", ret)); break;
	default:
		LOG((" -> %02x	  - unsupported register\n", ret));
	}
	return ret;
}

WRITE8_MEMBER (mvme162_state::pcc8_w){
	LOG(("Call to %s[%04x] <- %02x	  - ", FUNCNAME, offset, data));
	switch(offset + P8BASE)
	{
	case P8_TIMER1_INT_CNTL	 : LOG(("Timer 1 Interrupt Control - not implemented\n")); break;
	case P8_TIMER1_CNTL	 : LOG(("Timer 1 Control - not implemented\n")); break;
	case P8_TIMER2_INT_CNTL	 : LOG(("Timer 2 Interrupt Control - not implemented\n")); break;
	case P8_TIMER2_CNTL	 : LOG(("Timer 2 Control - not implemented\n")); break;
	case P8_ACFAIL_INT_CNTL	 : LOG(("AC Fail Interrupt Control Register - not implemented\n")); break;
	case P8_WDOG_TIMER_CNTL	 : LOG(("Watchdog Timer Control Register - not implemented\n")); break;
	case P8_PRINTER_INT_CNTL : LOG(("Printer Interrupt Control Register - not implemented\n")); break;
	case P8_PRINTER_CNTL	 : LOG(("Printer Control Register - not implemented\n")); break;
	case P8_DMA_INT_CNTL	 : LOG(("DMA Interrupt Control Register - not implemented\n")); break;
	case P8_DMA_CNTL_STAT	 : LOG(("DMA Control and Status Register - not implemented\n")); break;
	case P8_BUSERR_CNTL	 : LOG(("Bus Error Interrupt Control Register - not implemented\n")); break;
	case P8_DMA_STATUS	 : LOG(("DMA Status Register - not implemented\n")); break;
	case P8_ABORT_INT_CNTL	 : LOG(("Abort Interrupt Control Register - not implemented\n")); break;
	case P8_TABADD_FC_CNTL	 : LOG(("Table Address Function Code Register - not implemented\n")); break;
	case P8_SERIAL_INT_CNTL	 : LOG(("Serial Port Interrupt Control Register - not implemented\n")); break;
	case P8_GEN_PURP_CNTL	 : LOG(("General Purpose Control Register - not implemented\n"));
	 /*Bits 0-1 These bits control local RAM parity checking. These bits should not be enabled on the MVME162-010. 
		     These bits are cleared by reset. x0 = parity disabled, x1 = parity enabled	 
	    Bit	 2   This bit is used to test the parity generating and checking logic. When this bit is low, correct parity is written to the DRAM;
		     when high, incorrect parity is written to the DRAM. This bit is cleared by reset. 
	     NOTE: We really don't care about DRAM parity! 
	    Bit	 3   When set, this bit is used to enable the local bus timer that is part of the PCC. Because the VMEchip also contains a local bus
		     timer, this bit should be cleared, turning off the PCC local bus timer. This bit is cleared by reset.
	    Bit	 4   This bit is the master interrupt enable. When this bit is low, all interrupts on the MVME162 are disabled; when high, all
		     interrupts are enabled. This bit is cleared by reset
	    Bits 5-7 When the pattern %101 is written to these bits, the front panel RESET switch is disabled. The RESET switch is enabled for any
		     other pattern. These bits are cleared by reset. 
	     TODO: Bit 4-7 needs to be implemented
	  */
		break;
	case P8_LAN_INT_CNTL	 : LOG(("LAN Interrupt Control Register - not implemented\n")); break;
	case P8_GEN_PURP_STAT	 : LOG(("General Purpose Status Register\n")); 
	  /* Bit 0 This bit is set when a parity error occurs while the local processor is accessing RAM. This bit is cleared by writing a 1 to it. 
		   This bit is cleared by reset. 
	     Bit 1 This bit is set when a power-up reset occurs. It is cleared by writing a 1 to it. 
		   When the MVME162BUG is installed, its initialization code clears this bit.
	  */
		m_genpurp_stat &= ((data & 1) ? ~1 : 0xff); // Check if parity error bit needs to be cleared
		m_genpurp_stat &= ((data & 2) ? ~2 : 0xff); // Check if power up reset bit needs to be cleared
		break;
	case P8_SCSI_INT_CNTL	 : LOG(("SCSI Port Interrupt Control Register - not implemented\n")); break;
	case P8_SLAVE_BASE_ADDR	 : LOG(("Slave Base Address Register - not implemented\n")); break;
	case P8_SWI_1_CNTL	 : LOG(("Software Interrupt 1 Control Register - not implemented\n")); break;
	case P8_INT_VECT_BASE	 : LOG(("Interrupt Vector Base - not implemented\n")); break;
	case P8_SWI_2_CNTL	 : LOG(("Software Interrupt 2 Control Register - not implemented\n")); break;
	case P8_REVISION_LEVEL	 : LOG(("PCC Revision Level Register - not implemented\n")); break;
	case P8_PRINTER_DATA	 : LOG(("Printer Data Register - not implemented\n")); break;
	default:
		LOG(("unsupported register\n"));
	}
}

/*
 * VMEchip ASIC driver, might deserve its own driver but will rest here until another board wants it
 */
#define VCBASE 0xfffe2001
#define VC_SYS_CNTL_CONF    0xfffe2001
#define VC_VMEBUS_REQ_CONF  0xfffe2003
#define VC_MASTER_CONF	    0xfffe2005
#define VC_SLAVE_CONF	    0xfffe2007
#define VC_TIMER_CONF	    0xfffe2009
#define VC_SLAVE_ADR_MOD    0xfffe200B
#define VC_MASTER_ADR_MOD   0xfffe200D
#define VC_INT_HNDL_MASK    0xfffe200F
#define VC_UTIL_INT_MASK    0xfffe2011
#define VC_UTIL_INT_VECT    0xfffe2013
#define VC_INT_REQUEST	    0xfffe2015
#define VC_VMEBUS_STAT_ID   0xfffe2017
#define VC_BUS_ERR_STATUS   0xfffe2019
#define VC_GCSR_BASE_ADR    0xfffe201B


READ8_MEMBER (mvme162_state::vmechip_r){

	UINT8 ret = 0;

	LOG(("Call to %s[%04x]	    ", FUNCNAME, offset));
	switch(offset * 2 + VCBASE)
	{
	case VC_SYS_CNTL_CONF	 : 
		LOG((" -> %02x - System Controller Configuration Register - not implemented\n", ret)); 
		ret = m_vc_cntl_conf;
		break;
	case VC_VMEBUS_REQ_CONF	 : 
	  /* 
	    Bits 0-1 These control bits configure the VMEbus requester level as shown in the table below:
		     RQLEV1 RQLEV0 Level
		      0	     0	    0
		      0	     1	    1
		      1	     0	    2
		      1	     1	    3
	      These bits are set to 1, 1 by any reset. Note that writes to REQLEV1,0 do not change the actual
	       requester level until the MVME162 goes through the action of having VMEbus mastership and releasing it. This means that
	       there are times when the value written into REQLEV1,0 do not match the current requester level (the request level is lagging).
	       During such times, reads to REQLEV1,0 reflect the actual requester level, not the value written into REQLEV1,0.
      Bit  3   Setting this bit to 1 prevents the requester from releasing the VMEbus. However, unlike the DWB control bit, setting the
	       RNEVER bit does not cause the requester to request the VMEbus. Clearing the RNEVER bit allows the requester to
	       relinquish the VMEbus in accordance with the other control bits of the requester configuration register. 
	       This bit is cleared by any reset. 
      Bit  4   The RWD bit allows software to configure the requester release mode. When the bit is set, if RNEVER and DWB are both
	       cleared to 0, the requester releases the VMEbus after the MC68030 completes a VMEbus cycle. When the bit is cleared, if
	       RNEVER and DWB are both cleared to 0, the requester operates in the Release-On-Request (ROR) mode. After acquiring control
	       of the VMEbus, it maintains control until it detects another request pending on the VMEbus. This bit is cleared by any reset.
      Bit  5   The RONR bit controls the manner in which the VMEchip requests the VMEbus. When the bit is set; anytime the
	       MVME162 has bus mastership, then gives it up, the VMEchip does not request the VMEbus again until it detects the bus
	       request signal BR*, on its level, negated for at least 150 ns. When the VMEchip detects BR* negated, it refrains from
	       driving it again for at least 200 ns. This bit is cleared by any reset.
      Bit 6    The DHB status bit is 1 when the MVME162 is VMEbus master and 0 when it is not.
      Bit 7    Setting the DWB control bit to 1 causes the VMEchip to request the VMEbus (if not already bus master). When VMEbus
	       mastership has been obtained, it is not relinquished until after the DWB and RNEVER bits are both cleared. 
	       This bit is cleared by any reset. 
    */
		ret = 1 << 6; /* Let BUG162 think we are bus master. TODO: Implement proper VME bus signalling */ 
		LOG((" -> %02x - VMEbus Requester Configuration Register - not implemented\n", ret)); 
		break;
	case VC_MASTER_CONF	 : LOG((" -> %02x - Master Configuration Register - not implemented\n", ret)); break;
	case VC_SLAVE_CONF	 : LOG((" -> %02x - Slave Configuration Register - not implemented\n", ret)); break;
	case VC_TIMER_CONF	 : LOG((" -> %02x - Timer Configuration Register - not implemented\n", ret)); 
    /*Bits 0-1 These two bits configure the local time-out period. They are set to 1 by any reset.
	       LBTO1 LBTO0 Time-Out Period
		0     0	   102 microseconds
		0     1	   205 microseconds
		1     0	   410 microseconds
		1     1	   Timer disabled
	       The local bus timer activates bus error to the MC68030 when it tries to access nonexistent locations in the local memory map
      Bits 2-3 These two bits configure the VMEbus access time-out period. They are set to 1 by any reset.
	       ACTO1 ACTO0 Time-Out Period
		0     0	   102 microseconds
		0     1	   1.6 millisecond
		1     0	    51 milliseconds
		1     1	   Timer disabled
	       The VMEbus access timer activates bus error to the MC68030 (except on write posted time-outs) when the VMEchip is
	       unsuccessful in obtaining the VMEbus within the time-out period 
       Bits 4-5 These two bits configure the VMEbus global time-out period. VBTO1 is set to 1 and VBTO0 is cleared to 0 by SYSRESET.
	       VBTO1 VBTO0 Time-Out Period
		0     0	   102 microseconds
		0     1	   205 microseconds
		1     0	   410 microseconds
		1     1	   Timer disabled
	       The VMEbus global timer activates BERR* on the VMEbus. 
       Bit  6  Setting ARBTO to 1 enables the VMEbus arbitration timer. The VMEbus arbitration timer activates BBSY* if it is not activated
	       within 410 µs after the MVME162 arbiter issues a bus grant. The timer deactivates BBSY* as specified in the VMEbus specification. 
	       This causes the arbiter to arbitrate any pending requests for the bus. This bit is set to 1 by SYSRESET.
    */
		break;
	case VC_SLAVE_ADR_MOD	 : LOG((" -> %02x - Slave Address Modifier Register - not implemented\n", ret)); break;
	case VC_MASTER_ADR_MOD	 : LOG((" -> %02x - Master Address Modifier Register - not implemented\n", ret)); break;
	case VC_INT_HNDL_MASK	 : LOG((" -> %02x - Interrupt Handler Mask Register - not implemented\n", ret)); break;
	case VC_UTIL_INT_MASK	 : LOG((" -> %02x - Utility Interrupt Mask Register - not implemented\n", ret)); break;
	case VC_UTIL_INT_VECT	 : LOG((" -> %02x - Utility Interrupt Vector Register - not implemented\n", ret)); break;
	case VC_INT_REQUEST	 : LOG((" -> %02x - Interrupt Request Register - not implemented\n", ret)); break;
	case VC_VMEBUS_STAT_ID	 : LOG((" -> %02x - VMEbus Status/ID Register - not implemented\n", ret)); break;
	case VC_BUS_ERR_STATUS	 : LOG((" -> %02x - Bus Error Status Register - not implemented\n", ret)); break;
	case VC_GCSR_BASE_ADR	 : LOG((" -> %02x - GCSR Base Address Configuration Register - not implemented\n", ret)); break;
	default:
		LOG(("unsupported register"));
  }
  return ret;
}

WRITE8_MEMBER (mvme162_state::vmechip_w){
	LOG(("Call to %s[%04x] <- %02x - ", FUNCNAME, offset, data));
	switch(offset * 2 + VCBASE)
	{
	case VC_SYS_CNTL_CONF	 : 
		LOG(("System Controller Configuration Register - not implemented\n")); 
		m_vc_cntl_conf = data & 0xff;
		break;
	case VC_VMEBUS_REQ_CONF	 : LOG(("VMEbus Requester Configuration Register - not implemented\n")); break;
	case VC_MASTER_CONF	 : LOG(("Master Configuration Register - not implemented\n")); break;
	case VC_SLAVE_CONF	 : LOG(("Slave Configuration Register - not implemented\n")); break;
	case VC_TIMER_CONF	 : LOG(("Timer Configuration Register - not implemented\n")); break;
	case VC_SLAVE_ADR_MOD	 : LOG(("Slave Address Modifier Register - not implemented\n")); break;
	case VC_MASTER_ADR_MOD	 : LOG(("Master Address Modifier Register - not implemented\n")); break;
	case VC_INT_HNDL_MASK	 : LOG(("Interrupt Handler Mask Register - not implemented\n")); break;
	case VC_UTIL_INT_MASK	 : LOG(("Utility Interrupt Mask Register - not implemented\n")); break;
	case VC_UTIL_INT_VECT	 : LOG(("Utility Interrupt Vector Register - not implemented\n")); break;
	case VC_INT_REQUEST	 : LOG(("Interrupt Request Register - not implemented\n")); break;
	case VC_VMEBUS_STAT_ID	 : LOG(("VMEbus Status/ID Register - not implemented\n")); break;
	case VC_BUS_ERR_STATUS	 : LOG(("Bus Error Status Register - not implemented\n")); break;
	case VC_GCSR_BASE_ADR	 : LOG(("GCSR Base Address Configuration Register - not implemented\n")); break;
	default:
		LOG(("unsupported register\n"));
	}
}
#endif

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (mvme162_state::vme_a24_r){
	LOG (logerror ("vme_a24_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (mvme162_state::vme_a24_w){
	LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (mvme162_state::vme_a16_r){
	LOG (logerror ("vme_16_r\n"));
	return (UINT16) 0;
}

WRITE16_MEMBER (mvme162_state::vme_a16_w){
	LOG (logerror ("vme_a16_w\n"));
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (mvme162, mvme162_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68040, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP (mvme162_mem)

	MCFG_M48T02_ADD("m48t18") /* t08 differs only in accepted voltage levels compared to t18 */

	/* Terminal Port config */
	MCFG_SCC85230_ADD("scc", SCC_CLOCK, 0, 0, 0, 0 )
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
	MCFG_Z80SCC_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("scc", scc85230_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("scc", scc85230_device, ctsa_w))

//	MCFG_SCC85230_ADD("scc2", SCC_CLOCK, 0, 0, 0, 0 )
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (mvme162)
ROM_REGION32_BE(0xfff00000, "maincpu", 0)

ROM_LOAD("162bug4.0.bin", 0xff800000, 0x80000, CRC (56728e5b) SHA1 (0b8b6725c21d8a9048d24857d6acd2b68a7f3ba0))

/*
 * System ROM information
 *
 * 147bug version 2.44 is released 1999, coprighted by Motorola Inc from 1988
 *
 * 147bug SCC channel setup sequence
 *----------------------------------
 *  09 80 - Channel A reset
 *  04 44 - x16 clock, 1 stop bits, no parity
 *  03 C1 - 8 bit, receiver enable, auto enable off
 *  01 00 - 
 *  02 70 - set interrupt vector 70
 *  05 EA
 *  0B 56
 *  0C 0E - low baudrate divider 
 *  0D 00 - hi baudrate divider 
 *  0E 83 - Baud Rate Generator (BRG) enabled, PCLK is BRG source, BRG is DPLL source
 *  06 00 - Tx sync character SDLC
 *  07 00 - Rx sync character SDLC
 *  0F 00 - disable all external interrupt and status
 *  10
 * channel B is identical but resets Channel B of course, SCC2 is also identical except using interrupt vector 71 
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1993, mvme162,      0,      0,       mvme162,        mvme162, driver_device, 0,   "Motorola",   "MVME-162", MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
