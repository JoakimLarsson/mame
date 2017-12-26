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
#include "machine/wd11c00_17.h"
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
#define HLE 0

//**************************************************************************
//	GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_FCWFC1, vme_fcwfc1_card_device, "fcwfc1", "Force Computer SYS68K/WFC-1 Floppy and Winchester Controller Board");

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

DEVICE_ADDRESS_MAP_START( map, 8, vme_fcwfc1_card_device )
#if HLE
	AM_RANGE(0x00, 0x00) AM_WRITE(complete_irq_vector_w)
	AM_RANGE(0x01, 0x01) AM_READ(data_r) AM_WRITE(data_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(data_req_irq_w)
	AM_RANGE(0x03, 0x03) AM_READ(error_r) AM_WRITE(precomp_w)
	AM_RANGE(0x05, 0x05) AM_READ(sector_count_r) AM_WRITE(sector_count_w)
	AM_RANGE(0x07, 0x07) AM_READ(sector_number_r) AM_WRITE(sector_number_w)
	AM_RANGE(0x09, 0x09) AM_READ(cylinder_low_r) AM_WRITE(cylinder_low_w)
	AM_RANGE(0x0b, 0x0b) AM_READ(cylinder_high_r) AM_WRITE(cylinder_high_w)
	AM_RANGE(0x0d, 0x0d) AM_READ(size_drive_head_r) AM_WRITE(size_drive_head_w)
	AM_RANGE(0x0f, 0x0f) AM_READ(status_r) AM_WRITE(command_w)
#else
	AM_RANGE(0x0000, 0x000f) AM_READWRITE(dpram_r, dpram_w)
#if 0
	/* All these lines are ripped from wdxt_gen.cpp so need to be adjusted before being enabled in LLE */
	AM_RANGE(0x00, 0xff) AM_DEVREADWRITE(WD11C00_17_TAG, wd11c00_17_device, read, write)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(wd1015_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(wd1015_t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(wd1015_p1_r, wd1015_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(wd1015_p2_r, wd1015_p2_w)
#endif
#endif
ADDRESS_MAP_END

FLOPPY_FORMATS_MEMBER( vme_fcwfc1_card_device::fcwfc1_floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( fcwfc1_floppies )
	SLOT_INTERFACE( "525sd", FLOPPY_525_SD )
SLOT_INTERFACE_END

/*
 * Machine configuration
 */
MACHINE_CONFIG_MEMBER (vme_fcwfc1_card_device::device_add_mconfig)
#if HLE
#else
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", fcwfc1_floppies, "525sd", vme_fcwfc1_card_device::fcwfc1_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", fcwfc1_floppies, "525sd", vme_fcwfc1_card_device::fcwfc1_floppy_formats)
#if 0
	MCFG_CPU_ADD(WD1015_TAG, I8049, 5000000)
	MCFG_CPU_PROGRAM_MAP(wd1015_mem)
	MCFG_MCS48_PORT_T0_IN_CB(DEVREADLINE(WD11C00_17_TAG, wd11c00_17_device, busy_r))
	MCFG_MCS48_PORT_T1_IN_CB(READLINE(wdxt_gen_device, wd1015_t1_r))
	MCFG_MCS48_PORT_P1_IN_CB(READ8(wdxt_gen_device, wd1015_p1_r))
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(wdxt_gen_device, wd1015_p1_w))
	MCFG_MCS48_PORT_P2_IN_CB(READ8(wdxt_gen_device, wd1015_p2_r))
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(wdxt_gen_device, wd1015_p2_w))
#endif
#endif
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

#if 0
machine_config_constructor vme_fcwfc1_card_device::device_mconfig_additions() const
{
	LOG("%s %s\n", tag(), FUNCNAME);
	return MACHINE_CONFIG_NAME( fcwfc1 );
}
#endif

/* ROM definitions 
 * ROM has the following copyright string: 
 *  #Copyright (C) 1983 Western Digital Corporation  Written by Chandru Sippy & Michael Friese
 */
ROM_START (fcwfc1)
	ROM_REGION( 0x800, WD1015_TAG, 0 )
	ROM_LOAD( "WD1015-10.BIN", 0x000, 0x800, CRC(85dfe326) SHA1(f54803da3668193a3470ee0e24e3ea47ae605ec3) )
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
	,device_vme_card_interface(mconfig, *this)
	,m_maincpu (*this, WD1015_TAG)
{
	LOG("%s\n", FUNCNAME);
}

vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fcwfc1_card_device(mconfig, VME_FCWFC1, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

/* Start it up */
void vme_fcwfc1_card_device::device_start()
{
	LOG("%s\n", FUNCNAME);
	set_vme_device();

	uint32_t base = 0xFCB01000; // Miniforce default base + offset 0-f TODO: Make configurable

#if HLE
	m_vme->install_device(vme_device::A24_SC, base      , base + 0x0f, // Dual ported RAM A24:D8
						  read8_delegate(FUNC(vme_fcwfc1_card_device::not_implemented_r), this),
						  write8_delegate(FUNC(vme_fcwfc1_card_device::not_implemented_w), this), 0xffffffff);
#else
	// Dual ported RAM A24:D8
	m_vme->install_device(vme_device::A24_SC, base, base + 0x0f, &vme_fcwfc1_card_device::map, 8, 0xffffffff);
#endif
}

void vme_fcwfc1_card_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

READ8_MEMBER (vme_fcwfc1_card_device::dpram_r)
{
	uint8_t data = 0;

	LOGR("%s: %02x\n", FUNCNAME, data); 
	return data;
}

WRITE8_MEMBER (vme_fcwfc1_card_device::dpram_w)
{
	LOGSETUP("%s %02x <= %02x\n", FUNCNAME, offset, data); 
}

READ8_MEMBER (vme_fcwfc1_card_device::not_implemented_r)
{
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		LOG(TODO);
	}
	return (uint8_t) 0;
}

WRITE8_MEMBER (vme_fcwfc1_card_device::not_implemented_w)
{
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		LOG(TODO);
	}
	return;
}

#if 0
WRITE8_MEMBER (complete_irq_vector_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (data_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (data_req_irq_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
WRITE8_MEMBER (data_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (error_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (precomp_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (sector_count_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (sector_count_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (sector_number_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (sector_number_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (cylinder_low_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (cylinder_low_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (cylinder_high_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (cylinder_high_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (size_drive_head_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (size_drive_head_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
READ8_MEMBER (status_r){ uint8_t data = 0; LOGR("%s: %02x\n", FUNCNAME, data); return data; }
WRITE8_MEMBER (command_w){ LOGSETUP("%s %02x\n", FUNCNAME, data); }
#endif
