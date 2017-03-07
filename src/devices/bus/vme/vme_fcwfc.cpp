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
 * LOC   C|                 +------++------+                         | |   |
 * ERR   C|                 |J28   ||J36   |                         | |   |
 * BUSY  C|                 |WD2797||WD1015|                         | |VME|
 *       ||                 |      ||      |                         | |   |
 *       ||                 | FDC  || BMGR |                         | |P1 |
 *       ||                 |      || ECC  |                         | |   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 +------++------+                         |_|   |
 *       ||                                                            |___|
 *       ||                 +------++------+                           |
 *       ||                 |J27   ||J35   |                           |
 *       ||                 |WD1014||WD1010|                           |
 *       ||                 |      ||      |                           |
 *       ||                 | EDSD || HDC  |                           |
 *       ||                 |      ||      |                           |
 *       ||                 |      ||      |                           |
 *       ||                 |      ||      |                           |
 *       ||                 |      ||      |                           |___
 *       ||                 |      ||      |                          _|   |
 *       ||                 |      ||      |                         | |   |
 *       ||                 +------++------+                         | |   |
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
 * - Three VMEbus options: (A31:D16), (A23:D16), (A15:D16) jumper selectable
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
#include "cpu/mcs48/mcs48.h"
#include "vme_fcwfc.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP )
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define TODO "Driver for WD1015, WD2927, WD1014 and WD1010 needed\n"
#define WD1015_TAG      "j36"

//**************************************************************************
//	GLOBAL VARIABLES
//**************************************************************************

const device_type VME_FCWFC1 = device_creator<vme_fcwfc1_card_device>;

//-------------------------------------------------
//  ADDRESS_MAP( wd1015_io )
//-------------------------------------------------

static ADDRESS_MAP_START( wd1015_io, AS_IO, 8, vme_fcwfc1_card_device )
#if 0 // from wdxt_gen.cpp TODO: Check schematics and hook up stuff accordingly
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE(WD11C00_17_TAG, wd11c00_17_device, read, write)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(wd1015_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(wd1015_t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(wd1015_p1_r, wd1015_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(wd1015_p2_r, wd1015_p2_w)
#endif
ADDRESS_MAP_END

/*
 * Machine configuration
 */
static MACHINE_CONFIG_FRAGMENT (fcwfc1)
	MCFG_CPU_ADD(WD1015_TAG, I8049, 5000000)
	MCFG_CPU_IO_MAP(wd1015_io)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------


machine_config_constructor vme_fcwfc1_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( fcwfc1 );
}

/* ROM definitions 
 * ROM has the following copyright string: 
 *  #Copyright (C) 1983 Western Digital Corporation  Written by Chandru Sippy & Michael Friese
 */
ROM_START (fcwfc1)
	ROM_REGION( 0x800, WD1015_TAG, 0 )
	ROM_LOAD( "WD1015-10.BIN", 0x000, 0x800, CRC(85dfe326) SHA1(f54803da3668193a3470ee0e24e3ea47ae605ec3) )
ROM_END

const tiny_rom_entry *vme_fcwfc1_card_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);
	return ROM_NAME( fcwfc1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu (*this, WD1015_TAG)
{
	LOG("%s\n", FUNCNAME);
}

vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VME_FCWFC1, "Force Computer SYS68K/WFC-1 Floppy and Winchester Controller Board", tag, owner, clock, "fcwfc1", __FILE__)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu (*this, WD1015_TAG)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

/* Start it up */
void vme_fcwfc1_card_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	set_vme_device();

	uint32_t base = 0xFCB01000; // Miniforce default base + offset 0-f TODO: Make configurable

	m_vme->install_device(vme_device::A24_SC, base      , base + 0x0f, // Dual ported RAM A24:D8
						  read8_delegate(FUNC(vme_fcwfc1_card_device::not_implemented_r), this),
						  write8_delegate(FUNC(vme_fcwfc1_card_device::not_implemented_w), this), 0xffffffff);
}

void vme_fcwfc1_card_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

READ8_MEMBER (vme_fcwfc1_card_device::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		LOG(TODO);
	}
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_fcwfc1_card_device::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		LOG(TODO);
	}
	return;
}
