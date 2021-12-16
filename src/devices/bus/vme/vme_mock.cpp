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

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

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
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(VME_MOCK,         vme_mock_card_device,         "mock",        "VME card mock device template")
DEFINE_DEVICE_TYPE(VME_MOCK_FCAGC1,  vme_mock_fcagc1_card_device,  "mockfcagc1",  "Force AGC-1 VME card mock device")
DEFINE_DEVICE_TYPE(VME_MOCK_FCASCU1, vme_mock_fcascu1_card_device, "mockfcascu1", "Force ACUGA-1 VME card mock device")

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
	LOG("%s %s\n", tag, FUNCNAME);

	//  m_vme->static_set_custom_spaces(*this);
	uint32_t base = 0xFF0000;
	m_vme->install_device(vme_device::A24_SC, base, base + 0x0f,
			      read8sm_delegate(*this, FUNC(vme_mock_device::read)),
			      write8sm_delegate(*this, FUNC(vme_mock_device::write)),
			      0xffffffff);
}

// Force ASCU-1 mockup
vme_mock_fcascu1_card_device::vme_mock_fcascu1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : vme_mock_device(mconfig, VME_MOCK_FCASCU1, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mock_fcascu1_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	//  m_vme->static_set_custom_spaces(*this);
	uint32_t base = 0xFF0000;
	m_vme->install_device(vme_device::A24_SC, base, base + 0x0f,
			      read8sm_delegate(*this, FUNC(vme_mock_card_device::read)),
			      write8sm_delegate(*this, FUNC(vme_mock_card_device::write)),
			      0xffffffff);
}

uint8_t vme_mock_device::read(offs_t offset)
{
	LOGSETUP("%s offset:%02x\n", FUNCNAME, offset);
	return (uint8_t) 0;
}

void vme_mock_device::write(offs_t offset, uint8_t data)
{
	LOGSETUP("%s offset:%02x data:%02x\n", FUNCNAME, offset, data);
	return;
}
