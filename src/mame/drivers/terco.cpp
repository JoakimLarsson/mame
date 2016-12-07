// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * The Terco 4426 coco multicart board and the 4490 Mill CNC Control unit
 *__________________________________________________________________________________________________________
 *                                                                                                          |
 *  +-------------------------------------------------------------------------------+
 *  |    +-----+                                          |O ||||||||||||||| O|     |
 *  |    |     |                                                                    |
 *  |    |     |                  +--+ +--+                                         |
 *  |    |     |                  |  | |  | +------+                                |
 *  |    |     |                  |  | |  | |      |                                |
 *  |    |     |                  |  | |  | +------+                                |
 *  |    +-----+      00          +--+ +--+                    +--+                 |
 *  |                                      +-------------+     |  |                 |
 *  |    +-----+   +-----+    +-----+      |             |     |  |                 |
 *  |    |     |   |     |    |     |      |             |     |  |                 |
 *  |    |     |   |     |    |     |      +-------------+     +--+                 |
 *  |    |     |   |     |    |     |                                               |
 *  |    |     |   |     |    |     |   +-------------------+                       |
 *  |    |     |   |     |    |     |   |                   |                       |
 *  |    |     |   |     |    |     |   |                   |                       |
 *  |    |     |   |     |    +-----+   +-------------------+                       |
 *  |    +-----+   +-----+                                                          |
 *  |    +-----+   +-----+    +-----+   +-----+    +-----+   +-----+                |
 *  |    | 2764|   | 2764|    |     |   |     |    |     |   |     |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |       OO       |
 *  |    |     |   |     |    |     |   |     |    |     |   |     |                |
 *  |    +-----+   +-----+    +-----+   +-----+    +-----+   +-----+                |
 *  |                                                                               |
 *  +-------------------------------------------------------------------------------+
 *__________________________________________________________________________________________________________|
 *
 * History of Terco
 *------------------
 * 
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 *
 *                     Programming     Mill CNC
 *  TODO:                Station     Control unit
 *  Terco designs:        4426          4490
 * --------------------------------------------------------------------------
 *  - Add PCB layouts     
 *  - Dump ROM:s,          OK            OK
 *  - Keyboard            
 *  - Display/CRT         
 *  - Clickable Artwork   
 *  - Sound                N/A           N/A
 *  - Cassette i/f
 *  - Expansion bus
 *  - Expansion overlay
 */

#include "emu.h"
#include "includes/coco12.h" /* Terco 4426 is based on a Coco model 2 OEMed from Tandy */
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
//#include "machine/6850acia.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE >= 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define SEP 0
#define T4490 0

/* Terco CNC Programming Station 4426 */
#if SEP

class t4426_state : public coco12_state
{
public:
	t4426_state(const machine_config &mconfig, device_type type, const char *tag)
	: coco_state(mconfig, type, tag)
	{ }
};
#endif

#if T4490
/* Terco CNC Control Station 4490 */
class t4490_state : public driver_device
{
public:
	t4490_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	,m_maincpu(*this, "maincpu")
	,m_pia1(*this, "pia1")
	,m_pia2(*this, "pia2")
	  //	,m_acia(*this, "acia")
	{ }
	required_device<m6800_cpu_device> m_maincpu;
	virtual void machine_reset() override { m_maincpu->reset(); LOG(("--->%s()\n", FUNCNAME)); };
protected:
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
//	required_device<acia6850_device> m_acia;
};
#endif

//static ADDRESS_MAP_START( t4426_map, AS_PROGRAM, 8, t4426_state )
//ADDRESS_MAP_END

#if T4490
static ADDRESS_MAP_START( t4490_map, AS_PROGRAM, 8, t4490_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_ROM AM_REGION("maincpu", 0x3000)
	AM_RANGE(0x9500, 0x95ff) AM_RAM
	AM_RANGE(0x9036, 0x9037) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x903a, 0x903b) AM_DEVREADWRITE("pia2", pia6821_device, read, write)
//	AM_RANGE(0xc820, 0xc823) AM_DEVREADWRITE("acia", acia6850_device, read, write)
	AM_RANGE(0xa000, 0xffff) AM_ROM AM_REGION("maincpu", 0xa000)
ADDRESS_MAP_END
#endif

/* Input ports */
static INPUT_PORTS_START( t4426 )
INPUT_PORTS_END

#if T4490
static INPUT_PORTS_START( t4490 )
INPUT_PORTS_END
#endif

static MACHINE_CONFIG_DERIVED_CLASS( t4426, coco, coco_state )
	MCFG_COCO_CARTRIDGE_REMOVE(CARTRIDGE_TAG)
	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "fdcv11")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))
	MCFG_COCO_VHD_ADD(VHD0_TAG)
	MCFG_COCO_VHD_ADD(VHD1_TAG)
MACHINE_CONFIG_END



#if 0
#if SEP
static MACHINE_CONFIG_START( t4426, t4426_state )
	// basic machine hardware
	MCFG_CPU_ADD(MAINCPU_TAG, M6809E, XTAL_3_579545MHz)
