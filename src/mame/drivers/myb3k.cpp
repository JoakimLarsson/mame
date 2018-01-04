// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Joakim Larsson Edstrom
/***************************************************************************

    Matsushita Mybrain 3000 / Panasonic JB-3000 / Ericsson Step/One

    preliminary driver by Angelo Salese

    Slotified and corrected by Joakim Larsson Edstrom based on
    Step/One service manuals: http://nivelleringslikaren.eu/stepone/

    TODO:
    - Add hi-res graphics mode (640x400 monochrome)
    - Add monochrome monitor settings
    - Hook up all interrupts and 8255 Port C signals
    - Add printer support on Port A
    - Expansion Unit with 6 more ISA8 slots
    - Proper waitstate support when 8088 CPU core admits it and remove the workaround in machine_start 

    PC INCOMPATIBILITIES:
    - COM card lives at io address 0x540
    - FDC card lives at io address 0x20
    - DMA channel 0 is not part of ISA8 but implemented on B8 (DREQ0/SRDY)
      and B19 (DACK0/MEMREF)
    - Keyboard is not interfaced through 8255
    - Non standard graphics board

    These and other incompatibilities required many PC software's to be
    recompiled to work on this computer.

****************************************************************************/
#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i8255.h"
#include "machine/myb3k_kbd.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8257.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "machine/wd_fdc.h"
#include "bus/isa/isa.h"
#include "bus/isa/myb3k_com.h"
#include "bus/isa/myb3k_fdc.h"
#include "video/mc6845.h"
#include "screen.h"

//#define LOG_GENERAL (1U << 0) //defined in logmacro.h already
#define LOG_PPI     (1U << 1)
#define LOG_PIT     (1U << 2)
#define LOG_PIC     (1U << 3)
#define LOG_CRT     (1U << 4)
#define LOG_DMA     (1U << 5)
#define LOG_KBD     (1U << 6)
#define LOG_VMOD    (1U << 7)
#define LOG_PIX    (1U << 8)

//#define VERBOSE (LOG_VMOD)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGPPI(...)  LOGMASKED(LOG_PPI,  __VA_ARGS__)
#define LOGPIT(...)  LOGMASKED(LOG_PIT,  __VA_ARGS__)
#define LOGPIC(...)  LOGMASKED(LOG_PIC,  __VA_ARGS__)
#define LOGCRT(...)  LOGMASKED(LOG_CRT,  __VA_ARGS__)
#define LOGDMA(...)  LOGMASKED(LOG_DMA,  __VA_ARGS__)
#define LOGKBD(...)  LOGMASKED(LOG_KBD,  __VA_ARGS__)
#define LOGVMOD(...) LOGMASKED(LOG_VMOD, __VA_ARGS__)
#define LOGPIX(...)  LOGMASKED(LOG_PIX, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

// PPI Port C uses
enum {
	PC0_STROBE  = 0x01, // Printer interface
	PC1_SETPAGE = 0x02, // Graphics circuit
	PC2_DISPST  = 0x04, // Graphics circuit
	PC3_LPENB   = 0x08, // Lightpen enable
	PC4_CURSR   = 0x10, // Cursor Odd/Even
	PC5_BUZON   = 0x20, // Speaker On/Off
	PC6_CMTWRD  = 0x40,
	PC7_CMTEN   = 0x80 // Cassette or Speaker
};

class myb3k_state : public driver_device
{
public:
	myb3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic")
		, m_pit8253(*this, "pit")
		, m_ppi8255(*this, "ppi")
		, m_dma8257(*this, "dma")
		, m_speaker(*this, "speaker")
		, m_kb(*this, "myb3k_keyboard")
		, m_crtc(*this, "crtc")
		, m_vram(*this, "vram")
		, m_isabus(*this, "isa")
	{ }

	DECLARE_READ8_MEMBER(myb3k_kbd_r);
	void kbd_set_data_and_interrupt(u8 data);

