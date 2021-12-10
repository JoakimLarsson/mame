// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_FCWFC1_H
#define MAME_BUS_VME_VME_FCWFC1_H

#pragma once

#include "bus/vme/vme.h"

DECLARE_DEVICE_TYPE(VME_FCWFC1, vme_fcwfc1_card_device)

class vme_fcwfc1_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	// access methods from VME bus
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
};

#endif // MAME_BUS_VME_VME_FCWFC_H

