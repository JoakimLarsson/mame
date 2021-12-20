// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force Focus 32 driver
 *
 *  13/12/2021
 *
 * Thanks to Al Kossow and his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 * Tower version
 *        
 *   BACK                                         FRONT
 * +=========================================+    +=========================================+ 
 * |COM1 |ASCU1|ISIO1|ISCSI|CPU  |CPU  |SRAM |    |                                         |
 * |     |-2   | -1  | -1  |-21A |-21A |-22A |    |          +------------------------+     |
 * |     | RST |o RUN|o RUN| RST |     |     |    |          | 5 1/4" FDD             |     |
 * |     | TST | R/L | R/L | ABT |     |     |    |          |           _            |     |
 * |COM2 |     |     |     |     |     |     |    |          |          | |   [ ]     |     |
 * |     |o RST|o LOC|o LOC|O RUN|O RUN|O RUN|    |          |          | |    ||     |     |
 * |     |o SYS|o HLT|o HLT|O HLT|     |     |    |          |          | |    ||     |     |
 * |     |o AC |     |     |O BM |     |     |    |          |          | |    ||     |     |
 * |COM3 |     |     |     |     |     |     |    |          |          | |    ||     |     |
 * |     |o BFR|o SEL|o SEL|O FLM|O SL0|O SL0|    |          |          | |           |     |
 * |     |o BCL|o FAILo FAILO EPR|O SL1|O SL1|    |          |          | |           |     |
 * |     |o LV3|     |     |O 2WS|     |     |    |          |          | |           |     |
 * |COM4 |o LV2|     |     |O 4WS|     |     |    |          |          | |           |     |
 * |     |o LV1|o S1 |o S1 |O 6WS|     |     |    |   RST    |          | |           |     |
 * |     |o LV0|o S2 |o S2 |O 8WS|     |     |    |          |          | |           |     |
 * |     |     |o S3 |o S3 |O12WS|     |     |    |   TST    |          | |           |     |
 * |COM5 | T/L |o S4 |o S4 |O14WS|     |     |    |          |          | |           |     |
 * |     |     |     |     |     |     |     |    |  [LCK]   |          |_|           |     |
 * |     | GPIB|     |     | CSH |     |     |    |          |                        |     |
 * |     |     |     |     | R/M |     |     |    |          |                        |     |
 * |COM6 |     |     |     |     |     |     |    |          +------------------------+     |
 * |     |     |     |     |  o  |     |     |    |                                         |
 * |     |RS232|     |     |  o  |     |     |    |                                         |
 * |     |RS422|     |     |  o  |     |     |    |                                         |
 * |COM7 |     |     |     |  o  |     |     |    |    +----------+                         |
 * |     |     |     |     | RS232 RS232/422 |    |    | FORCE    |                         |
 * |     |     |     |     | P4  | P8  | P8  |    |    |          |                         |
 * |     |     |     |     |     |     |     |    |    | FOCUS 32 |                         |
 * |COM8 |SLOT1|SLOT2|SLOT3|SLOT4|SLOT5|SLOT6|    |    +----------+                         |
 * |-----------------------------------------|    |                                         |
 * |    user defined and optional boards     |    |                                         |
 * |-----------------------------------------|    |                                         |
 * |COM9 |     |     |     |AGC-1      |ISIO |    |                                         |
 * |     |     |     |     |   VME/VMX | -1  |    |                                         |
 * |     |     |     |     |   RUN/HLT |o RUN|    |                                         |
 * |     |     |     |     |     o VME | R/L |    |                                         |
 * |COM10|     |     |     |     o RUN |     |    |                                         |
 * |     |     |     |     |     o     |o LOC|    |                                         |
 * |     |     |     |     |           |o HLT|    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |COM11|     |     |     |           |     |    |                                         |
 * |     |     |     |     |           |o SEL|    |                                         |
 * |     |     |     |     |           |o FAI|    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |COM12|     |     |     |           |     |    |                                         |
 * |     |     |     |     |      R(o) |o S1 |    |                                         |
 * |     |     |     |     |           |o S2 |    |                                         |
 * |     |     |     |     |      G(o) |o S3 |    |                                         |
 * |COM13|     |     |     |           |o S4 |    |                                         |
 * |     |     |     |     |      B(o) |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |     |     |     |     |   SYNC(o) |     |    |                                         |
 * |COM14|     |     |     |           |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |     |     |     |     | Light Pen |     |    |                                         |
 * |COM15|     |     |     |  DSUB15   |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |     |     |     |     |           |     |    |                                         |
 * |COM16|SLOT7|SLOT8|SLOT9|SLT10 SLT11|SLT12|    |                                         |
 * +-----------------------------------------+    |                                         |
 * |                  --------------------   |    |                                         |
 * | AC 110/220v      \  CENTRONIX      /    |    |                                         |
 * |                   -----------------     |    |                                         |
 * +=========================================+    +=========================================+
 *
 * Desktop version was just a 90 degree CCW rotation of the front side labels and the corresponding
 * 90 degree CW rotation of the chassi allowing a monitor connected to a AGC-1 video board to sit on top.
 *
 * History of Force Computers
 *---------------------------
 * See fccpu30.cpp
 *
 * Misc links about Force Computers and this chassi:
 *-------------------------------------------------
 * http://bitsavers.org/pdf/forceComputers/
 *
 * Description, from datasheets etc
 * ------------------------------------------------
 * - Tower station for 32 bit VMEbus environments
 * - Two 6 slot motherboards for A32/D32 wide VMEbus (Pl,P2)
 * - 460W power supply to drive VMEbus and mass storage memory
 * - Metal chassis including modules for drives, power supply and connectors (170mm x 540mm x 600mm).
 * - Two independent cooling fans
 * - 16 layers motherboard
 * - 8 RS232 ports + 8 additional through optional ISIO-1 in slot 12 
 * - Up to 6 free slots for system expansion
 *

 *
 * Features per version
 * -------------------------------------------------------------------
 *  Description             PDOS System 21A   PDOS System 21B
 * -------------------------------------------------------------------
 *  CPU 68020                20 MHz           25 MHz       
 *  FPU 68881                20 MHz           20 MHz       
 *  Memory SRAM              1024 KB          1024 KB      
 *  Serial RS232             8 + 8            8 + 8
 *  GPIB                     IEEE-488          IEEE-488
 *  Winchester HDD           170MB            170MB          
 *  Floppy                   1MB              1MB           
 *  RTC with battery backup  58167            58167          
 *  RTOS                     PDOS             PDOS          
 *
 * Standard boards: CPU-21A/B, ASCU-1, ISIO-1, ISCSI-1, SRAM-22A/B
 * Optional boards: ISIO-1 (8 more RS232 ports), AGC-1 Graphics board
 * -------------------------------------------------------------------
 *
 * Address Map from CPU-21A/B board perspective
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
 * 00000000-0007FFFF        Local 512KB SRAM CPU-21 CPU board
 * 00080000-000FFFFF        VME A32 512KB SRAM CPU-22 SRAM board
 * 00080000-FAFFFFFF        VME A32 Memory if no CPU-22 installed
 * 00100000-FAFFFFFF        VME A32 Memory if CPU-22 installed
 * FCA02000-FCA1FFFF        VME A24 ISCSI-l card
 * FCB00000-FCB001FF        VME A24 ISIO-1 First card
 * FCB02000-FCB022FF        VME A24 ASCU-2 card
 * FF000000-FF07FFFF        EPROM Area 1
 * FF800000-FFFFFFFF        Local I/O devices
 * --------------------------------------------------------------------------
 */