	MC6845_UPDATE_ROW(crtc_update_row);
	DECLARE_WRITE8_MEMBER(myb3k_video_mode_w);
	DECLARE_WRITE8_MEMBER(myb3k_fdc_output_w);
	uint32_t screen_update_myb3k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER( pic_int_w );
	DECLARE_WRITE_LINE_MEMBER( pit_out1_changed );

	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);

	DECLARE_READ8_MEMBER( io_dack0_r ){ uint8_t tmp = m_isabus->dack_r(0); LOGDMA("%s: %02x\n", FUNCNAME, tmp); return tmp; }
	DECLARE_READ8_MEMBER( io_dack1_r ){ uint8_t tmp = m_isabus->dack_r(1); LOGDMA("%s: %02x\n", FUNCNAME, tmp); return tmp; }
	DECLARE_READ8_MEMBER( io_dack2_r ){ uint8_t tmp = m_isabus->dack_r(2); LOGDMA("%s: %02x\n", FUNCNAME, tmp); return tmp; }
	DECLARE_READ8_MEMBER( io_dack3_r ){ uint8_t tmp = m_isabus->dack_r(3); LOGDMA("%s: %02x\n", FUNCNAME, tmp); return tmp; }
	DECLARE_WRITE8_MEMBER( io_dack0_w ){ LOGDMA("%s: %02x\n", FUNCNAME, data); m_isabus->dack_w(0,data); }
	DECLARE_WRITE8_MEMBER( io_dack1_w ){ LOGDMA("%s: %02x\n", FUNCNAME, data); m_isabus->dack_w(1,data); }
	DECLARE_WRITE8_MEMBER( io_dack2_w ){ LOGDMA("%s: %02x\n", FUNCNAME, data); m_isabus->dack_w(2,data); }
	DECLARE_WRITE8_MEMBER( io_dack3_w ){ LOGDMA("%s: %02x\n", FUNCNAME, data); m_isabus->dack_w(3,data); }
	DECLARE_WRITE_LINE_MEMBER( dack0_w ) { LOGDMA("%s: %d\n", FUNCNAME, state); select_dma_channel(0, state); }
	DECLARE_WRITE_LINE_MEMBER( dack1_w ) { LOGDMA("%s: %d\n", FUNCNAME, state); select_dma_channel(1, state); }
	DECLARE_WRITE_LINE_MEMBER( dack2_w ) { LOGDMA("%s: %d\n", FUNCNAME, state); select_dma_channel(2, state); }
	DECLARE_WRITE_LINE_MEMBER( dack3_w ) { LOGDMA("%s: %d\n", FUNCNAME, state); select_dma_channel(3, state); }
	DECLARE_READ8_MEMBER(dma_memory_read_byte);
	DECLARE_WRITE8_MEMBER(dma_memory_write_byte);
	DECLARE_WRITE8_MEMBER(dma_segment_w);
	DECLARE_WRITE_LINE_MEMBER( hrq_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );
	void select_dma_channel(int channel, bool state);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8253_device> m_pit8253;
	required_device<i8255_device> m_ppi8255;
	required_device<i8257_device> m_dma8257;
	required_device<speaker_sound_device>   m_speaker;
	required_device<myb3k_keyboard_device> m_kb;
	required_device<h46505_device> m_crtc;
	required_shared_ptr<uint8_t> m_vram;
	required_device<isa8_device> m_isabus;
	int m_dma_channel;
	bool m_cur_tc;
	uint8_t m_kbd_data; // Data inside the 74LS164 serial to parallel converter.
	uint8_t m_kbd_second_byte;
	uint8_t m_crtc_vreg[0x100],m_crtc_index;
	uint8_t m_vmode;
	rgb_t m_cpal[8];
	uint8_t m_portc;
	uint8_t m_dma_page[4]; // a 74670, 4 x 4 bit storage latch
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
};

void myb3k_state::video_start()
{
	LOG("%s\n", FUNCNAME);
}

READ8_MEMBER( myb3k_state::myb3k_kbd_r )
{
	LOGKBD("%s: %02x\n", FUNCNAME, m_kbd_data);
	// IN from port 0x04 enables a 74LS244 buffer that
	// presents to the CPU the parallell bits from the 74LS164
	// serial to parallel converter.
	m_pic8259->ir1_w(CLEAR_LINE);
	return m_kbd_data;
}

void myb3k_state::kbd_set_data_and_interrupt(u8 data) {
	LOGKBD("%s: %02x\n", FUNCNAME, data);
	m_kbd_data = data;
	m_pic8259->ir1_w(ASSERT_LINE);
}

