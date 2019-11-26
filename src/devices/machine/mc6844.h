// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/***************************************************************************

 Motorola 6844 emulation header file

***************************************************************************/

#ifndef MAME_MACHINE_MC6844_H
#define MAME_MACHINE_MC6844_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc6844_device

class mc6844_device : public device_t
{
public:
	// construction/destruction
	mc6844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_int_callback() { return m_out_int_cb.bind(); }

	// I/O operations
	DECLARE_READ8_MEMBER ( m6844_r );
	DECLARE_WRITE8_MEMBER ( m6844_w );

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_out_int_cb;

	/* channel_data structure holds info about each 6844 DMA channel */
	struct m6844_channel_data
	{
		int active;
		int address;
		int counter;
		// Channel control register.
		//  bit 0: Read / Write mode
		//  bit 1: Mode control B
		//  bit 2: Mode control A
		//  bit 3: Address up (0) / down (1).
		//  bit 4: Not used
		//  bit 5: Not used
		//  bit 6: Busy / Ready. Read only. Set when request
		//         made. Cleared when transfer completed.
		//  bit 7: DMA end flag. Read only? Set when transfer
		//         completed. Cleared when control register
		//          read. Sets IRQ.
		// Mode control A,B: 0,0 Mode2; 0,1 Mode 3; 1,0 Mode 0;
		//                   1,1 Undefined.
		uint8_t control;
		int start_address;
		int start_counter;
	};

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	uint8_t m_m6844_priority;
	// Interrupt control register.
	// Bit 0-3: channel interrupt enable, 1 enabled, 0 masked.
	// Bit 4-6: unused
	// Bit 7: Read only. Set to 1 when IRQ asserted. Clear when the
	// control register associated with the channel that caused the
	// interrut is read.
	uint8_t m_m6844_interrupt;
	uint8_t m_m6844_chain;
	void m6844_update_interrupt();
	void m6844_fdc_dma_transfer(uint8_t channel);
	void m6844_hdc_dma_transfer(uint8_t channel);
};

// device type definition
DECLARE_DEVICE_TYPE(MC6844, mc6844_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_MC6844_H
