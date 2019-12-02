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
	, device_execute_interface(mconfig, *this)
	, m_out_int_cb(*this)
	, m_out_txak_cb(*this)
	, m_out_drq1_cb(*this)
	, m_out_drq2_cb(*this)
	, m_in_memr_cb(*this)
	, m_out_memw_cb(*this)
	, m_in_ior_cb{ { *this },{ *this },{ *this },{ *this } }
	, m_out_iow_cb{ { *this },{ *this },{ *this },{ *this } }
	, m_state(STATE_S0)
	, m_icount(0)
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
//  device_resolve_objects - device-specific setup
//-------------------------------------------------
void mc6844_device::device_resolve_objects()
{
	m_out_int_cb.resolve_safe();
	m_out_txak_cb.resolve_safe();
	m_out_drq1_cb.resolve_safe();
	m_out_drq2_cb.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6844_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6844_device::device_reset()
{
	// reset the 6844
	for (int i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;
	m_state = STATE_S0;
}

//-------------------------------------------------
//  dma_request -
//-------------------------------------------------

void mc6844_device::dma_request(int channel, int state)
{
	LOG("MC6844 Channel %u DMA Request: %u\n", channel, state);

	m_dreq[channel & 3] = state;

	trigger(1);
}

//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mc6844_device::execute_run()
{
	do
	{
		switch (m_state)
		{
		case STATE_SI:  // IDLE state, will suspend until a DMA request comes through
			{   //                    Hi ------> Lo
				int const priorities[][4] = {{ 1, 2, 3, 0 },
							   { 2, 3, 0, 1 },
							   { 3, 0, 1, 2 },
							   { 0, 1, 2, 3 }};

				for (int prio = 0; prio < 4; prio++)
				{
					// Rotating or static channel prioritizations
					int current_channel = priorities[((m_m6844_priority & 0x80) ? m_last_channel : 3) & 3][prio];

					if (m_m6844_channel[current_channel].active == 1 && m_dreq[current_channel] == ASSERT_LINE)
					{
						m_current_channel = m_last_channel = current_channel;
						m_state = STATE_S0;
						break;
					}
				}
			}
			if(m_state == STATE_SI)
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;
		case STATE_S0: // Wait for BCR != 0 and Tx EN == 1
			if (m_m6844_channel[m_current_channel].active == 1 &&
				m_m6844_channel[m_current_channel].counter != 0)
			{
				m_state = STATE_S1;
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;
		case STATE_S1: // Wait for Tx RQ == 1
			if (m_dreq[m_current_channel] == ASSERT_LINE)
			{
				m_state = STATE_S2;
				switch(m_m6844_channel[m_current_channel].control & 0x06)
				{
				case 0x00: // Mode 2 - single-byte transfer HALT steal mode
				case 0x02: // Mode 3 - block transfer mode
					m_out_drq2_cb(ASSERT_LINE);
					break;
				case 0x04: // Mode 1 - single-byte transfer TSC steal mode
					m_out_drq1_cb(ASSERT_LINE);
					break;
				default:
					m_out_drq1_cb(CLEAR_LINE);
					m_out_drq2_cb(CLEAR_LINE);
					break;
				}
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;
		case STATE_S2: // Wait for DGRNT == 1
			if (m_dgrnt == ASSERT_LINE && m_dreq[m_current_channel] == ASSERT_LINE)
			{
				m_out_txak_cb(m_current_channel);

				if (m_m6844_channel[m_current_channel].active == 1)  //active dma transfer
				{
					if (!(m_m6844_channel[m_current_channel].control & 0x01))  // dma write to memory from device
					{
						uint8_t data = m_in_ior_cb[m_current_channel]();
						m_out_memw_cb(m_m6844_channel[m_current_channel].address, data);
					}
					else // dma write to device from memory
					{
						uint8_t data = m_in_memr_cb(m_m6844_channel[m_current_channel].address);
						m_out_iow_cb[m_current_channel](data);
					}

					if (m_m6844_channel[m_current_channel].control & 0x08)
					{
						m_m6844_channel[m_current_channel].address--;
					}
					else
					{
						m_m6844_channel[m_current_channel].address++;
					}

					m_m6844_channel[m_current_channel].counter--;

					if (m_m6844_channel[m_current_channel].counter == 0)
					{
						m_out_drq1_cb(CLEAR_LINE);
						m_out_drq2_cb(CLEAR_LINE);
						m_m6844_channel[m_current_channel].control |= 0x80;
						m6844_update_interrupt();
						m_state = STATE_SI;
					}
					else
					{
						switch(m_m6844_channel[m_current_channel].control & 0x06)
						{
						case 0x00: // Mode 2 - single-byte transfer HALT steal mode
							m_state = STATE_S1;
							m_out_drq2_cb(CLEAR_LINE);
							break;
						case 0x02: // Mode 3 - block transfer mode
							m_state = STATE_S2; // Just for clarity, we are still in STATE_S2
							break;
						case 0x04: // Mode 1 - single-byte transfer TSC steal mode
							m_state = STATE_S1;
							m_out_drq1_cb(CLEAR_LINE);
							break;
						default: // Undefined - needs verification on real hardware
							m_state = STATE_SI;
							m_out_drq1_cb(CLEAR_LINE);
							m_out_drq2_cb(CLEAR_LINE);
							break;
						}
					}
				}
			}
			else
			{
				suspend_until_trigger(1, true);
				m_icount = 0;
			}
			break;
		default:
			logerror("MC6844: bad state, please report error\n");
			break;
		}

		m_icount--;
	} while (m_icount > 0);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


uint8_t mc6844_device::read(offs_t offset)
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


void mc6844_device::write(offs_t offset, uint8_t data)
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
			m_out_int_cb(ASSERT_LINE);
		}
	}
	else
	{
		if (m_m6844_interrupt & 0x80)
		{
			// Clear interrupt indication bit 7.
			m_m6844_interrupt &= 0x7f;
			m_out_int_cb(CLEAR_LINE);
		}
	}
}

#if 0
void m6844_device::m6844_dma_transfer(uint8_t channel)
{
	uint32_t offset;
	//address_space &space = *m_banked_space;

	offset = m_dmaf_high_address[channel] << 16;

	if (m_m6844_channel[channel].active == 1)  //active dma transfer
	{
		if (!(m_m6844_channel[channel].control & 0x01))  // dma write to memory
		{
			uint8_t data = m_fdc->data_r();

			//space.write_byte(m_m6844_channel[channel].address + offset, data);
		}
		else
		{
			uint8_t data = 0; //space.read_byte(m_m6844_channel[channel].address + offset); // FIX: callback to read from memory

			m_fdc->data_w(data);
		}

		if (m_m6844_channel[channel].control & 0x08)
			m_m6844_channel[channel].address--;
		else
			m_m6844_channel[channel].address++;

		m_m6844_channel[channel].counter--;

		if (m_m6844_channel[channel].counter == 0)    // dma transfer has finished
		{
			m_m6844_channel[channel].control |= 0x80; // set dend flag
			m6844_update_interrupt();
		}
	}
}
#endif
