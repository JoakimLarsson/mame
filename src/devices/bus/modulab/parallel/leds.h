// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-===============================================================================================================-
 *
 * Modulab leds board
 *
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

#ifndef MAME_BUS_MODULAB_LEDS_H
#define MAME_BUS_MODULAB_LEDS_H

#pragma once

#include "parallel.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class modulab_leds_device:
	public device_t,
	public device_modulab_parallel_interface
{
public:
	// construction/destruction
	modulab_leds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void porta_w(uint8_t data) override;
	virtual void portb_w(uint8_t data) override;
	void update_leds();
	DECLARE_READ8_MEMBER(leds_r);	
	//output_finder<6> m_leds;
#if 1
	required_ioport m_leda;
	required_ioport m_ledb;
	required_ioport m_ledc;
	required_ioport m_ledd;
	required_ioport m_lede;
	required_ioport m_ledf;
#endif
	uint16_t m_bits;
};

// device type definition
DECLARE_DEVICE_TYPE(MODULAB_LEDS, modulab_leds_device)

#endif // MAME_BUS_MODULAB_LEDS_H
