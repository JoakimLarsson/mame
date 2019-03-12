// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-==============================================-
 *
 *   Modulab lab pins, 2 x 8 bit + IRQ
 *
 *-==============================================-*/

#include "emu.h"
#include "parallel.h"

// Device definition
DEFINE_DEVICE_TYPE(MODULAB_PARALLEL_SLOT, modulab_parallel_slot_device, "modulab_parallel_slot_device", "Modulab Parallel Lab Port")

//
// Slot device
//

// Constructor
modulab_parallel_slot_device::modulab_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MODULAB_PARALLEL_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_dev(nullptr)
{
}

// Destructor
modulab_parallel_slot_device::~modulab_parallel_slot_device()
{
}

// device_start
void modulab_parallel_slot_device::device_start()
{
	m_dev = dynamic_cast<device_modulab_parallel_interface *>(get_card_device());
}

// device_reset
void modulab_parallel_slot_device::device_reset()
{
}

// Ports
READ8_MEMBER( modulab_parallel_slot_device::porta_r )
{
	if (m_dev)
		return m_dev->porta_r();
	else
		return 0xff;
}

WRITE8_MEMBER( modulab_parallel_slot_device::porta_w )
{
	if (m_dev)
		m_dev->porta_w(data);
}

READ8_MEMBER( modulab_parallel_slot_device::portb_r )
{
	if (m_dev)
		return m_dev->portb_r();
	else
		return 0xff;
}

WRITE8_MEMBER( modulab_parallel_slot_device::portb_w )
{
	if (m_dev)
		m_dev->portb_w(data);
}

uint16_t modulab_parallel_slot_device::leds_r()
{
	if (m_dev)
		return m_dev->leds_r();
	else
		return 0x00;
}

//
// Port interface
//

//  constructor
device_modulab_parallel_interface::device_modulab_parallel_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<modulab_parallel_slot_device *>(device.owner());
}

//  destructor
device_modulab_parallel_interface::~device_modulab_parallel_interface()
{
}
