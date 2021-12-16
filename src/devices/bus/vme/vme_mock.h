// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_MOCK_H
#define MAME_BUS_VME_VME_MOCK_H

#pragma once

#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_MOCK, vme_mock_card_device)
DECLARE_DEVICE_TYPE(VME_MOCK_FCAGC1, vme_mock_fcagc1_card_device)
DECLARE_DEVICE_TYPE(VME_MOCK_FCASCU1, vme_mock_fcascu1_card_device)

class vme_mock_device : public device_t, public device_vme_card_interface
{
protected:
	vme_mock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
public:
	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);
};

//**************************************************************************
//  Board Device declarations
//**************************************************************************

// Mock this template class, copy this class, rename it and tweak the method to capture the mocked cards accesses
// When the card is understood break it out in its own file and remove it from here
class vme_mock_card_device: public vme_mock_device
{
public:
	vme_mock_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mock_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;  
};

// Force AGC-1 mockup
class vme_mock_fcagc1_card_device: public vme_mock_device
{
public:
	vme_mock_fcagc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mock_fcagc1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
};

// Force ACUGA-1 mockup
class vme_mock_fcascu1_card_device: public vme_mock_device
{
public:
	vme_mock_fcascu1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mock_fcascu1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
};
#endif // MAME_BUS_VME_VME_MOCK_H
