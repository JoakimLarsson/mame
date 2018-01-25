// license:BSD-3-Clause copyright-holders:Joakim Larsson Edstrom
/*
  Diablo Printer TTL CPU
*/

#ifndef MAME_CPU_DIABLO_DIABLO1300_H
#define MAME_CPU_DIABLO_DIABLO1300_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diablo1300_cpu_device

// Used by core CPU interface
class diablo1300_cpu_device : public cpu_device
{
public:
	// construction/destruction
	diablo1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	//virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	//virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual util::disasm_interface *create_disassembler() override;

	// device_state_interface overrides
	//virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;

	// memory access
	inline uint16_t program_read16(uint16_t addr);
	inline void program_write16(uint16_t addr, uint16_t data);

	inline uint16_t read_reg(uint16_t reg){ return 0;}
	inline uint16_t read_port(uint16_t port){ return 0;}
	inline uint16_t read_table(uint16_t offset){ return 0;}
	inline void write_reg(uint16_t reg, uint16_t data){}
	inline void write_port(uint16_t port, uint16_t data){}
	inline uint16_t read_ibus(){ return 0; }

	// CPU registers
	uint16_t m_pc;
	uint16_t m_a;
	uint16_t m_b;
	uint16_t m_carry;
	uint16_t m_power_on;

	// other internal states
	int m_icount;

	// address spaces
	address_space *m_program;
};

// device type definition
DECLARE_DEVICE_TYPE(DIABLO1300, diablo1300_cpu_device)

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	DIABLO_PC = 1,
	DIABLO_A,
	DIABLO_B
};

#endif // MAME_CPU_DIABLO_DIABLO1300_H