MACHINE_CONFIG_END
#else
static MACHINE_CONFIG_START( t4426, coco12_state )
	// basic machine hardware
	MCFG_CPU_ADD(MAINCPU_TAG, M6809E, XTAL_3_579545MHz)
	// devices
	MCFG_CASSETTE_ADD("cassette")
//	MCFG_CASSETTE_FORMATS(coco_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)
	// video hardware
	MCFG_SCREEN_MC6847_NTSC_ADD(SCREEN_TAG, VDG_TAG)

	MCFG_DEVICE_ADD(VDG_TAG, MC6847_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_HSYNC_CALLBACK(WRITELINE(coco12_state, horizontal_sync))
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(coco12_state, field_sync))
	MCFG_MC6847_INPUT_CALLBACK(DEVREAD8(SAM_TAG, sam6883_device, display_read))

#if 0
	MCFG_CPU_PROGRAM_MAP(coco_mem)
	// devices
	MCFG_DEVICE_ADD(PIA0_TAG, PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia0_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia0_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia0_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia0_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia0_irq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia0_irq_b))

	MCFG_DEVICE_ADD(PIA1_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(coco_state, pia1_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(coco_state, pia1_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(coco_state, pia1_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(coco_state, pia1_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(coco_state, pia1_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(coco_state, pia1_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(coco_state, pia1_firq_a))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(coco_state, pia1_firq_b))

	MCFG_SAM6883_ADD(SAM_TAG, XTAL_3_579545MHz, MAINCPU_TAG, AS_PROGRAM)
	MCFG_SAM6883_RES_CALLBACK(READ8(coco12_state, sam_read))

	// Becker Port device
	MCFG_DEVICE_ADD(DWSOCK_TAG, COCO_DWSOCK, 0)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, "printer")
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(PIA1_TAG, pia6821_device, ca1_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("printer", printer)

	MCFG_COCO_CARTRIDGE_ADD(CARTRIDGE_TAG, coco_cart, "pak")
	MCFG_COCO_CARTRIDGE_CART_CB(WRITELINE(coco_state, cart_w))
	MCFG_COCO_CARTRIDGE_NMI_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_NMI))
	MCFG_COCO_CARTRIDGE_HALT_CB(INPUTLINE(MAINCPU_TAG, INPUT_LINE_HALT))

	// sound hardware
	MCFG_FRAGMENT_ADD( coco_sound )

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("4K,16K,32K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list","coco_cart")
#endif
MACHINE_CONFIG_END
#endif
#endif

#if T4490
static MACHINE_CONFIG_START( t4490, t4490_state )
	MCFG_CPU_ADD("maincpu", M6800, XTAL_8MHz/4) // divided by a MC6875
	MCFG_CPU_PROGRAM_MAP(t4490_map)

	/* devices */
	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_DEVICE_ADD("pia2", PIA6821, 0)
MACHINE_CONFIG_END
#endif

ROM_START( t4426 )
	ROM_REGION(0x8000,MAINCPU_TAG,0)
//	ROM_LOAD("bas10.rom",   0x2000, 0x2000, CRC(00b50aaa) SHA1(1f08455cd48ce6a06132aea15c4778f264e19539))
//1dc57b3e4a6ef6a743ca21d8f111a74b1ea9d54e *../../../roms/terco/tercoSOFT4426-U-13-1.2.bin
//7275f1e3f165ff6a4657e4e5e24cb8b817239f54 *../../../roms/terco/tercoSOFT4426-U-14-1.2.bin
//3c1af94a	tercoSOFT4426-U-13-1.2.bin
//e031d076	tercoSOFT4426-U-14-1.2.bin

	ROM_LOAD("tercoSOFT4426-U-13-1.2.bin", 0x2000, 0x2000, CRC(3c1af94a) SHA1(1dc57b3e4a6ef6a743ca21d8f111a74b1ea9d54e))
	ROM_LOAD("tercoSOFT4426-U-14-1.2.bin", 0x4000, 0x2000, CRC(e031d076) SHA1(7275f1e3f165ff6a4657e4e5e24cb8b817239f54))
ROM_END

#if T4490
ROM_START( t4490 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "terco4490-3861104.bin", 0x3000, 0x1000, CRC(d5fd17cc) SHA1(9a3564fa69b897ec51b49ad34f2d2696cb78ee9b) ) // *roms/terco/terco4490-3861104.bin
	ROM_LOAD( "terco4490-A861104.bin", 0xa000, 0x1000, CRC(65b8e7d0) SHA1(633217fc4aa301d87790bb8744b72ef030a4c262) ) // *roms/terco/terco4490-A861104.bin
	ROM_LOAD( "terco4490-B861104.bin", 0xb000, 0x1000, CRC(5a0ce3f2) SHA1(7ec455b9075454ce5943011a1dfb5725857168f5) ) // *roms/terco/terco4490-B861104.bin
	ROM_LOAD( "terco4490-C861104.bin", 0xc000, 0x1000, CRC(0627c68c) SHA1(bf733d3ffad3f1e75684e833afc9d10d33ca870f) ) // *roms/terco/terco4490-C861104.bin
	ROM_LOAD( "terco4490-D861104.bin", 0xd000, 0x1000, CRC(2156476d) SHA1(0d70c6285541746ef15cad0d47b2d752e228abfc) ) // *roms/terco/terco4490-D861104.bin
	ROM_LOAD( "terco4490-E861104.bin", 0xe000, 0x1000, CRC(b317fa37) SHA1(a2e037a3a88b5d780067a86e52c6f7c103711a98) ) // *roms/terco/terco4490-E861104.bin
	ROM_LOAD( "terco4490-F861104.bin", 0xf000, 0x1000, CRC(a45bc3e7) SHA1(e12efa9a4c72e4bce1d59ad359ee66d7c3babfa6) ) // *roms/terco/terco4490-F861104.bin
ROM_END
#endif
/*
	ROM_LOAD( "terco4490-3861104.bin", 0xf000, 0x1000, CRC(d5fd17cc) SHA1(9a3564fa69b897ec51b49ad34f2d2696cb78ee9b) ) // *roms/terco/terco4490-3861104.bin
	ROM_LOAD( "terco4490-A861104.bin", 0xa000, 0x1000, CRC(65b8e7d0) SHA1(633217fc4aa301d87790bb8744b72ef030a4c262) ) // *roms/terco/terco4490-A861104.bin
	ROM_LOAD( "terco4490-B861104.bin", 0xb000, 0x1000, CRC(5a0ce3f2) SHA1(7ec455b9075454ce5943011a1dfb5725857168f5) ) // *roms/terco/terco4490-B861104.bin
	ROM_LOAD( "terco4490-C861104.bin", 0xc000, 0x1000, CRC(0627c68c) SHA1(bf733d3ffad3f1e75684e833afc9d10d33ca870f) ) // *roms/terco/terco4490-C861104.bin
	ROM_LOAD( "terco4490-D861104.bin", 0xd000, 0x1000, CRC(2156476d) SHA1(0d70c6285541746ef15cad0d47b2d752e228abfc) ) // *roms/terco/terco4490-D861104.bin
	ROM_LOAD( "terco4490-E861104.bin", 0xe000, 0x1000, CRC(b317fa37) SHA1(a2e037a3a88b5d780067a86e52c6f7c103711a98) ) // *roms/terco/terco4490-E861104.bin
	ROM_LOAD( "terco4490-F861104.bin", 0xf000, 0x1000, CRC(a45bc3e7) SHA1(e12efa9a4c72e4bce1d59ad359ee66d7c3babfa6) ) // *roms/terco/terco4490-F861104.bin

d5fd17cc	work/mame/roms/terco/terco4490-3861104.bin
65b8e7d0	work/mame/roms/terco/terco4490-A861104.bin
5a0ce3f2	work/mame/roms/terco/terco4490-B861104.bin
0627c68c	work/mame/roms/terco/terco4490-C861104.bin
2156476d	work/mame/roms/terco/terco4490-D861104.bin
b317fa37	work/mame/roms/terco/terco4490-E861104.bin
a45bc3e7	work/mame/roms/terco/terco4490-F861104.bin
df18397b	work/mame/roms/terco/tercoCA4426-4-8549-3.4.bin
3fcdf92e	work/mame/roms/terco/tercoCA4426-5-8549-3.4.bin
27652ccf	work/mame/roms/terco/tercoCA4426-6-8549-3.4.bin
f6640569	work/mame/roms/terco/tercoCA4426-7-8549-3.4.bin
45665428	work/mame/roms/terco/tercoED4426-0-8549-5.3.bin
44baba33	work/mame/roms/terco/tercoED4426-1-8549-5.3.bin
258e443a	work/mame/roms/terco/tercoPD4426-2-8632-6.4.bin
640d1de4	work/mame/roms/terco/tercoPD4426-3-8638-6.4.bin
bc65c45c	work/mame/roms/terco/tercoPMOS4426-8549-4.31.bin
3c1af94a	work/mame/roms/terco/tercoSOFT4426-U-13-1.2.bin
e031d076	work/mame/roms/terco/tercoSOFT4426-U-14-1.2.bin

*/

//    YEAR  NAME    PARENT      COMPAT  MACHINE     INPUT   CLASS            INIT  COMPANY             FULLNAME                              FLAGS
#if T4490
COMP( 1986, t4490,	0,          0,      t4490,      t4490,  driver_device,   0,    "Terco AB",         "Terco 4490 Mill CNC Control 4490",   MACHINE_IS_SKELETON )
#endif
COMP( 1986, t4426,	0,          0,      t4426,      t4426,  driver_device,   0,    "Terco AB",         "Terco 4426 CNC Programming station", MACHINE_IS_SKELETON )
