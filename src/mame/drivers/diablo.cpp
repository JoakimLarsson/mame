// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*

 WIP Diablo 630 daisy wheel printer driver

 This mother of all daisy wheel printers has its own TTL CPU board to be dumped and emulated at some point.

 There is an optional z80 based i/o board which is currently emulated here

 The serial board is marked DIABLO-1300-V24

 Identified ICs:
 ---------------
 1 Z80-CPU 7904 Zilog CPU
 1 TMS2716 7906 2KB EPROM
 1 AM9551  7850 8251 USART
 2 Z80-PIO 7852 Zilog Paralell IO interface
10 74367   7845 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 2 UPB7400 7845 Quad NAND gates
 3 7432N   7832 QUAD OR gates 
 1 1489    7841 Quad line receivers
 1 1488    7823 Quad line tranceivers
 1 74163   7827 Synchrounous 4 bit counters
 2 7493    7822 4 bit binary counters
 2 7404    7849 Hex inverters
 1 7410    7849 Tripple 3-input NAND gates
 2 2114         1024 x 4 bit SRAM
 1 9602    7423 Dual retriggable resetable one shots
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/i8251.h"

class diablo_state : public driver_device
{
public:
	diablo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio1(*this, "pio1")
		, m_pio2(*this, "pio2")
		, m_usart(*this, "usart")
  { }
  
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio1;
	required_device<z80pio_device> m_pio2;
	required_device<i8251_device> m_usart;
public:
	void diablo(machine_config &config);
};

static ADDRESS_MAP_START( diablo_map, AS_PROGRAM, 8, diablo_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_RAM
ADDRESS_MAP_END

// Also reading from io 74 and b4, probably latches or similar
static ADDRESS_MAP_START( diablo_io_map, AS_IO, 8, diablo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xd4, 0xd7) AM_DEVREADWRITE("pio1", z80pio_device, read, write)
	AM_RANGE(0xe4, 0xe7) AM_DEVREADWRITE("pio2", z80pio_device, read, write)
	AM_RANGE(0xf0, 0xf0) AM_DEVREADWRITE("usart", i8251_device, data_r, data_w)
	AM_RANGE(0xf1, 0xf1) AM_DEVREADWRITE("usart", i8251_device, status_r, control_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( diablo )
INPUT_PORTS_END

void diablo_state::machine_start()
{
}

void diablo_state::machine_reset()
{
}

MACHINE_CONFIG_START( diablo_state::diablo )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_1_6896MHz)
	MCFG_CPU_PROGRAM_MAP(diablo_map)
	MCFG_CPU_IO_MAP(diablo_io_map)

        MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL_1_6896MHz)
//MCFG_Z80PIO_IN_PB_CB(READ8(z1013_state, port_b_r))
//MCFG_Z80PIO_OUT_PB_CB(WRITE8(z1013_state, port_b_w))
        MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL_1_6896MHz)
//MCFG_Z80PIO_IN_PB_CB(READ8(z1013_state, port_b_r))
//MCFG_Z80PIO_OUT_PB_CB(WRITE8(z1013_state, port_b_w))

        MCFG_DEVICE_ADD("usart", I8251, XTAL_1_6896MHz)
#if 0
        MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
        MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
        MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))

        MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, "terminal")
        MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_B_TAG, i8251_device, write_rxd))
        MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_B_TAG, i8251_device, write_dsr))
        MCFG_RS232_CTS_HANDLER(DEVWRITELINE(I8251_B_TAG, i8251_device, write_cts))
#endif
MACHINE_CONFIG_END


ROM_START( diablo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ok1.bin", 0x0000, 0x800, CRC(59236340) SHA1(7548ad0ad32cedad856a158228d3e24c5ebf542d) )
ROM_END

  GAME( 19??, diablo,  0, diablo, diablo, diablo_state,  0, 0, "Diablo", "Diablo 630", MACHINE_IS_SKELETON )