MC6845_UPDATE_ROW( myb3k_state::crtc_update_row )
{
	//m_maincpu->eat_cycles(32 * 8 * 40 * 100);
	//m_maincpu->adjust_icount(-1 * 32 * 8 * 40 * 100000);
	for (int x_pos = 0; x_pos < x_count; x_pos++)
	{
		//int h_step = 64 >> (m_vmode & 3);
		uint16_t page = (m_portc & PC1_SETPAGE) ? 0x8000 : 0;

		if ((m_portc & PC2_DISPST) == 0)
		{
			for (int pxl = 0; pxl < 8; pxl++)
			{
				bitmap.pix32(y, ( x_pos * 8) + pxl) = rgb_t::black();
			}
		}
		else if (m_vmode == 1)
		{
			uint32_t pdat;
			pdat  = ((m_vram[(x_pos + ma) * 32 + ra + page +  0]  & 0xff) << 16); // Green 8 bits
			pdat |= ((m_vram[(x_pos + ma) * 32 + ra + page +  8]  & 0xf0) << 8);  // Red upper 4 bits
			pdat |= ((m_vram[(x_pos + ma) * 32 + ra + page +  8]  & 0x0f) <<  4); // Blue upper 4 bits
			pdat |= ((m_vram[(x_pos + ma) * 32 + ra + page +  24] & 0xf0) <<  4); // Red lower 4 bits
			pdat |= ((m_vram[(x_pos + ma) * 32 + ra + page +  24] & 0x0f) <<  0); // Blue lower 4 bits
			if (pdat != 0)
			  LOGPIX(" - PDAT:%06x from offset %04x RA=%d\n", pdat, (x_pos + ma) * 32 + ra + page + 0, 0);

			for (int pxl = 0; pxl < 8; pxl++)
			{
				uint16_t pind = 0;
				/* Pixeldata for 8 pixels are stored in pdat as GGRRBB  */
				pind = (((pdat & (0x800000 >> pxl)) ? 0x04 : 0x00) |
					((pdat & (0x008000 >> pxl)) ? 0x02 : 0x00) |
					((pdat & (0x000080 >> pxl)) ? 0x01 : 0x00) );

				/* Pick up the color */
				bitmap.pix32(y, ( x_pos * 8) + pxl) = m_cpal[pind & 0x07];
			}
		}
		else
		{
			int h_step = 64 >> (m_vmode & 3);
			uint16_t pdat = m_vram[(x_pos + ma) * h_step + ra + page + 8];
			for (int pxl = 0; pxl < 8; pxl++)
			{
				if ((pdat & (0x80 >> pxl)) != 0)
				{
					//bitmap.pix32(y, ( x_pos * 8) + pxl) = m_cpal[pind & 0x07];
					bitmap.pix32(y, ( x_pos * 8) + pxl) = rgb_t::green();
				}
			}
		}
		//col ^= ((cursor_x != -1 && x_pos >= cursor_x && x_pos < (cursor_x + m_cursor_size)) ? 7 : 0);
	}
}

/*
 * Setup of 6845 in different graphics modes from basica command line
 *  screen chars per row 36/40/80, rows 20/25, mode 0/1/2/3
 *
 *  chars 36  40  80  80  36  40  80  36  40
 *  rows  25  25  25  25  20  20  20  25  25
 *  mode   1   1   2   3   0   0   0   0   0
 *----------------------------------------------------------------------
 *  R0    55  55  55  55  55  55  55  55  55 - Horizontal Total
 *  R1    40  40  40  40  40  40  40  40  40 - Horizontal Displayed
 *  R2    44  44  44  44  44  44  44  44  44 - Horizontal Sync Position
 *  R3   132 132 132  52 132 132 132  52  52 - Sync Width
 *  R4    31  31  31  26  24  24  24  26  26 - Vertical Total
 *  R5     0   0   0   7   9   9   9   7   7 - Vertical Total Adjust
 *  R6    25  25  25  25  20  20  20  25  25 - Vertical Displayed
 *  R7    27  27  27  25  22  22  22  25  25 - Vertical Sync Position
 *  R8     0   0   0   3   0   0   0   3   3 - Interlace & Skew
 *  R9     7   7   7  14   9   9   9  14  14 - Maximum Raster Address
 *  R10   96  96  96  96  96  96  96  96  96 - Cursor Start Address
 *  R11    7   7   7  15   9   9   9  15  15 - Cursor End Address
 *  R12    0   0   0   0   0   0   0   0   0 - Start Address (H)
 *  R13    0   0   0   0   0   0   0   0   0 - Start Address (L)
 * -------------------------------------------------------------------
 *  vmode  1   1   2   6   1   1   2   5   5 - 3 bits latch at I/O 0x04
 *  xres 320 320 640 640 320 320 640 320 320
 *  yres 200 200 200 400 200 200 200 400 400
 *  char  40  40  80  80  40  40  80  40  40<
 */

