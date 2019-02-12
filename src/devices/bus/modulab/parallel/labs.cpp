// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************

    Modulab laborations

***************************************************************************/

#include "emu.h"
#include "labs.h"
#include "cassette.h"

void modulab_parallel_slot_labs(device_slot_interface &device)
{
	device.option_add("cassette", MODULAB_CASSETTE);
}
