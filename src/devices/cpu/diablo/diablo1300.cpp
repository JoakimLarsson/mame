// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
  Diablo Printer TTL CPU
*/

#include "emu.h"
#include "debugger.h"
#include "diablo1300.h"
#include "diablo1300dasm.h"

/*****************************************************************************/

inline uint16_t diablo1300_cpu_device::program_read16(uint16_t address)
{
	return m_program->read_word(address);
}

inline void diablo1300_cpu_device::program_write16(uint16_t address, uint16_t data)
{
	m_program->write_word(address, data);
	return;
}

#if 0
inline uint16_t diablo1300_cpu_device::data_read16(uint16_t address)
{
	return m_program->read_word(address);
}

inline void diablo1300_cpu_device::program_write16(uint16_t address, uint16_t data)
{
	m_program->write_word(address, data);
	return;
}
#endif
/*****************************************************************************/

DEFINE_DEVICE_TYPE(DIABLO1300, diablo1300_cpu_device, "diablo1300_cpu", "DIABLO 1300 CPU")

//-------------------------------------------------
//  diablo1300_cpu_device - constructor
//-------------------------------------------------

diablo1300_cpu_device::diablo1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, DIABLO1300, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16)
	, m_pc(1)
	, m_a(0)
	, m_b(0)
	, m_carry(0)
	, m_power_on(CLEAR_LINE)
{
	// Allocate & setup
}


void diablo1300_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	//m_data    = &space(AS_DATA);

	// register our state for the debugger
	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	//state_add(STATE_GENFLAGS,  "GENFLAGS",  m_halt).callimport().callexport().formatstr("%1s").noshow();
	state_add(DIABLO_PC,         "PC",        m_pc).mask(0xffff);
	state_add(DIABLO_A,          "A",         m_a).mask(0xffff);
	state_add(DIABLO_B,          "B",         m_a).mask(0xffff);

	/* setup regtable */
	save_item(NAME(m_pc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_power_on));

	// set our instruction counter
	m_icountptr = &m_icount;
}

void diablo1300_cpu_device::device_stop()
{
}

void diablo1300_cpu_device::device_reset()
{
	m_pc    = 0;
	m_a     = 0;
	m_b     = 0;
	m_carry = 0;

	m_power_on = CLEAR_LINE;
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector diablo1300_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

#if 0
//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void diablo1300_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c", m_halt ? 'H' : '.');
			break;
	}
}
#endif

//-------------------------------------------------
//  disaassemble - call the disadiablobly
//  helper function
//-------------------------------------------------

