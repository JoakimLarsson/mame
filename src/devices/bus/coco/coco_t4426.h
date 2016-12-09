// license:BSD-3-Clause
// copyright-holders:Nathan Woods
#pragma once

#ifndef __COCO_T4426_H__
#define __COCO_T4426_H__

#include "emu.h"
#include "cococart.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_t4426_device

class coco_t4426_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_t4426_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual DECLARE_READ8_MEMBER(read) override;
		virtual DECLARE_WRITE8_MEMBER(write) override;
private:
		// internal state
		required_device<acia6850_device> m_uart;
		required_device<pia6821_device> m_pia;
};


// device type definition
extern const device_type COCO_T4426;

#endif  /* __COCO_T4426_H__ */
