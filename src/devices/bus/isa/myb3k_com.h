// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
#ifndef MAME_BUS_ISA_MYB3K_COM_H
#define MAME_BUS_ISA_MYB3K_COM_H

#pragma once

#include "isa.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_myb3k_com_device

class isa8_myb3k_com_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_myb3k_com_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(myb3k_com_int);

protected:
	isa8_myb3k_com_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_MYB3K_COM, isa8_myb3k_com_device)

#endif  // MAME_BUS_ISA_MYB3K_COM_H