WRITE8_MEMBER( myb3k_state::myb3k_video_mode_w )
{
	LOG("%s: %02x\n", FUNCNAME, data);
	LOGVMOD("Video Mode %02x\n", data);

	/* ---- -x-- interlace mode */
	/* ---- --xx horizontal step count (number of offsets of vram RAM data to skip, 64 >> n) */

	m_vmode = data;
	switch (data & 7)
	{
	case 0: /* Green connector 640x200 80x25 chars 1 tone or GRB 320x200 40x20 8 tones */
		LOGVMOD(" - 640x200 on Green or 320x200 on GRB output...\n");
		break;
	case 1: /* 320x200 */
		LOGVMOD(" - 320x200, 40 char, 8 color or 8 tones of green...\n");
		break;
	case 2: /* 640x200 - boots up in this mode */
		LOGVMOD(" - 640x200, 80 char, white on black...\n");
		break;
	case 3: /* Fail  */
		LOGVMOD(" - bad mode...\n");
		break;
	case 4: /* Fail  */
		LOGVMOD(" - bad mode...\n");
		break;
	case 5: LOGVMOD("320x400, 40 char, white on black\n");
		break;
	case 6: LOGVMOD("640x400, 80 char, white on black\n");
		break;
	case 7: /* Fail  */
		LOGVMOD(" - bad mode...\n");
		break;
	default: logerror("Wrong Video Mode %d, contact the maintainer\n", data);
	}
}

/**********************************************************
 *
 * PPI8255 interface
 *
 *
 * PORT A (output)
 *
 *  Printer data lines
 *
 * PORT B (input)
 *
 *  SW1
 *
 * PORT C
 * 0 - PC0 - STROBE       - Printer handshake
 * 1 - PC1 - SET PAGE     - Video RAM page
 * 2 - PC2 - DISP ST      -
 * 3 - PC3 - LPSTB        - Light Pen
 * 4 - PC4 - CURS ODD/EVN -
 * 5 - PC5 - BUZON        - ON-OFF of speaker circuit
 * 6 - PC6 - CMTWD
 * 7 - PC7 - CMTEN        - Separation from cassette interface
 *
 * Mybrain 3000, JB-3000, Step/One SW1: (Service manual numbers switches 1-8)
 * 0   - Unused
 * 1   - Unused
 * 2+3 - Display Mode
 *       OFF OFF - 36 Char/line, 10 raster
 *       ON  OFF - 40 Char/line,  8 raster
 *       OFF ON  - 80 Char/line, 16 raster
 *       ON  ON  - 80 Char/line,  8 raster
 * 4   - Expansion Unit
 *       OFF     - None
 *       ON      - Attached
 * 5+6 - Boot drive
 *       OFF OFF - A:
 *       ON  OFF - B:
 *       OFF ON  - C:
 *       ON  ON  - D:
 * 7   - Boot drive type
 *       OFF     - 5.25 inch Flexible Disk Drive
 *       ON      - 8 inch Flexible Disk Unit
 *
 * Mybrain 3000, JB-3000, Step/One SW2:  (Service manual numbers switches 1-8)
 * 0   - Check Mode (BIOS version info on boot screen)
 *       OFF     - Yes
 *       ON      - No
 * 1-3 - Reserved
 * 4   - Always off (yes so service manual says)
 * 5   - Number of disk drives
 *       OFF     - 2 drives
 *       ON      - 1 drive
 * 6-7   Initial Setting of "basic RS232 Adapter", read by software
 *       OFF OFF - ?
 *       ON  OFF - ?
 *       OFF ON  - ?
 *       ON  ON  - ?
 **********************************************************/
