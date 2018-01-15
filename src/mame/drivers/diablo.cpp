// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*

 WIP Diablo 630 daisy wheel printer driver

 This mother of all daisy wheel printers has its own TTL CPU board to be dumped and emulated at some point.

 There is an optional z80 based i/o board which is currently emulated here

*/

#include "emu.h"
#include "cpu/z80/z80.h"

class diablo_state : public driver_device
{
public:
	diablo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( diablo_map, AS_PROGRAM, 8, diablo_state )
	AM_RANGE(0x0000, 0x7ff) AM_ROM
	AM_RANGE(0x800, 0x47ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( diablo_io_map, AS_IO, 8, diablo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( diablo )
INPUT_PORTS_END

void diablo_state::machine_start()
{
}

void diablo_state::machine_reset()
{
}

static MACHINE_CONFIG_START( diablo )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(diablo_map)
	MCFG_CPU_IO_MAP(diablo_io_map)
MACHINE_CONFIG_END


ROM_START( diablo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ok1.bin", 0x0000, 0x800, CRC(59236340) SHA1(7548ad0ad32cedad856a158228d3e24c5ebf542d) )
ROM_END

  GAME( 19??, diablo,  0, diablo, diablo, diablo_state,  0, 0, "Diablo", "Diablo 630", MACHINE_IS_SKELETON )
