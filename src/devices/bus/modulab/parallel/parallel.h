// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-==============================================-
 *
 *   Modulab lab pins
 *
 *   The Modulab board has 2 eight bit port pins directly connected to the INS8154 through breadboard wires
 *   In addition there are GND, VCC and an IRQ pin scattered over the board as well as two pins marked R of
 *   for now unknown use. This device deals with PORT A, PORT B and the IRQ pins below.
 *   See didact.cpp for additional info.
 *
 *   The Modulab CPU board, by Didact/Esselte ca 1984
 *  __________________________________________________________________________________________
 * |                                                    ADRESS               DATA             |
 * |              PORT A                      +-_--++-_--++-_--++-_--+   +-_--++-_--+   VCC   |
 * |    o   o   o   o   o   o   o   o         || | ||| | ||| | ||| | |   || | ||| | |    O    |
 * |    7   6   5   4   3   2   1   0         | -  || -  || -  || -  |   | -  || -  |         |
 * |    o   o   o   o   o   o   o   o         ||_|.|||_|.|||_|.|||_|.|   ||_|.|||_|.|   GND   |
 * |              PORT B                      +----++----++----++----+   +----++----+    O    |
 * |  o VCC                                    +--+  +--+  +--+  +--+     +--+  +--+          |
 * |                                           |LS|  |LS|  |LS|  |LS|     |LS|  |LS|          |
 * |  o GND                                    |164  |164  |164  |164     |164  |164          |
 * \\                                          |-5|<-|-4|<-|-3|<-|-2| <-  |-1|<-|-0|<- DB0    |
 * |\\ ____                                    +--+  +--+  +--+  +--+     +--+  +--+          |
 * | \/o  O|                                          +-------+-------+-------+-------+-------+
 * | |     |E           +--------------------+ +--+   |       |       |       |       |       |
 * | |     |X   +----+  |  PIA + 128x8 SRAM  | |LS|   |  RUN  |  ADS  |  FWD  | C/B   | RESET |
 * | |     |P   |4MHz|  |  INS8154N          | |14|   |       |       |       |       |       |
 * | |     |A   |XTAL|  +--------------------+ |  |   +-------+-------+-------+-------+-------+
 * | |     |N   |____|                         +--+   |       |       |       |       |       |
 * | |__   |S    |  |   +--------------------+ +--+   |   C   |   D   |   E   |   F   |       |
 * |  __|  |I           |  CPU               | |LS|   |       |       |       |       |       |
 * | |     |O           |  MC6802P           | |138   +-------+-------+-------+-------+       |
 * | |     |N           +--------------------+ |  |   |       |       |       |       |       |
 * | |     |B                                  +--+   |   8   |   9   |   A   |   B   |       |
 * | |     |U    IRQ    +-------------+        +--+   |       |       |       |       |       |
 * | |     |S    o      |  EPROM      |        |74|   +-------+-------+-------+-------+       |
 * | /\o  O|            |  2764       |        |C |   |       |       |       |       |  R o  |
 * |// ----             +-------------+        |923   |   4   |   5   |   6   |   7   |       |
 * //                     +-----------+        |  |   |       |       |       |       |  R o  |
 * |                      | 2KB SRAM  |        +--+   +-------+-------+-------+-------+       |
 * |                      | 6116      |        +--+   |       |       |       |       |       |
 * |                      +-----------+        |LS|   |   0   |   1   |   2   |   3   |       |
 * | ESSELTE       +-------+ +---+ +--------+  |138   |       |       |       |       |       |
 * | STUDIUM       |74LS123| |TRM| |SN74367 |  |  |   +-------+-------+-------+-------+       |
 * |               +-------+ +---+ +--------+  +--+
 * |__________________________________________________________________________________________|
 *
 */

#ifndef MAME_BUS_MODULAB_PARALLEL_H
#define MAME_BUS_MODULAB_PARALLEL_H

#pragma once

#define LOCAL_ARTWORK 0 // Awaiting suppor for sub-layouts

#include "labs.h"

class device_modulab_parallel_interface;

class modulab_parallel_slot_device : public device_t, public device_slot_interface
{
public:
  modulab_parallel_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
	: modulab_parallel_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		modulab_parallel_slot_labs(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}
	modulab_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~modulab_parallel_slot_device();
	
	virtual uint16_t leds_r(); // Read status of all leads/lamps of a laboration to update layout

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(portb_w);
 private:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	device_modulab_parallel_interface *m_dev;
};

class device_modulab_parallel_interface : public device_slot_card_interface
{
 public:
  virtual ~device_modulab_parallel_interface();
  virtual uint8_t porta_r() { return 0xff; } // Should reflect unloaded port pins
  virtual void porta_w(uint8_t data) { }
  virtual uint8_t portb_r() { return 0xff; } // Should reflect unloaded port pins
  virtual void portb_w(uint8_t data) { }
  virtual uint16_t leds_r() { return 0x00; } // Should reflect status of leds/lamps for a prticular laboration
 protected:
  device_modulab_parallel_interface(const machine_config &mconfig, device_t &device);

  modulab_parallel_slot_device *m_slot;
};

// device type
DECLARE_DEVICE_TYPE(MODULAB_PARALLEL_SLOT, modulab_parallel_slot_device)

#endif // MAME_BUS_MODULAB_PARALLEL_H
