// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom
/***************************************************************************

  Dynabyte BASIC CONTROLLER 

  TODO:
    - Set up basic memory map
    - Dump ROM:s and load them into emulator
    - TBC

  Features
  --------
  4-16Kb of RAM
  4 Kb of EPROM with programming capabilities
  ZIBL in ROM
  2 serial ports (bit banged)
  1 parallel i/o port
  A 64x16 character video terminal
  1 Keyboard input port
  32 TTL flag outputs
  32 sense inputs
  8 relays
  8 indicator LED:s
  audio cassette i/o

  History of Dynabyte
  -------------------
  Dynabyte was founded by Michael Watts in California 1976 "to make add-in memory for HP
  minicomputers. Later extended products into S100 bus microcomputer market and also into
  the BASIC CONTROLLER, the worlds first single board computer with an embedded version 
  of BASIC, specialized for industrial controll applications" according to his LinkedIn.
  In 1983 Dynabyte was sold to Action Instruments Inc, an industrial controll company which 
  was investing "a million" dollars in the BASIC CONTROLLER and transformed it into the 
  $5000 "Action Pak" equipped with ABLE (Action Basic Language Enhancement), the successor
  of ZIBL (Z80 Industrial Basic Language). Action Instruments was aquired by Eurotherm 1998
  which then was aquired by Invensys, see http://www.jimpinto.com/enews/oct10-2002.html 

****************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"

class dynabc_state : public driver_device
{
public:
	dynabc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		{ }

private:
	bool m_irq;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(dynabc_mem, AS_PROGRAM, 8, dynabc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xbfff ) AM_RAM // 4-16Kb RAM
	AM_RANGE( 0xc000, 0xdfff ) AM_ROM // ZIBL
	AM_RANGE( 0xe000, 0xefff ) AM_ROM // User ROM
	//AM_RANGE( 0xfa00, 0xfdff ) AM_RAM  AM_SHARE("videoram") // Video RAM
	//AM_RANGE( 0xfe00, 0xfe3f ) // SENSE inputs
	//AM_RANGE( 0xfe40, 0xfe7f ) // ATOD inputs
	//AM_RANGE( 0xfe80, 0xfebf ) // FLAG outputs
	//AM_RANGE( 0xfec0, 0xfeff ) // LITES
	//AM_RANGE( 0xff00, 0xff3f ) // RELAYS
	//AM_RANGE( 0xff40, 0xff7f ) // DTOA outputs
	//AM_RANGE( 0xff80, 0xffbf ) // PORTS
	//AM_RANGE( 0xff80, 0xff80 ) // Bitbanging ports, 2 serial, 1 rtc, cassette, parallel port status
	//AM_RANGE( 0xff81, 0xff81 ) // Parallel port
	//AM_RANGE( 0xff82, 0xff82 ) // LITE port
	//AM_RANGE( 0xff83, 0xff83 ) // TAPE port
	//AM_RANGE( 0xff84, 0xff84 ) // Keyboard port
	//AM_RANGE( 0xffc0, 0xffff ) // User defined
ADDRESS_MAP_END

static ADDRESS_MAP_START( dynabc_io, AS_IO, 8, dynabc_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( dynabc )
INPUT_PORTS_END

void dynabc_state::machine_reset()
{
	m_irq = 0;
}
  
static MACHINE_CONFIG_START( dynabc )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_2_5MHz)
	MCFG_CPU_PROGRAM_MAP(dynabc_mem)
	MCFG_CPU_IO_MAP(dynabc_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dynabc )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ziblec000.bin",  0x0000, 0x800, NO_DUMP )
	ROM_LOAD( "ziblec800.bin",  0x0800, 0x800, NO_DUMP )
	ROM_LOAD( "zibled000.bin",  0x1000, 0x800, NO_DUMP )
	ROM_LOAD( "zibled800.bin",  0x1800, 0x800, NO_DUMP )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    STATE          INIT  COMPANY             FULLNAME                 FLAGS */
COMP( 1978, dynabc,  0,     0,      dynabc,   dynabc, dynabc_state, 0,    "Dynabyte Inc", "BASIC CONTROLLER", MACHINE_IS_SKELETON)