#include "emu.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_fccpu20.h"
#include "bus/vme/vme_fcisio.h"
#include "bus/vme/vme_fcscsi.h"
#include "bus/vme/vme_mock.h"
//#include "bus/vme/vme_fcascu.h" // Mocked for now
//#include "bus/vme/vme_fcagc.h" // Mocked for now
#include "machine/clock.h"

#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)

#define VERBOSE ( LOG_SETUP)// | LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class focus32_state : public driver_device
{
public:
	focus32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vme(*this, "vmef32")
	{ }

	void focus32a(machine_config &config);
	void focus32b(machine_config &config);

private:
	required_device<vme_device> m_vme;
	// virtual void machine_start () override { LOGSETUP("%s\n", FUNCNAME); }
	// virtual void machine_reset() override { LOGSETUP("%s\n", FUNCNAME); }
	// void focus32_mem(address_map &map);
};

/* Input ports */
static INPUT_PORTS_START (focus32)
INPUT_PORTS_END

static void focus32_vme_cards(device_slot_interface &device)
{
  	device.option_add("fccpu21a", VME_FCCPU21A);
	device.option_add("fccpu21b", VME_FCCPU21B);
	device.option_add("fcisio1",  VME_FCISIO1);
	device.option_add("fciscsi1", VME_FCSCSI1);
	device.option_add("fcascu2",  VME_MOCK_FCASCU2);
	device.option_add("fcagc1",   VME_MOCK_FCAGC1);
}