static ADDRESS_MAP_START(myb3k_map, AS_PROGRAM, 8, myb3k_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000,0x3ffff) AM_RAM // It's either 128Kb or 256Kb RAM
	AM_RANGE(0x40000,0x7ffff) AM_NOP
	AM_RANGE(0x80000,0xcffff) AM_NOP // Expansion Unit connected through an ISA8 cable
	AM_RANGE(0xd0000,0xdffff) AM_RAM AM_SHARE("vram")  // Area 6, physical at 30000-3FFFF (128Kb RAM) or 10000-1FFFF (256KB RAM)
	AM_RANGE(0xf0000,0xfffff) AM_ROM AM_REGION("ipl", 0) // Area 7, 8 x 8Kb
ADDRESS_MAP_END

static ADDRESS_MAP_START(myb3k_io, AS_IO, 8, myb3k_state)
	ADDRESS_MAP_UNMAP_LOW
//  AM_RANGE(0x000,0x7ff) // Main Unit
// 0-3 8255A PPI parallel port
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi", i8255_device, read, write)
// Discrete latches
	AM_RANGE(0x04, 0x04) AM_READ(myb3k_kbd_r)
	AM_RANGE(0x04, 0x04) AM_WRITE(myb3k_video_mode_w) // b0=40CH, b1=80CH, b2=16 raster
//  AM_RANGE(0x05, 0x05) AM_READ(myb3k_io_status_r)
	AM_RANGE(0x05, 0x05) AM_WRITE(dma_segment_w) // b0-b3=addr, b6=A b7=B
	AM_RANGE(0x06, 0x06) AM_READ_PORT("DSW2")
// 8-9 8259A interrupt controller
	AM_RANGE(0x08, 0x09) AM_DEVREADWRITE("pic", pic8259_device, read, write)
// c-f 8253 PIT timer
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("pit", pit8253_device, read, write)
// 10-18 8257 DMA
	AM_RANGE(0x10, 0x18) AM_DEVREADWRITE("dma", i8257_device, read, write)
// 1c-1d HD46505S CRTC
#if 1
	AM_RANGE(0x1c, 0x1c) AM_DEVREADWRITE("crtc", h46505_device, status_r, address_w)
	AM_RANGE(0x1d, 0x1d) AM_DEVREADWRITE("crtc", h46505_device, register_r, register_w)
#else
	AM_RANGE(0x1c, 0x1c) AM_WRITE(myb3k_6845_address_w)
	AM_RANGE(0x1d, 0x1d) AM_WRITE(myb3k_6845_data_w)
#endif
//	AM_RANGE(0x800,0xfff) // Expansion Unit
ADDRESS_MAP_END

/* Input ports - from Step/One service manual */
static INPUT_PORTS_START( myb3k )
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPNAME( 0x0c, 0x00, "Display Mode") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "80CH 8 raster" )
	PORT_DIPSETTING(    0x04, "80CH 16 raster" )
	PORT_DIPSETTING(    0x08, "40CH 8 raster" )
	PORT_DIPSETTING(    0x00, "36CH 10 raster" )
	PORT_DIPNAME( 0x10, 0x10, "Expansion Unit" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "Attached" )
	PORT_DIPSETTING(    0x10, "None" )
	PORT_DIPNAME( 0x60, 0x60, "Flexible Disk Drive for boot" )  PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "Drive A" )
	PORT_DIPSETTING(    0x20, "Drive B" )
	PORT_DIPSETTING(    0x40, "Drive C" )
	PORT_DIPSETTING(    0x00, "Drive D" )
	PORT_DIPNAME( 0x80, 0x80, "Flexible Disk Drive type for boot" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "8-inch Flexible Disk Unit" )     // 0x520-0x524 range
	PORT_DIPSETTING(    0x80, "5.25-inch Flexible Disk Drive" ) // 0x20-0x24 range
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Check mode" )  PORT_DIPLOCATION("SW2:1") // ROM information
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNUSED_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNUSED_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPNAME( 0x10, 0x10, "Always off" ) PORT_DIPLOCATION("SW2:5") // Some factory magic maybe?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Number of disk drive(s)" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "2 drives" )
	PORT_DIPSETTING(    0x00, "1 drive" )
	PORT_DIPNAME( 0xc0, 0xc0, "Initial setting of basic RS-232C Adapter" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "00" )
	PORT_DIPSETTING(    0x80, "01" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "11" )