util::disasm_interface *diablo1300_cpu_device::create_disassembler()
{
	return new diablo1300_disassembler;
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t diablo1300_cpu_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t diablo1300_cpu_device::execute_max_cycles() const
{
	return 1;
}

#if 0
//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

uint32_t diablo1300_cpu_device::execute_input_lines() const
{
	return 0;
}


//-------------------------------------------------
//  execute_set_input - set the state of an input
//  line during execution
//-------------------------------------------------

void diablo1300_cpu_device::execute_set_input(int inputnum, int state)
{
}
#endif

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------
void diablo1300_cpu_device::execute_run()
{
	uint32_t op;

	m_pc &= 0x1f;

	while (m_icount > 0)
	{
		debugger_instruction_hook(this, m_pc);

		if( m_power_on == ASSERT_LINE )
		{
			op = program_read16(m_pc);
			m_pc++;
			switch (op & 0x0007)
			{
			case 0:
				/* OUTPUT Dport, Sreg: Output register SSSS via reg A to port DDD, reg B and carry are cleared 
				   111A SSSS 0DDD RIII
				      A                = 0: register is ORed into reg A, = 1: register is copied into reg A 
				        SSSS           = Source register
				              DDD      = Destination port address
				                  R    = RAM bank select
				                   III = 000 (opcode)
				*/
				m_a = read_reg(((op & 0x0f00) >> 8) + ((op & 0x0008) ? 0x10 : 0));
				m_b = 0;
				m_carry = 0;
				write_port((op & 0x0070) >> 4, m_a);
				break;
			case 1:			
				/* JNC Addr: Set PC to address H AAAA AAAA, reg B and carry are cleared
				   AAAA AAAA 0000 HIII
				   AAAA AAAA           = 8 low bits in Destination Address
				                  H    = The 9th hi address bit
				                   III = 001 (opcode)
				*/
				m_a = (op & 0xff00) >> 8;
				m_b = 0;
				m_carry = 0;
				m_pc = (op & 0x0008) + m_a;
				break;
			case 2:
				/* RST Dport : Reset Port
				   1111 0AAA BBBB RIII
				         AAA           = Device address
				             BBBB      = I8-I5 signals
				                  R    = RAM bank select
				                   III = 010 (opcode)
				*/
				m_b = read_ibus();
				m_a = read_port((op & 0x0700) >> 8);
				m_carry = (m_carry + m_a + m_b) > 0xff ? 1 : 0;
				break;
			case 3:
				/* LDBBIT Sreg, #value: Load AAAA AAAA #value into reg A, register BBBB reg B and set carry if #value != 0
				   AAAA AAAA BBBB RIII
				   AAAA AAAA           = bits to load immediate into A
				             BBBB      = register to load into B
				                  R    = RAM bank select
				                   III = 011 (opcode)
				*/
				m_a = (op & 0xff00) >> 8;
				m_b = read_reg(((op & 0x00f0) >> 4));
				m_carry = (m_a & m_b) != 0 ? 1 : 0;
				break;
			case 4:
				switch(op & 0xc000)
				{
				case 0x4000:
					/* XLAT Dreg: Load table data into A and reg, 0 into B
					   II10 0000 AAAA RIII
					             AAAA      = Register 
						          R    = RAM bank select
					   II              III = 01xx xxxx xxxx x100 (opcode) */
					m_a = read_table(m_b + m_carry);
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;

				case 0xc000:
					/* MOVCPL Dreg, Sreg: register to register within RAM bank, acc B and carry is cleared
					   II11 SSSS DDDD RIII
				                SSSS           = Source Register 
				                     DDDD      = Destination register 
				                          R    = RAM bank select
				           II              III = 11xx xxxx xxxx x100 (opcode)
					*/
				  	m_a = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x0f00) >> 8));
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;

				case 0x8000:
					/* INPUT Dreg, Sport: port to register, acc B and carry is cleared
					   II10 SSSS DDDD RIII
					        SSSS           = Source Port
						     DDDD      = Destination register 
						          R    = RAM bank select
					   II              III = 01xx xxxx xxxx x100 (opcode)
					*/
				  	m_a = read_port((op & 0x0f00) >> 8);
					m_b = 0;
					m_carry = 0;
					write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
					break;
				default:
					break;
				}
				break;
			case 5:
				/* LOAD# Dreg,#val: Load value AAAA AAAA into register DDDD, acc B and carry is cleared
				   AAAA AAAA DDDD RIII
				   AAAA AAAA           = bits to load into A
				             DDDD      = register put A into
				                  R    = RAM bank select
				                   III = 101 (opcode)
				*/
				m_a = (op & 0xff00) >> 8;
				m_b = 0;
				m_carry = 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			case 6:
				/* ADCCPL S/Dreg, Sreg
				   1111 AAAA BBBB RIII
				        AAAA           = Load register AAAA into reg A
				             BBBB      = Load register into reg B
				                  R    = RAM bank select
				                   III = 110 (opcode)
				*/
				m_a = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x0f00) >> 8));
				m_b = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4));
				m_carry = (m_a + m_b + m_carry) > 255 ? 1 : 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			case 7:
				/* ADC# S/Dreg, #val
				   AAAA AAAA BBBB RIII
				   AAAA AAAA           = Load bits AAAA AAAA into A
				             BBBB      = Load register BBBB into B
				                     R = RAM bank select
				                   III = 100 (opcode)
				*/
				m_a = (op & 0xff00) >> 8;
				m_b = read_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4));
				m_carry = (m_a + m_b + m_carry) > 255 ? 1 : 0;
				write_reg(((op & 0x0008) != 0 ? 0x10 : 0) + ((op & 0x00f0) >> 4), m_a);
				break;
			default:
				break;
			}
		}
		--m_icount;
	}
}
