// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_FCWFC1_H
#define VME_FCWFC1_H
#pragma once

#include "bus/vme/vme.h"

extern const device_type VME_FCWFC1;

class vme_fcwfc1_card_device :
	public device_t
	,public device_vme_card_interface
{
public:
	vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	//	virtual const tiny_rom_entry *device_rom_region() const override;

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER (not_implemented_r);
	DECLARE_WRITE8_MEMBER (not_implemented_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
};

#endif // VME_FCWFC_H