INPUT_PORTS_END


void myb3k_state::machine_start()
{
	LOG("%s\n", FUNCNAME);

	m_cpal[0] = rgb_t(  0,   0,   0); // black   0.29v
	m_cpal[1] = rgb_t(  0,   0, 255); // blue    0.52v
	m_cpal[2] = rgb_t(255,   0,   0); // red     0.58v
	m_cpal[3] = rgb_t(255,   0, 255); // magenta 0.63v
	m_cpal[4] = rgb_t(  0, 255,   0); // green   0.71v
	m_cpal[5] = rgb_t(  0, 255, 255); // cyan    0.80v
	m_cpal[6] = rgb_t(255, 255,   0); // yellow  0.90v
	m_cpal[7] = rgb_t(255, 255, 255); // white   1.04v

	// CPU can only access RAM 50% of the time and the CRTC the other 50%. This waitstate workaround gives
	// close enough performance of the DOS 1.25 "basica demo" compared to the real hardware
	m_maincpu->set_clock_scale(0.5f);

	m_kbd_data = 0;
}

void myb3k_state::machine_reset()
{
	LOG("%s\n", FUNCNAME);
	m_cur_tc = false;
	m_dma_channel = -1;
	m_vmode = 0;
	m_portc = 0;
	memset(m_dma_page, 0, sizeof(m_dma_page));
}

void myb3k_state::select_dma_channel(int channel, bool state)
{
	LOGDMA("%s: %d:%d\n", FUNCNAME, channel, state);
	if(!state) {
		m_dma_channel = channel;
		if(!m_cur_tc)
			m_isabus->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_tc)
			m_isabus->eop_w(channel, CLEAR_LINE );
	}
}

WRITE_LINE_MEMBER( myb3k_state::tc_w )
{
	LOGDMA("%s: %d\n", FUNCNAME, state);
	if(m_dma_channel != -1 && (state == ASSERT_LINE) != m_cur_tc)
		m_isabus->eop_w(m_dma_channel, m_cur_tc ? ASSERT_LINE : CLEAR_LINE );
	m_cur_tc = state == ASSERT_LINE;
}

WRITE_LINE_MEMBER(myb3k_state::pic_int_w)
{
	LOGPIC("%s: %d\n", FUNCNAME, state);
	m_maincpu->set_input_line(0, state);
}

WRITE_LINE_MEMBER( myb3k_state::pit_out1_changed )
{
	LOGPIT("%s: %d\n", FUNCNAME, state);
	m_speaker->level_w(state ? 1 : 0);
}

WRITE8_MEMBER(myb3k_state::dma_segment_w)
{
	LOGDMA("%s: %02x\n", FUNCNAME, data);
	m_dma_page[(data >> 6) & 3] = (data & 0x0f);
}

WRITE_LINE_MEMBER(myb3k_state::hrq_w)
{
	LOGDMA("%s: %d\n", FUNCNAME, state);
	// Should connect to hold input clocked by DMA clock but hold isn't emulated
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	// Should be called from CPU clocked by the DMA clock but hlda output isn't emulated
	m_dma8257->hlda_w(state);
}

READ8_MEMBER(myb3k_state::dma_memory_read_byte)
{
	assert(m_dma_channel != -1);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	uint8_t tmp = prog_space.read_byte(offset | m_dma_page[m_dma_channel & 3] << 16);
	LOGDMA("%s: %x:%04x => %02x\n", FUNCNAME, m_dma_channel, offset, tmp);
	return tmp;
}

WRITE8_MEMBER(myb3k_state::dma_memory_write_byte)
{
	assert(m_dma_channel != -1);

	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	LOGDMA("%s: %x:%04x <= %02x\n", FUNCNAME, m_dma_channel, offset, data);
	return prog_space.write_byte(offset |  m_dma_page[m_dma_channel & 3] << 16, data);
}

WRITE8_MEMBER( myb3k_state::ppi_porta_w )
{
	LOGPPI("%s: %02x\n", FUNCNAME, data);

	return;
}

READ8_MEMBER( myb3k_state::ppi_portb_r )
{
	LOGPPI("%s\n", FUNCNAME);
	return ioport("DSW1")->read();
}

