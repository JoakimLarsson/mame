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
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void isa8_myb3k_com_device::device_start()
{
  // TODO: Read A10-A10 base address from DPSW1, now using factory setting
	set_isa_device();
	m_isa->install_device(0x0530, 0x0530,
			      read8_delegate(FUNC(i8251_device::data_r), subdevice<i8251_device>("usart0")),
			      write8_delegate(FUNC(i8251_device::data_w), subdevice<i8251_device>("usart0")) );
	m_isa->install_device(0x0531, 0x0532,
			      read8_delegate(FUNC(i8251_device::status_r), subdevice<i8251_device>("usart0")),
			      write8_delegate(FUNC(i8251_device::control_w), subdevice<i8251_device>("usart0")) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_myb3k_com_device::device_reset()
{
}

WRITE_LINE_MEMBER(isa8_myb3k_com_device::myb3k_com_int)
{
	// TODO: Read switch DPSW2 and interrupt accordingly 
	m_isa->irq2_w(state); // If DPSW2:1 is ON
	m_isa->irq3_w(state); // If DPSW2:2 is ON
	m_isa->irq4_w(state); // If DPSW2:3 is ON
	m_isa->irq5_w(state); // If DPSW2:4 is ON
	// Schematics allows for more than one interrupt to be triggered 
}
