// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_FCWFC1_H
#define VME_FCWFC1_H
#pragma once

#include "bus/vme/vme.h"
#include "machine/wd_fdc.h"
#include "formats/imd_dsk.h"

DECLARE_DEVICE_TYPE(VME_FCWFC1, vme_fcwfc1_card_device);

class vme_fcwfc1_card_device :
	public device_t
	,public device_vme_card_interface
{
public:
	vme_fcwfc1_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	vme_fcwfc1_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

#if 0
	DECLARE_WRITE8_MEMBER (complete_irq_vector_w);
	DECLARE_READ8_MEMBER (data_r);
	DECLARE_WRITE8_MEMBER (data_req_irq_w);
	DECLARE_WRITE8_MEMBER (data_w);
	DECLARE_READ8_MEMBER (error_r);
	DECLARE_WRITE8_MEMBER (precomp_w);
	DECLARE_READ8_MEMBER (sector_count_r);
	DECLARE_WRITE8_MEMBER (sector_count_w);
	DECLARE_READ8_MEMBER (sector_number_r);
	DECLARE_WRITE8_MEMBER (sector_number_w);
	DECLARE_READ8_MEMBER (cylinder_low_r);
	DECLARE_WRITE8_MEMBER (cylinder_low_w);
	DECLARE_READ8_MEMBER (cylinder_high_r);
	DECLARE_WRITE8_MEMBER (cylinder_high_w);
	DECLARE_READ8_MEMBER (size_drive_head_r);
	DECLARE_WRITE8_MEMBER (size_drive_head_w);
	DECLARE_READ8_MEMBER (status_r);
	DECLARE_WRITE8_MEMBER (command_w);
#endif

	/* Dummy driver routines */
	DECLARE_READ8_MEMBER (not_implemented_r);
	DECLARE_WRITE8_MEMBER (not_implemented_w);

	/* Host interface*/
	DECLARE_READ8_MEMBER (dpram_r);
	DECLARE_WRITE8_MEMBER (dpram_w);
	DECLARE_ADDRESS_MAP(map, 8);

	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_fdd0;
	optional_device<floppy_connector> m_fdd1;
	optional_device<floppy_connector> m_fdd2;
	optional_device<floppy_connector> m_fdd3;

private:
	required_device<cpu_device> m_maincpu;
	DECLARE_FLOPPY_FORMATS( fcwfc1_floppy_formats );
};

#endif // VME_FCWFC1_H

