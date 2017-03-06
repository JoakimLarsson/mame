// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *
 *       ||
 * ||    ||
 * ||||--||
 * ||||--|| SYS68K/WFC-1 board
 * ||    ||__________________________________________________________    ___
 *       ||                                                          |_|   |
 *       ||                                                          | |   |
 * RUN   C|                                                          | |   |
 * R/L o-[|                                                          | |   |
 * LOC   C|                                                          | |   |
 * ERR   C|                                                          | |   |
 * BUSY  C|                                                          | |VME|
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
 * ||    ||------------------------------------------------------------+-+
 * ||||--||
 * ||||--||
 * ||
 *
 * History of Force Computers
 *------------------------------------------------------------------------
 *  See fccpu30.cpp
 *
 * Description from datasheet etc
 * ------------------------------
 * - Fully VMEbus compatible
 * - Jumper selectable base address with address modifier
 * - Generation at two different interrupts
 * - Jumper selectable interrupt level
 * - Software programmable interrupt vectors
 * - Three VMEbus options· (A31:D16), (A23:D16), (A15:D16) jumper selectable
 * - User selectable 5.25" Winchester or Floppy operation
 * - Controls up to 3 Winchester ST506 Interface and up to 4 Floppy 
 *   drives SA450 compatible 
 * - On-board data seperation circuitry
 * - On-board write precompensation for floppy and hard disks
 * - On-board sector buffer supports up to 1 KByte sectors
 * - Programmable sector sizes - 128, 2.56, 512, or 1024 bytes
 * - Automatic track formatting on hard and floppy disks
 * - Multiple sector operations on all disks
 * - Data rates up to 5 1'1bits/ sec on hard disk
 * - Single burst error correction up to 5 bits on hard disk data
 * - CRC generation/verification for data and all I.D. fields
 * - Automatic retries on all errors with simulated completion 
 * - ECC diagnostic commands included (READLONG & WRITELONG)
 * - Internal diagnostics
 * - 16 different stepping rates for both hard and floppy drives 
 *
 * VME side A24 address map
 * ----------------------------------------------------------
 * Default  Read            Write
 * ----------------------------------------------------------
 * B01000                   Complete Interrupt Vector register
 * B01001 Data Register     Data Register
 * B01002                   Data Request Interrupt Vector Register
 * B01003 Error register    Write Precomp
 * B01005 Sector Count      Sector Count
 * B01007 Sector Number     Sector Number
 * B01009 Cylinder Low      Cylinder Low
 * B0100B Cylinder High     Cylinder High
 * B0100D Size/Drive/Head   Size/Drive/Head
 * B0100F Status Register   Command register
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Checkout the ISA board based on similar/same WD chipsets
 *    - make or port decision for the different chips
 *    - HLE or MLE decision
 *  - add VME bus device
 *
 ****************************************************************************/

#include "emu.h"
#include "vme_wfc.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 //(LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//	GLOBAL VARIABLES
//**************************************************************************

const device_type VME_WFC1 = &device_creator<vme_wfc1_card_device>;

static ADDRESS_MAP_START (fcisio1_mem, AS_PROGRAM, 16, vme_fcisio1_card_device)
	ADDRESS_MAP_UNMAP_HIGH
//	AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Vectors mapped from System EPROM */
//	AM_RANGE (0x000000, 0x01ffff) AM_RAM /* SRAM */
//	AM_RANGE (0xe00000, 0xe001ff) AM_DEVREADWRITE8("duscc0", duscc68562_device, read, write, 0x00ff)
//	AM_RANGE (0xf00000, 0xf7ffff) AM_ROM /* System EPROM Area 32Kb DEBUGGER supplied */
//  AM_RANGE (0xc40000, 0xc800ff) AM_READWRITE8 (not_implemented_r, not_implemented_w, 0xffff)  /* Dummy mapping af address area to display message */
ADDRESS_MAP_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_FRAGMENT (fcwfc1)
	/* basic machine hardware */
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fcwfc1)
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu (*this, "maincpu")
	,m_duscc0(*this, "duscc0")
	,m_duscc1(*this, "duscc1")
	,m_duscc2(*this, "duscc2")
	,m_duscc3(*this, "duscc3")
	,m_pit (*this, "pit")
	,m_bim (*this, "bim")
{
	LOG("%s\n", FUNCNAME);
}

vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_FCWFC1, "Force Computer SYS68K/ISIO-1/2 Intelligent Serial I/O Board", tag, owner, clock, "fcwfc1", __FILE__)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu(*this, "maincpu")
	,m_duscc0(*this, "duscc0")
	,m_duscc1(*this, "duscc1")
	,m_duscc2(*this, "duscc2")
	,m_duscc3(*this, "duscc3")
	,m_pit (*this, "pit")
	,m_bim (*this, "bim")
{
	LOG("%s %s\n", tag, FUNCNAME);
}

/* Start it up */
void vme_fcwfc1_card_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	set_vme_device();

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint16_t*)(memregion ("maincpu")->base () + 0xf00000);

#if 0 // TODO: Setup VME access handlers for shared memory area
	uint32_t base = 0xFFFF5000;
	m_vme->install_device(base + 0, base + 1, // Channel B - Data
							 read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("pit")), 0x00ff);
	m_vme->install_device(base + 2, base + 3, // Channel B - Control
							 read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("pit")), 0x00ff);
#endif

}

void vme_fcwfc1_card_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

READ8_MEMBER (vme_fcwfc1_card_device::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_fcwfc1_card_device::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return;
}
