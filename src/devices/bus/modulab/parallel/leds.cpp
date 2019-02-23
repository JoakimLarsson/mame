// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-==============================================-
 *
 *   Modulab IC-D6A, 6 leds module 8004-450
 *
 *-==============================================-*/

#include "emu.h"
#include "parallel.h"
#include "leds.h"

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MODULAB_LEDS, modulab_leds_device, "mlleds", "Modulab IC-D6A, 6 LEDs board")

//****************
//  LIVE DEVICE
//****************
modulab_leds_device::modulab_leds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MODULAB_LEDS, tag, owner, clock)
	, device_modulab_parallel_interface(mconfig, *this)
#if 0
	, m_leds(*this, "led%u", 0U)
#endif
	, m_leda(*this, "leda")
	, m_ledb(*this, "ledb")
	, m_ledc(*this, "ledc")
	, m_ledd(*this, "ledd")
	, m_lede(*this, "lede")
	, m_ledf(*this, "ledf")
	, m_bits(0)
{
}

void modulab_leds_device::device_start()
{
  //	m_leds.resolve();
}

READ16_MEMBER(modulab_leds_device::leds_r)
{
  uint8_t m_leds = 0;
  m_leds |= (m_bits & (1 << m_leda->read())) == 0 ? 0x01 : 0x00;
  m_leds |= (m_bits & (1 << m_ledb->read())) == 0 ? 0x02 : 0x00;
  m_leds |= (m_bits & (1 << m_ledc->read())) == 0 ? 0x04 : 0x00;
  m_leds |= (m_bits & (1 << m_ledd->read())) == 0 ? 0x08 : 0x00;
  m_leds |= (m_bits & (1 << m_lede->read())) == 0 ? 0x10 : 0x00;
  m_leds |= (m_bits & (1 << m_ledf->read())) == 0 ? 0x20 : 0x00;

  return m_leds;
}

void modulab_leds_device::update_leds()
{
#if 0
  m_leds[0] = m_bits & (1 << m_leda->read());
  m_leds[1] = m_bits & (1 << m_ledb->read());
  m_leds[2] = m_bits & (1 << m_ledc->read());
  m_leds[3] = m_bits & (1 << m_ledd->read());
  m_leds[4] = m_bits & (1 << m_lede->read());
  m_leds[5] = m_bits & (1 << m_ledf->read());
#endif
}

void modulab_leds_device::porta_w(uint8_t data)
{
  m_bits &= 0xff00;
  m_bits |= data;
  update_leds();
}

void modulab_leds_device::portb_w(uint8_t data)
{
  m_bits &= 0x00ff;
  m_bits |= (data << 8);
  update_leds();
}

static INPUT_PORTS_START( modulab_leds_pins )
	PORT_START("leda")
	PORT_DIPNAME( 0x00000f, 0x00, "LED A")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
	PORT_START("ledb")
	PORT_DIPNAME( 0x0f, 0x01, "LED B")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
	PORT_START("ledc")
	PORT_DIPNAME( 0x0f, 0x02, "LED C")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
	PORT_START("ledd")
	PORT_DIPNAME( 0x0f, 0x03, "LED D")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
	PORT_START("lede")
	PORT_DIPNAME( 0x0f, 0x04, "LED E")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
	PORT_START("ledf")
	PORT_DIPNAME( 0x0f, 0x05, "LED F")
	PORT_DIPSETTING( 0x00, "PA0" )
	PORT_DIPSETTING( 0x01, "PA1" )
	PORT_DIPSETTING( 0x02, "PA2" )
	PORT_DIPSETTING( 0x03, "PA3" )
	PORT_DIPSETTING( 0x04, "PA4" )
	PORT_DIPSETTING( 0x05, "PA5" )
	PORT_DIPSETTING( 0x06, "PA6" )
	PORT_DIPSETTING( 0x07, "PA7" )
	PORT_DIPSETTING( 0x08, "PB0" )
	PORT_DIPSETTING( 0x09, "PB1" )
	PORT_DIPSETTING( 0x0A, "PB2" )
	PORT_DIPSETTING( 0x0B, "PB3" )
	PORT_DIPSETTING( 0x0C, "PB4" )
	PORT_DIPSETTING( 0x0D, "PB5" )
	PORT_DIPSETTING( 0x0E, "PB6" )
	PORT_DIPSETTING( 0x0F, "PB7" )
INPUT_PORTS_END


ioport_constructor modulab_leds_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( modulab_leds_pins );
}
