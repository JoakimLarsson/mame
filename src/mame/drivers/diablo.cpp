// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*

 Diablo Printer Series 1300 HyType II driver

 - Microprocessor based control logic for increased capacity and flexibility, plus provision for implementation of additional features.
 - Advanced servo design for improved efficiency and performance.
 - Rigid one piece cast aluminum frame to better maintain print quality, and reduce maintenance requirements.
 - Rugged highly stable carriage assembly for enhanced print position accuracy and reduced maintenance.
 - Plug-in interchangeable printed pircuit boards (PCB's), readily accessible for ease and simplicity of service, and implementation
   of options and interfaces.
 - Operator control of print hammer energy (Impression Control Switch) to shift the printer's internal hammer energy scale up
   for multiple carbon forms or down for smaller lighter print font styles.
 - 1/120 inch (.212 mm) horizontal spacing on command.
 - 88/82 or 96 character metal wheel
 - Optional interface access to directly address print hammer energy levels character by character.
 - Optional interface access to command ribbon advance.
 - Optional Paper Out Switch installation for either normal top or an optional bottom paper feed.
 - Optional Cover Open Switch installation.
 - Optional End Of Ribbon sensor installation for use with mu1tistrike carbon ribbon cartridges which are not the recirculating type.
 - Carriage Return takes max 300 mS
 - Tabulation can be set as right or left
 - Column spacing 60 pt/inch by operator or 120 pt/inch by controller
 - Print Line: 13.1 inch (332.74mm)
 - Paper Feed: 4 inch/sec
 - Dimensions: 591x365x400mm
 - Weight: 12Kg

Model performance
-----------------
                     1345A    1355HS    1355WP
Print Speed char/sec  45       55        40
Character Set         96       96        88/92/96

Configurations
--------------
There are many options that comes with the Diablo 1300 series and while many are mechanical the electronics are built up with cards
interconnected by a backplane. The backplane has well defined slots for each type of cards and there are also many external cables
between the cards, sensors and motors of the printer. The backplane consists of up to 8 female connectors for 56 signals card edge 
connectors numbered A-H ordered in two rows, D,C,B,A on top with the fans to the left and H,G,F,E bellow. The signals are routed as 
needed and the slots are NOT generic, a specific card goes in at a specific slot but can be interchanged to accomodate improved
performance or replaced for repair. Slots E and F are used for feature expansions such as serial, network cards etc. 

The slots are populated as follows:

A: Logic #1 Command buffering and host signalling over a 50 pin ribbon cable. Sends commands to Logic #2 as needed
B: Logic #2 TTL CPU that interpretes commands from Logic #1 and controls all motors in the system
C: Servo
D: Carriage Power Amp
E: Optional 8080/Z80interface board, connects to Logic #1 board acting as host over the bus or the 50 pin ribbon cable
F: Optional slot with all signals of slot F
G: Transducer
H: Print Wheel Power Amp

In case the serial/IEEE488/network interface card is missing in the printer the host computer is supposed to drive which 
connects to the printer over the 50 pin ribbon cable instead of the printer hosted interface card.

Logic #1 Card - printer command management
------------------------------------------
The board is marked 40505 and has an option field at the top and a J7 connector for the 50 pin ribbon cable. It produces the 
system clock of 5 MHz that is used by the TTL CPU at Logic #2 Card,

 Identified IC:s
 ---------------
 1 74LS221       Dual Monostable multivibrator
 7 74LS74   7907-7908 Dual D-type pos edg trg flip-flops w clr and preset
 3 74LS367  7849 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 1 7451     7849 Dual AND+OR invert gates
 1 7486     7849 Quad XOR gates
 3 74LS170  7906 4 by 4 register file
 4 8837     7736 
 2 7408     7906 Quad AND gales
 2 74LS42   7906 BCD to decimal decoder
 1 7426     7906 Quad NAND gates
 1 74LS174  7836 Hex D-type flip flops
 1 7432     7901 QUAD OR gates
 2 74LSI07  7906 Dual J-K M/S flip flops w clear
 1 7404     7901 Hex Inverters
 5 75452    7840-7901
 2 7400     7849 Quad NAND gates

Logic #2 Card - printer command execution (TTL CPU)
---------------------------------------------------
The board is marked 40510 and has no connectors except the 56 signal bus edge connector

 Identified IC:s
 ---------------
 4 7400     7848-7902 Quad NAND gates
 3 74LS04   7850 Hex Inverters
 1 7408     7901 Quad AND gales
 1 7410     7840 Tripple 3-input NAND gates
 2 7453     7903 Expandable 4 wide AND+OR invert gates
 1 74LS74   7908 Dual D-type pos edg trg flip-flops w clr and preset
 2 74LS83   7901 4 bit binary full addres with fast carry
 4 74S289        4x16 bit RAM
 1 74107         Dual J-K M/S flip flops w clear
 1 74LS155  7731 1/2/3 to 4/8 lines decoder nwih totem pole ouputs
 2 74161    7904 synchronous binary 4 bit counter 
 4 74LS259  7906 8 bit addressable latches
 4 74298    7849 Quad 2 input mux with storage
 1 74367    7840 Non inverted 3 state outputs, 2 and 4 line enabled inputs
 1 74LS174       Hex D-type flip flops

RS232 Serial Interface Card
----------------------------
The serial interface card is z80 based and marked DIABLO-1300-V24

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


 Address decoding
 ----------------
 Z80 A0 30 -> 74367 -> Z80 PIO* Port A/B     6
 Z80 A1 31 -> 74367 -> Z80 PIO* Control/Data 5
 (Z80 A5 35 -> 74367) OR (Z80 IORQ 20) -> Z80 PIO1 CE* 4
 (Z80 A4 34 -> 74367) OR (Z80 IORQ 20) -> Z80 PIO2 CE* 4
*/

#include "emu.h"

#if 0
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
#endif

GAME( 1979, diablo,  0, diablo, diablo, diablo_state,  0, 0, "Diablo", "Diablo 1355WP", MACHINE_IS_SKELETON )
