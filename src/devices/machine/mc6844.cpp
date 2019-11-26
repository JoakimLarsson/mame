// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/**********************************************************************

  Motorola 6844 emulation. 

 "MC6844 — Direct Memory Access Controller

  This DMAC works with an M6800 MPU Clock Pulse Generator and an I/O Peripheral Controller,
  such as the units described here, to facilitate  direct access to the computer memory by
  the peripheral, thus by passing MPU interactive time delay.

  General Description

  The MC6844 is operable in three modes: HALT Burst, Cycle Steal and TSC Steal. 
  In the Burst Mode, the MPU is halted by the first transfer request (TxRQ) input and
  is restarted when the Byte Count Register (BCR) is zero. Each data transfer is synchronized 
  by a pulse input of TxRQ. In the Cycle Steal Mode, the MPU is halted by each TxRQ and 
  is restarted after each one byte of data transferred. In the TSC Steal Mode, DMAC uses the 
  three-state control function of the MPU to control the system bus. One byte of data is
  transferred during each DMA cycle.

  The DMAC has four channels. A Priority Control Register determines which of the channels 
  is enabled. While data is being transferred on one channel, the other channels are inhibited.
  When one channel completes transferring, the next will become valid for DMA transfer. The PCR 
  also utilizes a Rotate Control bit. Priority of DMA transfer is normally fixed in sequential 
  order. The highest priority is in #0 Channel and the lowest is in #3. When this bit is in high
  level, channel priority is rotated such that the just-serviced channel has the lowest priority 
  in the next DMA transfer."   

  Source: https://en.wikipedia.org/wiki/File:Motorola_Microcomputer_Components_1978_pg13.jpg

  TODO:

**********************************************************************/

#include "emu.h"
#include "mc6844.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
#define LOG_SETUP   (1U << 1)
#define LOG_INT     (1U << 2)

#define VERBOSE (LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

// device type definition
DEFINE_DEVICE_TYPE(MC6844, mc6844_device, "mc6844", "MC6844 DMA")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc6844_device - constructor
//-------------------------------------------------
mc6844_device::mc6844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MC6844, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific machine
//  configuration addiitons
//-------------------------------------------------
void mc6844_device::device_add_mconfig(machine_config &config)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6844_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6844_device::device_reset()
{
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ8_MEMBER( mc6844_device::m6844_r )
{
	uint8_t result = 0;

	// switch off the offset we were given
	switch (offset)
	{
		// upper byte of address
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			result = m_m6844_channel[offset / 4].address >> 8;
			break;

		// lower byte of address
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			result = m_m6844_channel[offset / 4].address & 0xff;
			break;

		// upper byte of counter
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			result = m_m6844_channel[offset / 4].counter >> 8;
			break;

		// lower byte of counter
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			result = m_m6844_channel[offset / 4].counter & 0xff;
			break;

		// channel control
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			result = m_m6844_channel[offset - 0x10].control;

			// A read here clears the 'DMA end' flag of the
			// associated channel.
			if (!machine().side_effects_disabled())
			{
				m_m6844_channel[offset - 0x10].control &= ~0x80;
				if (m_m6844_interrupt & 0x80)
					m6844_update_interrupt();
			}
			break;

		// priority control
		case 0x14:
			result = m_m6844_priority;
			break;

		// interrupt control
		case 0x15:
			result = m_m6844_interrupt;
			break;

		// chaining control
		case 0x16:
			result = m_m6844_chain;
			break;

		// 0x17-0x1f not used
		default: break;
	}

	return result & 0xff;
}


WRITE8_MEMBER( mc6844_device::m6844_w )
{
	int i;

	// switch off the offset we were given
	switch (offset)
	{
		// upper byte of address
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff) | (data << 8);
			break;

		// lower byte of address
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff00) | (data & 0xff);
			break;

		// upper byte of counter
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff) | (data << 8);
			break;

		// lower byte of counter
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff00) | (data & 0xff);
			break;

		// channel control
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			m_m6844_channel[offset - 0x10].control = (m_m6844_channel[offset - 0x10].control & 0xc0) | (data & 0x3f);
			break;

		// priority control
		case 0x14:
			m_m6844_priority = data;

			// update each channel
			for (i = 0; i < 4; i++)
			{
				// if we're going active...
				if (!m_m6844_channel[i].active && (data & (1 << i)))
				{
					// mark us active
					m_m6844_channel[i].active = 1;

					// set the DMA busy bit and clear the DMA end bit
					m_m6844_channel[i].control |= 0x40;
					m_m6844_channel[i].control &= ~0x80;

					// set the starting address, counter, and time
					m_m6844_channel[i].start_address = m_m6844_channel[i].address;
					m_m6844_channel[i].start_counter = m_m6844_channel[i].counter;
				}

				// if we're going inactive...
				else if (m_m6844_channel[i].active && !(data & (1 << i)))
				{
					//mark us inactive
					m_m6844_channel[i].active = 0;
				}
			}
			break;

		// interrupt control
		case 0x15:
			m_m6844_interrupt = (m_m6844_interrupt & 0x80) | (data & 0x7f);
			m6844_update_interrupt();
			break;

		// chaining control
		case 0x16:
			m_m6844_chain = data;
			break;

		// 0x17-0x1f not used
		default: break;
	}
}

void mc6844_device::m6844_update_interrupt()
{
	uint8_t interrupt = 0;

	interrupt |= BIT(m_m6844_channel[0].control, 7) & BIT(m_m6844_interrupt, 0);
	interrupt |= BIT(m_m6844_channel[1].control, 7) & BIT(m_m6844_interrupt, 1);
	interrupt |= BIT(m_m6844_channel[2].control, 7) & BIT(m_m6844_interrupt, 2);
	interrupt |= BIT(m_m6844_channel[3].control, 7) & BIT(m_m6844_interrupt, 3);

	if (interrupt)
	{
		if (!(m_m6844_interrupt & 0x80))
		{
			// Set interrupt indication bit 7.
			m_m6844_interrupt |= 0x80;
			//swtpc09_irq_handler(DMAC_IRQ, ASSERT_LINE); // FIX: callback
		}
	}
	else
	{
		if (m_m6844_interrupt & 0x80)
		{
			// Clear interrupt indication bit 7.
			m_m6844_interrupt &= 0x7f;
			// swtpc09_irq_handler(DMAC_IRQ, CLEAR_LINE); // FIX: callback
		}
	}
}