WRITE8_MEMBER( myb3k_state::ppi_portc_w )
{
	LOGPPI("%s: %02x\n", FUNCNAME, data);
	LOGPPI(" - STROBE : %d\n", (data & PC0_STROBE)  ? 1 : 0);
	LOGPPI(" - SETPAGE: %d\n", (data & PC1_SETPAGE) ? 1 : 0);
	LOGPPI(" - DISPST : %d\n", (data & PC2_DISPST)  ? 1 : 0);
	LOGPPI(" - LPENB  : %d\n", (data & PC3_LPENB)   ? 1 : 0);
	LOGPPI(" - CURSR  : %d\n", (data & PC4_CURSR)   ? 1 : 0);
	LOGPPI(" - BUZON  : %d\n", (data & PC5_BUZON)   ? 1 : 0);
	LOGPPI(" - CMTWRD : %d\n", (data & PC6_CMTWRD)  ? 1 : 0);
	LOGPPI(" - CMTEN  : %d\n", (data & PC7_CMTEN)   ? 1 : 0);
	LOGPPI(" => CMTEN: %d BUZON: %d\n", (data & PC7_CMTEN) ? 1 : 0, (data & PC5_BUZON)? 1 : 0);

	/*
	 * The actual logic around enabling the buzzer is a bit more complicated involving the cassette interface
	 * According to the schematics gate1 is enabled if either
	 *  (CMTEN is inactive high and BUZON active high) OR
	 *  (CMTEN is active   low  and CMTRD is inactive high)
	 * and CMTRD is low). Problem is that the schematics fails to show where CMTRD comes from so only the first case is emulated
	 */
	m_pit8253->write_gate1(!(data & PC5_BUZON) && (data & PC7_CMTEN)? 1 : 0);

	m_portc = data;
	
	return;
}

static const gfx_layout myb3k_charlayout =
{
	8, 8,
	0x400,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( myb3k )
	GFXDECODE_ENTRY( "ipl", 0x0000, myb3k_charlayout, 0, 1 )
GFXDECODE_END

static SLOT_INTERFACE_START(stepone_isa_cards)
	SLOT_INTERFACE("myb3k_com", ISA8_MYB3K_COM)
	SLOT_INTERFACE("myb3k_fdc4710", ISA8_MYB3K_FDC4710)
	SLOT_INTERFACE("myb3k_fdc4711", ISA8_MYB3K_FDC4711)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( myb3k )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz / 3) /* 14.3182 main crystal divided by three through a 8284A */
	MCFG_CPU_PROGRAM_MAP(myb3k_map)
	MCFG_CPU_IO_MAP(myb3k_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic", pic8259_device, inta_cb)

	/* Expansion  bus */
	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic", pic8259_device, ir5_w)) // Jumper J4 selectable
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic", pic8259_device, ir7_w)) // Jumper J5 selectable
//  MCFG_ISA_OUT_DRQ0_CB(DEVWRITELINE("dma", i8257_device, dreq0_w)) // Part of ISA16 but not ISA8 standard but implemented on ISA8 B8 (SRDY) on this motherboard
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma", i8257_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma", i8257_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma", i8257_device, dreq3_w))
	MCFG_ISA8_SLOT_ADD("isa", "isa1", stepone_isa_cards, "myb3k_fdc4711", false)
	MCFG_ISA8_SLOT_ADD("isa", "isa2", stepone_isa_cards, "myb3k_com", false)
	MCFG_ISA8_SLOT_ADD("isa", "isa3", stepone_isa_cards, nullptr, false)

	/* Interrupt Controller TODO: Add trampolin to read jumper J4 and J5 for IRQ sources */
	/* Jumper J4 selects LPSTB or ISA interrupt 5 for IR5 */
	/* Jumper J5 selects PRINT or ISA interrupt 7 for IR7 */
	MCFG_DEVICE_ADD("pic", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(WRITELINE(myb3k_state, pic_int_w))

	MCFG_DEVICE_ADD("ppi", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(myb3k_state, ppi_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(myb3k_state, ppi_portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(myb3k_state, ppi_portc_w))

	/* DMA chip */
	MCFG_DEVICE_ADD("dma", I8257, XTAL_14_31818MHz / 6)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(myb3k_state, hrq_w))
	MCFG_I8257_OUT_TC_CB(WRITELINE(myb3k_state, tc_w))
	MCFG_I8257_IN_MEMR_CB(READ8(myb3k_state, dma_memory_read_byte))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(myb3k_state, dma_memory_write_byte))
	MCFG_I8257_IN_IOR_0_CB(READ8(myb3k_state, io_dack0_r))
	MCFG_I8257_IN_IOR_1_CB(READ8(myb3k_state, io_dack1_r))
	MCFG_I8257_IN_IOR_2_CB(READ8(myb3k_state, io_dack2_r))
	MCFG_I8257_IN_IOR_3_CB(READ8(myb3k_state, io_dack3_r))
	MCFG_I8257_OUT_IOW_0_CB(WRITE8(myb3k_state, io_dack0_w))
	MCFG_I8257_OUT_IOW_1_CB(WRITE8(myb3k_state, io_dack1_w))
	MCFG_I8257_OUT_IOW_2_CB(WRITE8(myb3k_state, io_dack2_w))
	MCFG_I8257_OUT_IOW_3_CB(WRITE8(myb3k_state, io_dack3_w))
	MCFG_I8257_OUT_DACK_0_CB(WRITELINE(myb3k_state, dack0_w))
	MCFG_I8257_OUT_DACK_1_CB(WRITELINE(myb3k_state, dack1_w))
	MCFG_I8257_OUT_DACK_2_CB(WRITELINE(myb3k_state, dack2_w))
	MCFG_I8257_OUT_DACK_3_CB(WRITELINE(myb3k_state, dack3_w))

	/* Timer chip */
	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz / 12.0) /* TIMINT straight into IRQ0 */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz / 12.0) /* speaker if port c bit 5 is low */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(myb3k_state, pit_out1_changed))