/*
 * Machine configurations
 */
void focus32_state::focus32a(machine_config &config)
{
	VME(config, m_vme, 0);

	VME_SLOT(config, "slot1",  focus32_vme_cards, "fcascu2",  1, m_vme); // System controller
	VME_SLOT(config, "slot2",  focus32_vme_cards, "fcisio1",  2, m_vme); // RS232 1-8
	VME_SLOT(config, "slot3",  focus32_vme_cards, "fciscsi1", 3, m_vme); // Storage controller
	VME_SLOT(config, "slot4",  focus32_vme_cards, "fccpu21a", 4, m_vme); // 20MHz 68020 CPU + two slots with RAM
	VME_SLOT(config, "slot5",  focus32_vme_cards, nullptr,    5, m_vme); //  RAM here
	VME_SLOT(config, "slot6",  focus32_vme_cards, nullptr,    6, m_vme); // and here
	VME_SLOT(config, "slot7",  focus32_vme_cards, nullptr,    7, m_vme);
	VME_SLOT(config, "slot8",  focus32_vme_cards, nullptr,    8, m_vme);
	VME_SLOT(config, "slot9",  focus32_vme_cards, nullptr,    9, m_vme);
	VME_SLOT(config, "slot10", focus32_vme_cards, "fcagc1",  10, m_vme); // optional AGC-1 goes here
	VME_SLOT(config, "slot11", focus32_vme_cards, nullptr,   11, m_vme); //   for two slots
	VME_SLOT(config, "slot12", focus32_vme_cards, nullptr,   12, m_vme); // optional ISIO-1 goes here for RS232 9-16
}

// Focus 32 PDOS System 21B, like PDOS System 21A but a faster CPU
void focus32_state::focus32b(machine_config &config)
{
	VME(config, m_vme, 0);
	VME_SLOT(config, "slot1",  focus32_vme_cards, "fcascu2",  1, m_vme);
	VME_SLOT(config, "slot2",  focus32_vme_cards, "fcisio1",  2, m_vme);
	VME_SLOT(config, "slot3",  focus32_vme_cards, "fciscsi1", 3, m_vme); 
	VME_SLOT(config, "slot4",  focus32_vme_cards, "fccpu21b", 4, m_vme); // 25MHz 68020 CPU + two slots with RAM
	VME_SLOT(config, "slot5",  focus32_vme_cards, nullptr,    5, m_vme); //  RAM here
	VME_SLOT(config, "slot6",  focus32_vme_cards, nullptr,    6, m_vme); // and here
	VME_SLOT(config, "slot7",  focus32_vme_cards, nullptr,    7, m_vme);
	VME_SLOT(config, "slot8",  focus32_vme_cards, nullptr,    8, m_vme);
	VME_SLOT(config, "slot9",  focus32_vme_cards, nullptr,    9, m_vme);
	VME_SLOT(config, "slot10", focus32_vme_cards, "fcagc1",  10, m_vme);
	VME_SLOT(config, "slot11", focus32_vme_cards, nullptr,   11, m_vme);
	VME_SLOT(config, "slot12", focus32_vme_cards, nullptr,   12, m_vme);
}

/* ROM configurations */
ROM_START(focus32) ROM_END

/* Boards supported by same rom set, need to do like this to avoid need for multi named rom sets */
#define rom_focus32a     rom_focus32
#define rom_focus32b     rom_focus32

/* Drivers TODO: setup distinct focus32 machine configurations */
/*    YEAR  NAME      PARENT       COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY            FULLNAME     FLAGS */
COMP( 1986, focus32a, 0,            0,      focus32a, focus32, focus32_state, empty_init, "Force Computers", "Force FOCUS 32, PDOS System 21A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1986, focus32b, focus32a,     0,      focus32b, focus32, focus32_state, empty_init, "Force Computers", "Force FOCUS 32, PDOS System 21B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
