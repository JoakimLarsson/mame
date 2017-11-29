// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/********************************************************************************

  ADP 4703 ISA 8 bit RS232C Adapter Card for Step/One and possibly MyBrain 3000 and JB-3000.

TODO:
 - Check if this board was manufactured by Matsushita or specific to the Step/One system
 - Add missing logic
 - Fix interrupt handler
 - Add configuration switches
 - Install card according to switch settings into io map
 - Put it into global ISA8 collection

*********************************************************************************/

#include "emu.h"
#include "myb3k_com.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/null_modem.h"
#include "machine/i8251.h"

static SLOT_INTERFACE_START(isa8_myb3k_com)
	SLOT_INTERFACE("null_modem", NULL_MODEM)
SLOT_INTERFACE_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
  
DEFINE_DEVICE_TYPE(ISA8_MYB3K_COM, isa8_myb3k_com_device, "isa8_myb3k_com", "ADP 4703 RS-232C Adapter")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
MACHINE_CONFIG_MEMBER( isa8_myb3k_com_device::device_add_mconfig )
	MCFG_DEVICE_ADD( "usart0", I8251, XTAL_15_9744MHz / 8 )
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("com1", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("com1", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("com1", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(isa8_myb3k_com_device, myb3k_com_int))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(isa8_myb3k_com_device, myb3k_com_int))

	MCFG_RS232_PORT_ADD( "com1", isa8_myb3k_com, nullptr )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("usart0", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("usart0", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("usart0", i8251_device, write_cts))
MACHINE_CONFIG_END

// PORT definitions moved to the end of this file as it became very long

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_myb3k_com_device - constructor
//-------------------------------------------------
isa8_myb3k_com_device::isa8_myb3k_com_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_myb3k_com_device(mconfig, ISA8_MYB3K_COM, tag, owner, clock)
{
}

isa8_myb3k_com_device::isa8_myb3k_com_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_iobase(*this, "DPSW1")
	, m_isairq(*this, "DPSW2")
	, m_installed(false)
	, m_irq(4)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void isa8_myb3k_com_device::device_start()
{
	// IO base factory setting is 0x540
	set_isa_device();
	m_installed = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void isa8_myb3k_com_device::device_reset()
{
	if (!m_installed)
	{
		uint32_t base = m_iobase->read();
		m_isa->install_device(base, base,
					read8_delegate(FUNC(i8251_device::data_r), subdevice<i8251_device>("usart0")),
					write8_delegate(FUNC(i8251_device::data_w), subdevice<i8251_device>("usart0")) );
		m_isa->install_device(base + 1, base + 1,
					read8_delegate(FUNC(i8251_device::status_r), subdevice<i8251_device>("usart0")),
					write8_delegate(FUNC(i8251_device::control_w), subdevice<i8251_device>("usart0")) );
		
		m_irq = m_isairq->read();
		m_installed = true;
	}
}

WRITE_LINE_MEMBER(isa8_myb3k_com_device::myb3k_com_int)
{
  // TODO: Implement proper interrupt logics 
	// Schematics allows for more than one interrupt to be triggered but there is probably only one jumper
	switch (m_irq)
	{
	case 2:	m_isa->irq2_w(state); break;
	case 3:	m_isa->irq3_w(state); break;
	case 4:	m_isa->irq4_w(state); break;
	case 5:	m_isa->irq5_w(state); break;
	}
}


//--------------------------------------------------------------------
//  Port definition - Needs refactoring as becoming ridiculously long
//--------------------------------------------------------------------
static INPUT_PORTS_START( myb3k_com_dpsw )
	PORT_START("DPSW2")
	PORT_DIPNAME( 0x0f, 0x04, "USART ISA IRQ")
 	PORT_DIPSETTING( 0x01, "IRQ2" )
 	PORT_DIPSETTING( 0x02, "IRQ3" )
 	PORT_DIPSETTING( 0x04, "IRQ4" )
 	PORT_DIPSETTING( 0x08, "IRQ5" )
	PORT_START("DPSW1")
	PORT_DIPNAME( 0x7fc, 0x540, "I/O Base address")
 	PORT_DIPSETTING( 0x000, "0x000" )
	PORT_DIPSETTING( 0x008, "0x008" )
	PORT_DIPSETTING( 0x010, "0x010" )
	PORT_DIPSETTING( 0x018, "0x018" )
	PORT_DIPSETTING( 0x020, "0x020" )
	PORT_DIPSETTING( 0x028, "0x028" )
	PORT_DIPSETTING( 0x030, "0x030" )
	PORT_DIPSETTING( 0x038, "0x038" )
	PORT_DIPSETTING( 0x040, "0x040" )
	PORT_DIPSETTING( 0x048, "0x048" )
	PORT_DIPSETTING( 0x050, "0x050" )
	PORT_DIPSETTING( 0x058, "0x058" )
	PORT_DIPSETTING( 0x060, "0x060" )
	PORT_DIPSETTING( 0x068, "0x068" )
	PORT_DIPSETTING( 0x070, "0x070" )
	PORT_DIPSETTING( 0x078, "0x078" )
	PORT_DIPSETTING( 0x080, "0x080" )
	PORT_DIPSETTING( 0x088, "0x088" )
	PORT_DIPSETTING( 0x090, "0x090" )
	PORT_DIPSETTING( 0x098, "0x098" )
	PORT_DIPSETTING( 0x0a0, "0x0a0" )
	PORT_DIPSETTING( 0x0a8, "0x0a8" )
	PORT_DIPSETTING( 0x0b0, "0x0b0" )
	PORT_DIPSETTING( 0x0b8, "0x0b8" )
	PORT_DIPSETTING( 0x0c0, "0x0c0" )
	PORT_DIPSETTING( 0x0c8, "0x0c8" )
	PORT_DIPSETTING( 0x0d0, "0x0d0" )
	PORT_DIPSETTING( 0x0d8, "0x0d8" )
	PORT_DIPSETTING( 0x0e0, "0x0e0" )
	PORT_DIPSETTING( 0x0e8, "0x0e8" )
	PORT_DIPSETTING( 0x0f0, "0x0f0" )
	PORT_DIPSETTING( 0x0f8, "0x0f8" )
	PORT_DIPSETTING( 0x100, "0x100" )
	PORT_DIPSETTING( 0x108, "0x108" )
	PORT_DIPSETTING( 0x110, "0x110" )
	PORT_DIPSETTING( 0x118, "0x118" )
	PORT_DIPSETTING( 0x120, "0x120" )
	PORT_DIPSETTING( 0x128, "0x128" )
	PORT_DIPSETTING( 0x130, "0x130" )
	PORT_DIPSETTING( 0x138, "0x138" )
	PORT_DIPSETTING( 0x140, "0x140" )
	PORT_DIPSETTING( 0x148, "0x148" )
	PORT_DIPSETTING( 0x150, "0x150" )
	PORT_DIPSETTING( 0x158, "0x158" )
	PORT_DIPSETTING( 0x160, "0x160" )
	PORT_DIPSETTING( 0x168, "0x168" )
	PORT_DIPSETTING( 0x170, "0x170" )
	PORT_DIPSETTING( 0x178, "0x178" )
	PORT_DIPSETTING( 0x180, "0x180" )
	PORT_DIPSETTING( 0x188, "0x188" )
	PORT_DIPSETTING( 0x190, "0x190" )
	PORT_DIPSETTING( 0x198, "0x198" )
	PORT_DIPSETTING( 0x1a0, "0x1a0" )
	PORT_DIPSETTING( 0x1a8, "0x1a8" )
	PORT_DIPSETTING( 0x1b0, "0x1b0" )
	PORT_DIPSETTING( 0x1b8, "0x1b8" )
	PORT_DIPSETTING( 0x1c0, "0x1c0" )
	PORT_DIPSETTING( 0x1c8, "0x1c8" )
	PORT_DIPSETTING( 0x1d0, "0x1d0" )
	PORT_DIPSETTING( 0x1d8, "0x1d8" )
	PORT_DIPSETTING( 0x1e0, "0x1e0" )
	PORT_DIPSETTING( 0x1e8, "0x1e8" )
	PORT_DIPSETTING( 0x1f0, "0x1f0" )
	PORT_DIPSETTING( 0x1f8, "0x1f8" )
	PORT_DIPSETTING( 0x200, "0x200" )
	PORT_DIPSETTING( 0x208, "0x208" )
	PORT_DIPSETTING( 0x210, "0x210" )
	PORT_DIPSETTING( 0x218, "0x218" )
	PORT_DIPSETTING( 0x220, "0x220" )
	PORT_DIPSETTING( 0x228, "0x228" )
	PORT_DIPSETTING( 0x230, "0x230" )
	PORT_DIPSETTING( 0x238, "0x238" )
	PORT_DIPSETTING( 0x240, "0x240" )
	PORT_DIPSETTING( 0x248, "0x248" )
	PORT_DIPSETTING( 0x250, "0x250" )
	PORT_DIPSETTING( 0x258, "0x258" )
	PORT_DIPSETTING( 0x260, "0x260" )
	PORT_DIPSETTING( 0x268, "0x268" )
	PORT_DIPSETTING( 0x270, "0x270" )
	PORT_DIPSETTING( 0x278, "0x278" )
	PORT_DIPSETTING( 0x280, "0x280" )
	PORT_DIPSETTING( 0x288, "0x288" )
	PORT_DIPSETTING( 0x290, "0x290" )
	PORT_DIPSETTING( 0x298, "0x298" )
	PORT_DIPSETTING( 0x2a0, "0x2a0" )
	PORT_DIPSETTING( 0x2a8, "0x2a8" )
	PORT_DIPSETTING( 0x2b0, "0x2b0" )
	PORT_DIPSETTING( 0x2b8, "0x2b8" )
	PORT_DIPSETTING( 0x2c0, "0x2c0" )
	PORT_DIPSETTING( 0x2c8, "0x2c8" )
	PORT_DIPSETTING( 0x2d0, "0x2d0" )
	PORT_DIPSETTING( 0x2d8, "0x2d8" )
	PORT_DIPSETTING( 0x2e0, "0x2e0" )
	PORT_DIPSETTING( 0x2e8, "0x2e8" )
	PORT_DIPSETTING( 0x2f0, "0x2f0" )
	PORT_DIPSETTING( 0x2f8, "0x2f8" )
	PORT_DIPSETTING( 0x300, "0x300" )
	PORT_DIPSETTING( 0x308, "0x308" )
	PORT_DIPSETTING( 0x310, "0x310" )
	PORT_DIPSETTING( 0x318, "0x318" )
	PORT_DIPSETTING( 0x320, "0x320" )
	PORT_DIPSETTING( 0x328, "0x328" )
	PORT_DIPSETTING( 0x330, "0x330" )
	PORT_DIPSETTING( 0x338, "0x338" )
	PORT_DIPSETTING( 0x340, "0x340" )
	PORT_DIPSETTING( 0x348, "0x348" )
	PORT_DIPSETTING( 0x350, "0x350" )
	PORT_DIPSETTING( 0x358, "0x358" )
	PORT_DIPSETTING( 0x360, "0x360" )
	PORT_DIPSETTING( 0x368, "0x368" )
	PORT_DIPSETTING( 0x370, "0x370" )
	PORT_DIPSETTING( 0x378, "0x378" )
	PORT_DIPSETTING( 0x380, "0x380" )
	PORT_DIPSETTING( 0x388, "0x388" )
	PORT_DIPSETTING( 0x390, "0x390" )
	PORT_DIPSETTING( 0x398, "0x398" )
	PORT_DIPSETTING( 0x3a0, "0x3a0" )
	PORT_DIPSETTING( 0x3a8, "0x3a8" )
	PORT_DIPSETTING( 0x3b0, "0x3b0" )
	PORT_DIPSETTING( 0x3b8, "0x3b8" )
	PORT_DIPSETTING( 0x3c0, "0x3c0" )
	PORT_DIPSETTING( 0x3c8, "0x3c8" )
	PORT_DIPSETTING( 0x3d0, "0x3d0" )
	PORT_DIPSETTING( 0x3d8, "0x3d8" )
	PORT_DIPSETTING( 0x3e0, "0x3e0" )
	PORT_DIPSETTING( 0x3e8, "0x3e8" )
	PORT_DIPSETTING( 0x3f0, "0x3f0" )
	PORT_DIPSETTING( 0x3f8, "0x3f8" )
	PORT_DIPSETTING( 0x400, "0x400" )
	PORT_DIPSETTING( 0x408, "0x408" )
	PORT_DIPSETTING( 0x410, "0x410" )
	PORT_DIPSETTING( 0x418, "0x418" )
	PORT_DIPSETTING( 0x420, "0x420" )
	PORT_DIPSETTING( 0x428, "0x428" )
	PORT_DIPSETTING( 0x430, "0x430" )
	PORT_DIPSETTING( 0x438, "0x438" )
	PORT_DIPSETTING( 0x440, "0x440" )
	PORT_DIPSETTING( 0x448, "0x448" )
	PORT_DIPSETTING( 0x450, "0x450" )
	PORT_DIPSETTING( 0x458, "0x458" )
	PORT_DIPSETTING( 0x460, "0x460" )
	PORT_DIPSETTING( 0x468, "0x468" )
	PORT_DIPSETTING( 0x470, "0x470" )
	PORT_DIPSETTING( 0x478, "0x478" )
	PORT_DIPSETTING( 0x480, "0x480" )
	PORT_DIPSETTING( 0x488, "0x488" )
	PORT_DIPSETTING( 0x490, "0x490" )
	PORT_DIPSETTING( 0x498, "0x498" )
	PORT_DIPSETTING( 0x4a0, "0x4a0" )
	PORT_DIPSETTING( 0x4a8, "0x4a8" )
	PORT_DIPSETTING( 0x4b0, "0x4b0" )
	PORT_DIPSETTING( 0x4b8, "0x4b8" )
	PORT_DIPSETTING( 0x4c0, "0x4c0" )
	PORT_DIPSETTING( 0x4c8, "0x4c8" )
	PORT_DIPSETTING( 0x4d0, "0x4d0" )
	PORT_DIPSETTING( 0x4d8, "0x4d8" )
	PORT_DIPSETTING( 0x4e0, "0x4e0" )
	PORT_DIPSETTING( 0x4e8, "0x4e8" )
	PORT_DIPSETTING( 0x4f0, "0x4f0" )
	PORT_DIPSETTING( 0x4f8, "0x4f8" )
	PORT_DIPSETTING( 0x500, "0x500" )
	PORT_DIPSETTING( 0x508, "0x508" )
	PORT_DIPSETTING( 0x510, "0x510" )
	PORT_DIPSETTING( 0x518, "0x518" )
	PORT_DIPSETTING( 0x520, "0x520" )
	PORT_DIPSETTING( 0x528, "0x528" )
	PORT_DIPSETTING( 0x530, "0x530" )
	PORT_DIPSETTING( 0x538, "0x538" )
	PORT_DIPSETTING( 0x540, "0x540" )
	PORT_DIPSETTING( 0x548, "0x548" )
	PORT_DIPSETTING( 0x550, "0x550" )
	PORT_DIPSETTING( 0x558, "0x558" )
	PORT_DIPSETTING( 0x560, "0x560" )
	PORT_DIPSETTING( 0x568, "0x568" )
	PORT_DIPSETTING( 0x570, "0x570" )
	PORT_DIPSETTING( 0x578, "0x578" )
	PORT_DIPSETTING( 0x580, "0x580" )
	PORT_DIPSETTING( 0x588, "0x588" )
	PORT_DIPSETTING( 0x590, "0x590" )
	PORT_DIPSETTING( 0x598, "0x598" )
	PORT_DIPSETTING( 0x5a0, "0x5a0" )
	PORT_DIPSETTING( 0x5a8, "0x5a8" )
	PORT_DIPSETTING( 0x5b0, "0x5b0" )
	PORT_DIPSETTING( 0x5b8, "0x5b8" )
	PORT_DIPSETTING( 0x5c0, "0x5c0" )
	PORT_DIPSETTING( 0x5c8, "0x5c8" )
	PORT_DIPSETTING( 0x5d0, "0x5d0" )
	PORT_DIPSETTING( 0x5d8, "0x5d8" )
	PORT_DIPSETTING( 0x5e0, "0x5e0" )
	PORT_DIPSETTING( 0x5e8, "0x5e8" )
	PORT_DIPSETTING( 0x5f0, "0x5f0" )
	PORT_DIPSETTING( 0x5f8, "0x5f8" )
	PORT_DIPSETTING( 0x600, "0x600" )
	PORT_DIPSETTING( 0x608, "0x608" )
	PORT_DIPSETTING( 0x610, "0x610" )
	PORT_DIPSETTING( 0x618, "0x618" )
	PORT_DIPSETTING( 0x620, "0x620" )
	PORT_DIPSETTING( 0x628, "0x628" )
	PORT_DIPSETTING( 0x630, "0x630" )
	PORT_DIPSETTING( 0x638, "0x638" )
	PORT_DIPSETTING( 0x640, "0x640" )
	PORT_DIPSETTING( 0x648, "0x648" )
	PORT_DIPSETTING( 0x650, "0x650" )
	PORT_DIPSETTING( 0x658, "0x658" )
	PORT_DIPSETTING( 0x660, "0x660" )
	PORT_DIPSETTING( 0x668, "0x668" )
	PORT_DIPSETTING( 0x670, "0x670" )
	PORT_DIPSETTING( 0x678, "0x678" )
	PORT_DIPSETTING( 0x680, "0x680" )
	PORT_DIPSETTING( 0x688, "0x688" )
	PORT_DIPSETTING( 0x690, "0x690" )
	PORT_DIPSETTING( 0x698, "0x698" )
	PORT_DIPSETTING( 0x6a0, "0x6a0" )
	PORT_DIPSETTING( 0x6a8, "0x6a8" )
	PORT_DIPSETTING( 0x6b0, "0x6b0" )
	PORT_DIPSETTING( 0x6b8, "0x6b8" )
	PORT_DIPSETTING( 0x6c0, "0x6c0" )
	PORT_DIPSETTING( 0x6c8, "0x6c8" )
	PORT_DIPSETTING( 0x6d0, "0x6d0" )
	PORT_DIPSETTING( 0x6d8, "0x6d8" )
	PORT_DIPSETTING( 0x6e0, "0x6e0" )
	PORT_DIPSETTING( 0x6e8, "0x6e8" )
	PORT_DIPSETTING( 0x6f0, "0x6f0" )
	PORT_DIPSETTING( 0x6f8, "0x6f8" )
	PORT_DIPSETTING( 0x700, "0x700" )
	PORT_DIPSETTING( 0x708, "0x708" )
	PORT_DIPSETTING( 0x710, "0x710" )
	PORT_DIPSETTING( 0x718, "0x718" )
	PORT_DIPSETTING( 0x720, "0x720" )
	PORT_DIPSETTING( 0x728, "0x728" )
	PORT_DIPSETTING( 0x730, "0x730" )
	PORT_DIPSETTING( 0x738, "0x738" )
	PORT_DIPSETTING( 0x740, "0x740" )
	PORT_DIPSETTING( 0x748, "0x748" )
	PORT_DIPSETTING( 0x750, "0x750" )
	PORT_DIPSETTING( 0x758, "0x758" )
	PORT_DIPSETTING( 0x760, "0x760" )
	PORT_DIPSETTING( 0x768, "0x768" )
	PORT_DIPSETTING( 0x770, "0x770" )
	PORT_DIPSETTING( 0x778, "0x778" )
	PORT_DIPSETTING( 0x780, "0x780" )
	PORT_DIPSETTING( 0x788, "0x788" )
	PORT_DIPSETTING( 0x790, "0x790" )
	PORT_DIPSETTING( 0x798, "0x798" )
	PORT_DIPSETTING( 0x7a0, "0x7a0" )
	PORT_DIPSETTING( 0x7a8, "0x7a8" )
	PORT_DIPSETTING( 0x7b0, "0x7b0" )
	PORT_DIPSETTING( 0x7b8, "0x7b8" )
	PORT_DIPSETTING( 0x7c0, "0x7c0" )
	PORT_DIPSETTING( 0x7c8, "0x7c8" )
	PORT_DIPSETTING( 0x7d0, "0x7d0" )
	PORT_DIPSETTING( 0x7d8, "0x7d8" )
	PORT_DIPSETTING( 0x7e0, "0x7e0" )
	PORT_DIPSETTING( 0x7e8, "0x7e8" )
	PORT_DIPSETTING( 0x7f0, "0x7f0" )
	PORT_DIPSETTING( 0x7f8, "0x7f8" )
INPUT_PORTS_END

ioport_constructor isa8_myb3k_com_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( myb3k_com_dpsw );
}

