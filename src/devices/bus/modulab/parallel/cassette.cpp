// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*-==============================================-
 *
 *   Modulab Cassette interface 8004-581
 *
 *-==============================================-*/

#include "emu.h"
#include "parallel.h"
#include "cassette.h"
#include "imagedev/cassette.h"

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MODULAB_CASSETTE, modulab_cassette_device, "mlcass", "Modulab cassette board")

//****************
//  LIVE DEVICE
//****************
modulab_cassette_device::modulab_cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MODULAB_CASSETTE, tag, owner, clock)
	, device_modulab_parallel_interface(mconfig, *this)
	, m_cassette(*this, "cassette")
{
}

void modulab_cassette_device::device_add_mconfig(machine_config &config)
{
	/* Cassette support - Modulab uses ?? with xx/yy Hz modulation */
	/* NOTE on usage: mame modulab -window -cass <wav file> -lab cass? -ui_active
	 * Once running enable/disable internal UI by pressing Scroll Lock in case it interferes
         * with target keys Open the internal UI by pressing TAB and then select 'Tape Control'
         * or use F2/Shift F2 for PLAY/PAUSE In order to use a wav file it has first to be created
	 * using TAB and select the 'File manager' Once created it may be given on the commandline
	 * or mounted via TAB and select Modulab v2 supports cassette through two monitor routines
	 * while v1 lacks cassette support To use the routines first store 16 bits start address at 
	 * $F9F2 and stop address at $F9F4 Press 'ADS' twice and then 'A' for PLAY (record on cassette)
	 * or 'B' for REC (play from cassette). 
	 */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_MUTED | CASSETTE_MOTOR_ENABLED);
}

#define CASS_OUT 0x80
#define CASS_IN  0x40
uint8_t modulab_cassette_device::portb_r()
{
	return  (m_cassette->input() > 0.03 ? CASS_IN : 0x00);
}

void modulab_cassette_device::device_start()
{
}

void modulab_cassette_device::portb_w(uint8_t data)
{
	// Cassette
	m_cassette->output(data & CASS_OUT ? 1.0 : -1.0);
	LOG("write cassette level: %02x\n", data );
}