//  MCFG_PIT8253_CLK2(XTAL_14_31818MHz / 12.0) /* ANDed with port c bit 6 but marked as "not use"*/
//  MCFG_PIT8253_OUT2_HANDLER(WRITELINE(myb3k_state, pit_out2_changed))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("myb3k_keyboard", MYB3K_KEYBOARD, 0)
	MCFG_MYB3K_KEYBOARD_CB(PUT(myb3k_state, kbd_set_data_and_interrupt))

	/* video hardware */
	//MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)

	/* devices */
	MCFG_MC6845_ADD("crtc", H46505, "screen", XTAL_14_31818MHz / 16) /* Main crystal divided by 16 through a 74163 4 bit counter */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(myb3k_state, crtc_update_row)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jb3000, myb3k )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( stepone, myb3k )
MACHINE_CONFIG_END

/* ROM definitions, ROM area is 8 x 8Kb and can be populated with 2732 mask ROMs or 2764s */
ROM_START( myb3k )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "myb3kchrg-v2.07.bin", 0xc000, 0x2000, CRC(1e43e1e3) SHA1(956b580c9cbcaf2c5ff74e3ef80a5ac98c2df434))
	ROM_LOAD( "myb3kbios-v2.07.bin", 0xe000, 0x2000, CRC(c4c46cc5) SHA1(a3e186513fbe9ad0e369b481999393a3506db39e))
ROM_END

ROM_START( jb3000 )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "jb3000chrg-v2.07.bin", 0xc000, 0x2000, NO_DUMP)
	ROM_LOAD( "jb3000bios-v2.07.bin", 0xe000, 0x2000, NO_DUMP)
ROM_END

ROM_START( stepone )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "steponechrg-v2.07.bin", 0xc000, 0x2000, CRC(8284a391) SHA1(7203c5e9d83be37c1c195946fbee5c53b1bce391))
	ROM_LOAD( "steponebios-v2.07.bin", 0xe000, 0x2000, CRC(322c1618) SHA1(a7a3cc2af7cc9556007d98014714ba656f6e79d1))
ROM_END

//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT  STATE           INIT   COMPANY        FULLNAME        FLAGS
COMP( 1982, myb3k,   0,      0,       myb3k,     myb3k, myb3k_state,    0,     "Matsushita",  "MyBrain 3000", 0)
COMP( 1982, jb3000,  myb3k,  0,       jb3000,    myb3k, myb3k_state,    0,     "Panasonic",   "JB-3000",      MACHINE_NOT_WORKING) // No rom dump available
COMP( 1984, stepone, myb3k,  0,       stepone,   myb3k, myb3k_state,    0,     "Ericsson",    "Step/One",     0)
