// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  VME mock implement a number of not yet implemented cards as place holders
 *  and to provide debug printouts
 *
 *  15/12/2021
 *
 *
 ****************************************************************************/

#include "emu.h"
#include "vme_mock.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_WRITE   (1U <<  3)

#define VERBOSE 0 // (LOG_GENERAL | LOG_SETUP)
#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGREAD(...)  LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGWRITE(...) LOGMASKED(LOG_WRITE,   __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(VME_MOCK,         vme_mock_card_device,         "mock",        "VME card mock device template")
DEFINE_DEVICE_TYPE(VME_MOCK_FCAGC1,  vme_mock_fcagc1_card_device,  "mockfcagc1",  "Force AGC-1 VME card mock device")
DEFINE_DEVICE_TYPE(VME_MOCK_FCASCU2, vme_mock_fcascu2_card_device, "mockfcascu2", "Force ASCU-2 VME card mock device")

//**************************************************************************
//  Base Device
//**************************************************************************

vme_mock_device::vme_mock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
        , device_vme_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

//**************************************************************************
//  Card Device
//**************************************************************************
vme_mock_card_device::vme_mock_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : vme_mock_device(mconfig, VME_MOCK, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mock_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	//  m_vme->static_set_custom_spaces(*this);
	uint32_t base = 0xFF0000;
	m_vme->install_device(vme_device::A24_SC, base, base + 0x0f,
			      read8sm_delegate(*this, FUNC(vme_mock_card_device::read)),
			      write8sm_delegate(*this, FUNC(vme_mock_card_device::write)),
			      0xffffffff);
}

// Force AGC-1 mockup
vme_mock_fcagc1_card_device::vme_mock_fcagc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : vme_mock_device(mconfig, VME_MOCK_FCAGC1, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mock_fcagc1_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	// Default base address from http://www.bitsavers.org/pdf/forceComputers/800106_AGC-1_Users_Manual_Aug86.pdf
	uint32_t base = 0xC00000;

	m_vme->install_device(vme_device::A24_SC, base, base + 0x3ffff,
			      read8sm_delegate(*this, FUNC(vme_mock_device::read)),
			      write8sm_delegate(*this, FUNC(vme_mock_device::write)),
			      0xffffffff);
}

// Force ASCU-2 mockup
vme_mock_fcascu2_card_device::vme_mock_fcascu2_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : vme_mock_device(mconfig, VME_MOCK_FCASCU2, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mock_fcascu2_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	// Default base address from http://www.bitsavers.org/pdf/forceComputers/800047_Force_ASCU-1_Users_Manual_May85.pdf
	uint32_t base = 0xB02000;
	m_vme->install_device(vme_device::A24_SC, base, base + 0x150,
			      read8sm_delegate(*this, FUNC(vme_mock_card_device::read)),
			      write8sm_delegate(*this, FUNC(vme_mock_card_device::write)),
			      0xffffffff);
}

//
// Generic access functions
uint8_t vme_mock_device::read(offs_t offset)
{
	LOGREAD("%s offset:%02x\n", FUNCNAME, offset);
	return (uint8_t) 0;
}

void vme_mock_device::write(offs_t offset, uint8_t data)
{
	LOGWRITE("%s offset:%02x data:%02x\n", FUNCNAME, offset, data);
	return;
}
