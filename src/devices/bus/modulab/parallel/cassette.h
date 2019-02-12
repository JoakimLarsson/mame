// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-===============================================================================================================-
 *
 * Modulab cassette interface, design by Per-Ola Svensson 1985-03-21
 *
 * Pin B7 is connected to the cassette recorder input via simple circuit:
 *
 *   B7 o--- R1.5K ---+---+-----o to cassette recorder
 *                    |   |
 *                  R100 C0.1u
 *                    |   |
 *               GND--+---+
 *
 * Pin B6 is connected to the cassette recorder output
 *
 *   +5V---------------------------------+-------------+---------------+
 * from                                  |             |               |
 * cassette o--C470p-R4.7K-+-R4.7K-C10n--|----+-----|+ |          +R1M-|---+
 * recorder                |             |    |     |CA \         |    |   |
 *                         |             |    |     |3140>-+-R22K-+-|+ |   |       TRG = CD40106 schmitt trigger
 *                    +----+----+      R2.2K R22K   |   /  |        |CA \  |
 *                  AA119  |    |        |    |   +-|- |   |        |3140>-+-R1K-+-TRG-C33n-+-TRG--|
 *                    ^   C22n  v        +----+   |    |   |        |   /        |          |      |& o-TRG-TRG-o B6
 *                    |    |  AA119    + |    |   +-R1M|---+  +-----|- |         +-TRG------|------|
 *                    +----+----+      C22u R2.2K +----|-R33K-+        |       R18K        R10K    7400
 *                         |           - |    |        |    C0.1u      |         |          |
 *  GND--------------------+-------------+----+--------+------+--------+---------+----------+
 *
 *  Implementation is not going to emulate the analog specifics of this schematic
 *
 *-===============================================================================================================-*/

#ifndef MAME_BUS_MODULAB_CASSETTE_H
#define MAME_BUS_MODULAB_CASSETTE_H

#pragma once

#include "parallel.h"
#include "imagedev/cassette.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class modulab_cassette_device:
	public device_t,
	public device_modulab_parallel_interface
{
public:
	// construction/destruction
	modulab_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(cassette_w);

protected:
	virtual void device_start() override;
	//virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual uint8_t portb_r() override;
	virtual void portb_w(uint8_t data) override;

	optional_device<cassette_image_device> m_cassette;
};

// device type definition
DECLARE_DEVICE_TYPE(MODULAB_CASSETTE, modulab_cassette_device)

#endif // MAME_BUS_MODULAB_CASSETTE_H
