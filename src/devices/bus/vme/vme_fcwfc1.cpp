// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 * WIP HLE device driver with aim to boot PDOS in the miniforce VME system
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
 *
 ****************************************************************************/

#include "emu.h"
#include "vme_fcwfc1.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_SETUP     (1U << 1)

#define VERBOSE (LOG_SETUP  | LOG_GENERAL)
#define LOG_OUTPUT_FUNC printf

//#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
//#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)
#include "logmacro.h"

#define LOG(...)      LOGMASKED(LOG_GENERAL, __VA_ARGS__)
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

//const device_type VME_FCWFC1 = device_creator<vme_fcwfc1_card_device>;
DEFINE_DEVICE_TYPE(VME_FCWFC1, vme_fcwfc1_card_device, "fcwfc1", "Force Computer SYS68K/WFC-1 Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_fcwfc1_card_device::device_add_mconfig(machine_config &config)
{
  //  	LOGSETUP("%s %s\n", tag, FUNCNAME);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************
vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	  device_vme_card_interface(mconfig, *this)
{
  //	LOGSETUP("%s\n", FUNCNAME);
}

vme_fcwfc1_card_device::vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_fcwfc1_card_device(mconfig, VME_FCWFC1, tag, owner, clock)
{
	LOGSETUP("%s %s\n", tag, FUNCNAME);
}

void vme_fcwfc1_card_device::device_reset()
{
	LOGSETUP("%s %s\n", tag(), FUNCNAME);
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
	LOGSETUP("%s\n", FUNCNAME);
	return ROM_NAME( fcwfc1 );
}

/* Start it up */
void vme_fcwfc1_card_device::device_start()
{
	LOGSETUP("%s\n", FUNCNAME);

	uint32_t base = 0xFCB01000; // Miniforce default base + offset 0-f TODO: Make configurable

	m_vme->install_device(vme_device::A24_SC, base, base + 0x0f,
			      read8sm_delegate(*this, FUNC(vme_fcwfc1_card_device::read)),
			      write8sm_delegate(*this, FUNC(vme_fcwfc1_card_device::write)),
			      0xffffffff);
}

uint8_t vme_fcwfc1_card_device::read(offs_t offset)
{
	LOGSETUP("%s offset:%02x\n", FUNCNAME, offset);
	return (uint8_t) 0;
}

void vme_fcwfc1_card_device::write(offs_t offset, uint8_t data)
{
	LOGSETUP("%s offset:%02x data:%02x\n", FUNCNAME, offset, data);
	return;
}
