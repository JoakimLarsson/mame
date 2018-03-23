// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

  Monochrome Display Adapter (MDA) section

***************************************************************************/

#include "emu.h"
#include "mda.h"

#include "machine/pc_lpt.h"
#include "video/mc6845.h"
#include "screen.h"


#define MDA_SCREEN_NAME "mda_screen"
#define MDA_MC6845_NAME "mc6845_mda"

/*
  Hercules video card
 */
#define HERCULES_SCREEN_NAME "hercules_screen"
#define HERCULES_MC6845_NAME "mc6845_hercules"

#define LOG_READ    (1U << 1)
#define LOG_SETUP   (1U << 2)
#define LOG_ROW     (1U << 3)
#define LOG_MODE    (1U << 4)
#define LOG_CHRG    (1U << 5)
#define LOG_STAT    (1U << 6)

#define VERBOSE (LOG_MODE|LOG_STAT)
#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGROW(...)   LOGMASKED(LOG_ROW,   __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGCHRG(...)  LOGMASKED(LOG_CHRG,  __VA_ARGS__)
#define LOGSTAT(...)  LOGMASKED(LOG_STAT,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MDA_CLOCK   16257000

static const unsigned char mda_palette[4][3] =
{
	{ 0x00,0x00,0x00 },
	{ 0x00,0x55,0x00 },
	{ 0x00,0xaa,0x00 },
	{ 0x00,0xff,0x00 }
};

enum
{
	MDA_TEXT_INTEN = 0,
	MDA_TEXT_BLINK,
	HERCULES_GFX_BLINK,
	MDA_LOWRES_TEXT_INTEN,
	MDA_LOWRES_TEXT_BLINK
};

/* F4 Character Displayer */
static const gfx_layout pc_16_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static const gfx_layout pc_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( pcmda )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 1, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pc_8_charlayout, 1, 1 )
GFXDECODE_END


WRITE_LINE_MEMBER(isa8_mda_device::pc_cpu_line)
{
	m_isa->irq7_w(state);
}


ROM_START( mda )
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x02000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_MDA, isa8_mda_device, "isa_ibm_mda", "IBM Monochrome Display and Printer Adapter")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(isa8_mda_device::device_add_mconfig)
	MCFG_SCREEN_ADD( MDA_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(MDA_CLOCK, 882, 0, 720, 370, 0, 350 )
	MCFG_SCREEN_UPDATE_DEVICE( MDA_MC6845_NAME, mc6845_device, screen_update )

	MCFG_PALETTE_ADD( "palette", 4 )

	MCFG_MC6845_ADD(MDA_MC6845_NAME, MC6845, MDA_SCREEN_NAME, MDA_CLOCK/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_mda_device, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_mda_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_mda_device, vsync_changed))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcmda)

	MCFG_DEVICE_ADD("lpt", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(WRITELINE(isa8_mda_device, pc_cpu_line))
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_mda_device::device_rom_region() const
{
	return ROM_NAME( mda );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_mda_device - constructor
//-------------------------------------------------

isa8_mda_device::isa8_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_MDA, tag, owner, clock)
{
}

isa8_mda_device::isa8_mda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this), m_framecnt(0), m_mode_control(0),
	m_update_row_type(-1), m_chr_gen(nullptr), m_vsync(0), m_hsync(0), m_pixel(0),
	m_palette(*this, "palette")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_mda_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();
	m_videoram.resize(0x1000);
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate( FUNC(isa8_mda_device::io_read), this ), write8_delegate( FUNC(isa8_mda_device::io_write), this ) );
	m_isa->install_bank(0xb0000, 0xb0fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb1000, 0xb1fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb2000, 0xb2fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb3000, 0xb3fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb4000, 0xb4fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb5000, 0xb5fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb6000, 0xb6fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb7000, 0xb7fff, "bank_mda", &m_videoram[0]);

	/* Initialise the mda palette */
	for (int i = 0; i < 4; i++)
		m_palette->set_pen_color(i, rgb_t(mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_mda_device::device_reset()
{
	m_framecnt = 0;
	m_mode_control = 0;
	m_vsync = 0;
	m_hsync = 0;
	m_pixel = 0;

	m_chr_gen = memregion(subtag("gfx1").c_str())->base();
}

/***************************************************************************

  Monochrome Display Adapter (MDA) section

***************************************************************************/

/***************************************************************************
  Draw text mode with 80x25 characters (default) and intense background.
  The character cell size is 9x15. Column 9 is column 8 repeated for
  character codes 176 to 223.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_mda_device::mda_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 8 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
			bg = 2;
			fg = 1;
			break;
		case 0xF0:
			bg = 3;
			fg = 0;
			break;
		case 0xF8:
			bg = 3;
			fg = 1;
			break;
		}

		if ( ( i == cursor_x && ( m_framecnt & 0x08 ) ) || ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 )
		{
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		}
		else
		{
			*p = palette[bg]; p++;
		}
	}
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and blinking characters.
  The character cell size is 9x15. Column 9 is column 8 repeated for
  character codes 176 to 223.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_mda_device::mda_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 8 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
		case 0xF0:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
		case 0xF8:
			bg = 2;
			fg = 1;
			break;
		}

		if ( ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) )
			{
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 )
		{
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		}
		else
		{
			*p = palette[bg]; p++;
		}
	}
}


MC6845_UPDATE_ROW( isa8_mda_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case MDA_TEXT_INTEN:
			mda_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case MDA_TEXT_BLINK:
			mda_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


MC6845_UPDATE_ROW( isa8_hercules_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case HERCULES_GFX_BLINK:
			hercules_gfx_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			isa8_mda_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


WRITE_LINE_MEMBER( isa8_mda_device::hsync_changed )
{
	m_hsync = state ? 1 : 0;
}


WRITE_LINE_MEMBER( isa8_mda_device::vsync_changed )
{
	m_vsync = state ? 0x80 : 0;
	if ( state )
	{
		m_framecnt++;
	}
}


/*
 *  rW  MDA mode control register (see #P138)
 */
WRITE8_MEMBER( isa8_mda_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_TEXT_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}
}


/*  R-  CRT status register (see #P139)
 *      (EGA/VGA) input status 1 register
 *      7    HGC vertical sync in progress
 *      6-4  adapter 000  hercules
 *                   001  hercules+
 *                   101  hercules InColor
 *                   else unknown
 *      3    pixel stream (0 black, 1 white)
 *      2-1  reserved
 *      0    horizontal drive enable
 */
READ8_MEMBER( isa8_mda_device::status_r)
{
	// Faking pixel stream here
	m_pixel++;

	return 0xF0 | (m_pixel & 0x08) | m_hsync;
}


/*************************************************************************
 *
 *      MDA
 *      monochrome display adapter
 *
 *************************************************************************/
WRITE8_MEMBER( isa8_mda_device::io_write)
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(MDA_MC6845_NAME);
	pc_lpt_device *lpt = subdevice<pc_lpt_device>("lpt");
	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			mc6845->address_w( space, offset, data );
			break;
		case 1: case 3: case 5: case 7:
			mc6845->register_w( space, offset, data );
			break;
		case 8:
			mode_control_w(space, offset, data);
			break;
		case 12: case 13:  case 14:
			lpt->write(space, offset - 12, data);
			break;
	}
}

READ8_MEMBER( isa8_mda_device::io_read)
{
	int data = 0xff;
	mc6845_device *mc6845 = subdevice<mc6845_device>(MDA_MC6845_NAME);
	pc_lpt_device *lpt = subdevice<pc_lpt_device>("lpt");
	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = mc6845->register_r( space, offset );
			break;
		case 10:
			data = status_r(space, offset);
			break;
		/* 12, 13, 14  are the LPT ports */
		case 12: case 13:  case 14:
			data = lpt->read(space, offset - 12);
			break;
	}
	return data;
}


/***************************************************************************

  Hercules Display Adapter section (re-uses parts from the MDA section)

***************************************************************************/

/*
When the Hercules changes to graphics mode, the number of pixels per access and
clock divider should be changed. The correct mc6845 implementation does not
allow this.

The divder/pixels per 6845 clock is 9 for text mode and 16 for graphics mode.
*/

static GFXDECODE_START( pcherc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 1, 1 )
GFXDECODE_END

ROM_START( hercules )
	ROM_REGION(0x1000,"gfx1", 0)
	ROM_LOAD("um2301.bin",  0x00000, 0x1000, CRC(0827bdac) SHA1(15f1aceeee8b31f0d860ff420643e3c7f29b5ffc))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_HERCULES, isa8_hercules_device, "isa_hercules", "Hercules Graphics Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(isa8_hercules_device::device_add_mconfig)
	MCFG_SCREEN_ADD( HERCULES_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(MDA_CLOCK, 882, 0, 720, 370, 0, 350 )
	MCFG_SCREEN_UPDATE_DEVICE( HERCULES_MC6845_NAME, mc6845_device, screen_update )

	MCFG_PALETTE_ADD( "palette", 4 )

	MCFG_MC6845_ADD(HERCULES_MC6845_NAME, MC6845, HERCULES_SCREEN_NAME, MDA_CLOCK/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_hercules_device, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_mda_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_mda_device, vsync_changed))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcherc)

	MCFG_DEVICE_ADD("lpt", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(WRITELINE(isa8_mda_device, pc_cpu_line))
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_hercules_device::device_rom_region() const
{
	return ROM_NAME( hercules );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_hercules_device - constructor
//-------------------------------------------------

isa8_hercules_device::isa8_hercules_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_HERCULES, tag, owner, clock), m_configuration_switch(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_hercules_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	m_videoram.resize(0x10000);
	set_isa_device();
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate( FUNC(isa8_hercules_device::io_read), this ), write8_delegate( FUNC(isa8_hercules_device::io_write), this ) );
	m_isa->install_bank(0xb0000, 0xbffff, "bank_hercules", &m_videoram[0]);

	/* Initialise the mda palette */
	for(int i = 0; i < (sizeof(mda_palette) / 3); i++)
		m_palette->set_pen_color(i, mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_hercules_device::device_reset()
{
	isa8_mda_device::device_reset();
	m_configuration_switch = 0;

	m_chr_gen = memregion(subtag("gfx1").c_str())->base();
}

/***************************************************************************
  Draw graphics with 720x348 pixels (default); so called Hercules gfx.
  The memory layout is divided into 4 banks where of size 0x2000.
  Every bank holds data for every n'th scanline, 8 pixels per byte,
  bit 7 being the leftmost.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_hercules_device::hercules_gfx_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  gfx_base = ( ( m_mode_control & 0x80 ) ? 0x8000 : 0x0000 ) | ( ( ra & 0x03 ) << 13 );
	int i;
	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint8_t   data = m_videoram[ gfx_base + ( ( ma + i ) << 1 ) ];

		*p = palette[( data & 0x80 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x40 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x20 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x10 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x08 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x04 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x02 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x01 ) ? 2 : 0]; p++;

		data = m_videoram[ gfx_base + ( ( ma + i ) << 1 ) + 1 ];

		*p = palette[( data & 0x80 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x40 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x20 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x10 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x08 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x04 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x02 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x01 ) ? 2 : 0]; p++;
	}
}


WRITE8_MEMBER( isa8_hercules_device::mode_control_w )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(HERCULES_MC6845_NAME);

	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_TEXT_BLINK;
		break;
	case 0x0A:          /* Hercules modes */
	case 0x2A:
		m_update_row_type = HERCULES_GFX_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}

	mc6845->set_clock( m_mode_control & 0x02 ? MDA_CLOCK / 16 : MDA_CLOCK / 9 );
	mc6845->set_hpixels_per_column( m_mode_control & 0x02 ? 16 : 9 );
}


WRITE8_MEMBER( isa8_hercules_device::io_write )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(HERCULES_MC6845_NAME);
	pc_lpt_device *lpt = subdevice<pc_lpt_device>("lpt");
	switch( offset )
	{
	case 0: case 2: case 4: case 6:
		mc6845->address_w( space, offset, data );
		break;
	case 1: case 3: case 5: case 7:
		mc6845->register_w( space, offset, data );
		break;
	case 8:
		mode_control_w(space, offset, data);
		break;
	case 12: case 13:  case 14:
		lpt->write(space, offset - 12, data);
		break;
	case 15:
		m_configuration_switch = data;
		break;
	}
}


/*  R-  CRT status register (see #P139)
 *      (EGA/VGA) input status 1 register
 *      7    HGC vertical sync in progress
 *      6-4  adapter 000  hercules
 *                   001  hercules+
 *                   101  hercules InColor
 *                   else unknown
 *      3    pixel stream (0 black, 1 white)
 *      2-1  reserved
 *      0    horizontal drive enable
 */
READ8_MEMBER( isa8_hercules_device::status_r )
{
	// Faking pixel stream here
	m_pixel++;

	return m_vsync | ( m_pixel & 0x08 ) | m_hsync;
}


READ8_MEMBER( isa8_hercules_device::io_read )
{
	int data = 0xff;
	mc6845_device *mc6845 = subdevice<mc6845_device>(HERCULES_MC6845_NAME);
	pc_lpt_device *lpt = subdevice<pc_lpt_device>("lpt");
	switch( offset )
	{
	case 0: case 2: case 4: case 6:
		/* return last written mc6845 address value here? */
		break;
	case 1: case 3: case 5: case 7:
		data = mc6845->register_r( space, offset );
		break;
	case 10:
		data = status_r(space, offset);
		break;
	/* 12, 13, 14  are the LPT ports */
	case 12: case 13:  case 14:
		data = lpt->read(space, offset - 12);
		break;
	}
	return data;
}

DEFINE_DEVICE_TYPE(ISA8_EC1840_0002, isa8_ec1840_0002_device, "ec1840_0002", "EC1840.0002 (MDA)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// XXX
MACHINE_CONFIG_START(isa8_ec1840_0002_device::device_add_mconfig)
	MCFG_SCREEN_ADD( MDA_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(MDA_CLOCK, 792, 0, 640, 370, 0, 350 )
	MCFG_SCREEN_UPDATE_DEVICE( MDA_MC6845_NAME, mc6845_device, screen_update )

	MCFG_PALETTE_ADD( "palette", 4 )

	MCFG_MC6845_ADD( MDA_MC6845_NAME, MC6845, MDA_SCREEN_NAME, MDA_CLOCK/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_mda_device, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_mda_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_mda_device, vsync_changed))
MACHINE_CONFIG_END

//-------------------------------------------------
//  isa8_ec1840_0002_device - constructor
//-------------------------------------------------

isa8_ec1840_0002_device::isa8_ec1840_0002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_EC1840_0002, tag, owner, clock), m_soft_chr_gen(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ec1840_0002_device::device_start()
{
	isa8_mda_device::device_start();

	m_soft_chr_gen = std::make_unique<uint8_t[]>(0x2000);
	m_isa->install_bank(0xdc000, 0xddfff, "bank_chargen", m_soft_chr_gen.get());
	m_isa->install_bank(0xde000, 0xdffff, "bank_chargen", m_soft_chr_gen.get());
}

void isa8_ec1840_0002_device::device_reset()
{
	isa8_mda_device::device_reset();

	m_chr_gen = m_soft_chr_gen.get();
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and intense background.
  The character cell size is 8x14.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::mda_lowres_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ (chr_base + chr * 16) << 1 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
			bg = 2;
			fg = 1;
			break;
		case 0xF0:
			bg = 3;
			fg = 0;
			break;
		case 0xF8:
			bg = 3;
			fg = 1;
			break;
		}

		if ( ( i == cursor_x && ( m_framecnt & 0x08 ) ) || ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and blinking characters.
  The character cell size is 8x14.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::mda_lowres_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ (chr_base + chr * 16) << 1 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
		case 0xF0:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
		case 0xF8:
			bg = 2;
			fg = 1;
			break;
		}

		if ( ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) )
			{
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

WRITE8_MEMBER( isa8_ec1840_0002_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_LOWRES_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_LOWRES_TEXT_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}
}

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case MDA_LOWRES_TEXT_INTEN:
			mda_lowres_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case MDA_LOWRES_TEXT_BLINK:
			mda_lowres_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}

/*****************************************************************************

  Ericsson PC MDA

******************************************************************************/

/* PCB layouts and assembly years from online pictures and physical unit.
 Ericsson   -  marked SPVT02 8301 60 53-10, assembled in 1985 indicated by chip dates
 +--------------------------------------------------------------------------------------+ ___
 |	IC1  IC2   IC3   IC4   IC5 +-IC15--EPROM-+   IC6      IC7      IC8    S1        ||
 |                                 |8363 65 14-80|                                      ||
 | IC9  IC10 IC11  IC12  IC13  IC14|CG 50821 A64 |+------------------++-IC24 EPROM--+   ||
 |                                 +-------------+| CRTC HD46505SP-1 ||10-40VP      |   ||
 | IC16 IC17 IC18  IC19  IC20  IC21     IC22      | IC23 HD68A45SP   ||402 28 A19   | J4|| not
 |                                                +------------------++-------------+   || mounted
 | IC25 IC26 IC27  IC28  IC29  IC30       IC31       IC32      IC33      IC34           ||
 |                                                                                     O-|__
 | IC35 IC36 IC37  IC38  IC39  IC40       IC41       IC42      IC43      IC44           ||  |
 |                                                                                      ||DB15
 | IC45 IC46 IC47  IC48  IC49  IC50       IC51       IC52      IC53      IC54           ||  |
 |                                                                                      ||__|
 | IC55 IC56 IC57  IC58  IC59  IC60       IC61       IC62      IC63      IC64          O-|
 |                                                                               J1A    ||
 | IC65 IC66 IC67 IC68 IC69 IC70 IC71 IC72 +--------------------------------------------+|
 +-----------------------------------------+    |||||||||  |||||||||||||||||||||||||     |
   I85565  A85571 (labels)                                                               |
                                                                                         |

 IC's (from photos)
 ------------------------------------------------------------------------------
 IC1  74F109                              IC26 74F86                                IC51 TMS4416-15NL 4 x 16Kbits DRAM
 IC2  74LS393                             IC27 74LS08                               IC52 74ALS574
 IC3  74F64                               IC28 74F153                               IC53 74LS138
 IC4  74ALS299                            IC29 74LS174                              IC54 74F86
 IC5  74LS375                             IC30 74LS374                              IC55 74F109
 IC6  74LS151                             IC31 74LS374                              IC56 74F32
 IC7  74LS153                             IC32 74ALS574                             IC57 74F109
 IC8  74LS389?                            IC33 74LS08                               IC58 74F00?
 IC9  74F02                               IC34 74LS245                              IC59 74LS244
 IC10 74ALS109                            IC35 74F10?                               IC60 TMS4416-15NL 4 x 16Kbits DRAM
 IC11 Crystal 17.040MHz                   IC36 74LS02                               IC61 TMS4416-15NL 4 x 16Kbits DRAM
 IC12 74F64                               IC37 74LS00                               IC62 74ALS574
 IC13 74ALS299                            IC38 74F374                               IC63 74LS138
 IC14 PAL? 10-70ART40101                  IC39 74LS125                              IC64 74LS245
 IC15 EPROM 8363 65 14-80 CG 50821 A64    IC40 74LS244                              IC65 74LS00
 IC16 Crystal 19.170MHz                   IC41 74LS244                              IC66 74LS02
 IC17 74LS10                              IC42 74LS574                              IC67 74LS51
 IC18 74F08                               IC43 74LS32                               IC68 74LS04
 IC19 74ALS574                            IC44 MC10124 - TTL to MECL converter      IC69 74LS153
 IC20 74LS299                             IC45 74LS109                              IC70 74LS109
 IC21 74LS273                             IC46 74LS00                               IC71 74LS138
 IC22 74ALS574                            IC47 74F194                               IC72 74LS139
 IC23 CRTC HD46505SP,HD68A45SP            IC48 74F04 
 IC24 EPROM 2764, 10-40 VP 402 28 A19     IC49 74LS174
 IC25 74ALS109                            IC50 TMS4416-15NL 4 x 16Kbits DRAM

 General description
 -------------------
 The PCB has a 2 bit DIP switch S1 and a DB15 non standard video connector. There is also an unsoldered J4 connector
 above the DB15 but no hole prepared for a connector in the plate. Above the J4 connector there is a two pin PCB connector
 that probably receives the power for the monitor for the DB15 from the PSU.

 Just below IC65 and IC66 there are two labels saying "I 85565" and "A E85571" respectively

 Video cable, card DB15 <---> monitor DB25
 ---------------------------------------------------
  Ericsson       2  +VS             4  Ericsson
  Monochrome     3  VS return       2  Monochrome HR
  HR Graphics	10  +VS            17  Monitor 3111
  Board 1070	11  VS return      15
		 4  VSYNC           6
		12  VSYNC          19
		 5  HSYNC           7
		13  HSYNC          20
		 6  High intensity  8
		14  High intensity 21
		 7  Video           9
		15  Video          22
                 8  GND            11

 */


static GFXDECODE_START( pcepc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 1, 1 )
GFXDECODE_END

ROM_START( epc )
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("8363_65_14-80_CG_50821_A64.BIN",  0x00000, 0x2000, CRC(be709786) SHA1(38ab26224bbe66bbe2bb2ccac29b41cbf78bdbf8))
//ROM_LOAD("10-40_VP_402_28_IC_24_A19.BIN",  0x00000, 0x2000, CRC(2aa53b92) SHA1(87051a037249eb631d7d2191bc0e925125c60f39))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(ISA8_EPC_MDA, isa8_epc_mda_device, "isa_epc_mda", "Ericsson PC Monochrome HR Graphics Board 1070")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
/* There are two crystals on the board: 19.170Mhz and 17.040MHz TODO: verify usage */
MACHINE_CONFIG_START(isa8_epc_mda_device::device_add_mconfig)
	MCFG_SCREEN_ADD( MDA_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(19'170'000) / 4, 600, 0, 600, 400, 0, 400 ) // clock and divider are guesswork
	MCFG_SCREEN_UPDATE_DEVICE( MDA_MC6845_NAME, h46505_device, screen_update )

	MCFG_PALETTE_ADD( "palette", 4 )

	MCFG_MC6845_ADD( MDA_MC6845_NAME, H46505, MDA_SCREEN_NAME, XTAL(19'170'000) / 16) // clock and divider are guesswork
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_epc_mda_device, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_mda_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_mda_device, vsync_changed))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcepc)
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_epc_mda_device::device_rom_region() const
{
	return ROM_NAME( epc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_epc_mda_device - constructor
//-------------------------------------------------

isa8_epc_mda_device::isa8_epc_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_EPC_MDA, tag, owner, clock), m_soft_chr_gen(nullptr)
	, m_s1(*this, "S1")
	, m_color_mode(0)
	, m_mode_control2(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_epc_mda_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	m_videoram.resize(0x8000);
	set_isa_device();
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate( FUNC(isa8_epc_mda_device::io_read), this ), write8_delegate( FUNC(isa8_epc_mda_device::io_write), this ) );
	m_isa->install_device(0x3d0, 0x3df, read8_delegate( FUNC(isa8_epc_mda_device::io_read), this ), write8_delegate( FUNC(isa8_epc_mda_device::io_write), this ) );
	m_isa->install_bank(0xb0000, 0xb7fff, "bank_epc", &m_videoram[0]); // Monochrome emulation mode VRAM address
	m_isa->install_bank(0xb8000, 0xbffff, "bank_epc", &m_videoram[0]); // Color emulation mode VRAM address

	/* Initialise the mda palette */
	for (int i = 0; i < 4; i++)
		m_palette->set_pen_color(i, rgb_t(mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]));
#if 0
	/* Initialise the mda palette */
	for(int i = 0; i < (sizeof(mda_palette) / 3); i++)
		m_palette->set_pen_color(i, mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]);
#endif
}

void isa8_epc_mda_device::device_reset()
{
	isa8_mda_device::device_reset();

	//m_chr_gen = m_soft_chr_gen.get();
	m_color_mode = m_s1->read();
	LOGSETUP("%s: m_color_mode:%02x\n", FUNCNAME, m_color_mode);
}

/*
 * Register Address table from the manual
 * Ericsson name          MDA mode   CGA mode  Standard name
 *-------------------------------------------------------------------------------
 * 6845 Address Registers 0x3b4      0x3d4     wo CRT Index reg
 * 6845 Data Registers    0x3b5      0x3d5     wo CRT Data reg
 * Mode Register 1        0x3b8      0x3d8     rw MDA/CGA mode reg
 * Mode Register 2        0x3bf      0x3df     rw CRT/CPU page reg (PCjr only)
 * Status Register        0x3ba      0x3da     r  CGA/MDA status reg 
 *                                              w EGA/VGA feature ccontrol reg
 */
WRITE8_MEMBER( isa8_epc_mda_device::io_write)
{
	LOG("%s: %04x <- %02x\n", FUNCNAME, offset, data);
	mc6845_device *mc6845 = subdevice<mc6845_device>(MDA_MC6845_NAME);
	switch( offset )
	{
		case 0x04:
			LOGSETUP(" - 6845 address write\n");
			mc6845->address_w( space, offset, data );
			break;
		case 0x05:
			LOGSETUP(" - 6845 register write\n");
			mc6845->register_w( space, offset, data );
			break;
		case 0x08: // Mode 1 reg
			LOGMODE(" - Mode register 1 write: %02x\n", data);
			LOGMODE("   MSB attribute: %s\n", (data & 0x20) == 0 ? "intensity" : "blink");
			LOGMODE("   Horizontal px: %s\n", (data & 0x10) == 0 ? "320/LR" : "640/HR");
			LOGMODE("   Video        : %s\n", (data & 0x08) == 0 ? "Disabled" : "Enabled");
			LOGMODE("   Mode         : %s\n", (data & 0x02) == 0 ? "Text" : "Graphics");
			LOGMODE("   Text columns : %d\n", (data & 0x01) == 0 ? 40 : 80);
			m_mode_control = data;
			m_update_row_type = ((data & 0x20) == 0 ? MDA_LOWRES_TEXT_INTEN : MDA_LOWRES_TEXT_BLINK);
			break;
		case 0x0f: // Mode 2 reg
			LOGMODE(" - Mode register 2 write: %02x\n", data);
			LOGMODE("   Vertical px  : %s\n", (data & 0x80) == 0 ? "200" : "400");
			LOGMODE("   Character set: %s\n", (data & 0x40) == 0 ? "0" : "1");
			LOGMODE("   Emulated     : %s\n", (data & 0x04) == 0 ? "Color" : "Monochrome");
			m_mode_control2 = data;
			break;
		default:
			LOG("EPC MDA: io_write at wrong offset:%02x\n", offset);
			logerror("EPC MDA: io_write at wrong offset:%02x\n", offset);
	}
}

READ8_MEMBER( isa8_epc_mda_device::io_read)
{
	LOG("%s: %04x <- ???\n", FUNCNAME, offset);
	int data = 0xff;
	mc6845_device *mc6845 = subdevice<mc6845_device>(MDA_MC6845_NAME);
	switch( offset )
	{
		case 0x04:
			LOGR(" - 6845 address read\n");
			break;
		case 0x05:
			LOGR(" - 6845 register read\n");
			data = mc6845->register_r( space, offset );
			break;
		case 0x08: // Mode 1 reg
			data = m_mode_control;
			LOGMODE(" - Mode register 1 read: %02x\n", data);
			break;
		case 0x0a: // Status
			data = (m_vsync != 0 ? 0x08 : 0) | (m_hsync != 0 ? 1 : 0);
			LOGSTAT(" - Status register read: %02x\n", data);
			break;
		case 0x0f: // Mode 2 reg
			data = m_mode_control2;
			LOGMODE(" - Mode register 2 read: %02x\n", data);
			break;
		default:
			LOG("EPC MDA: io_read at wrong offset:%02x\n", offset);
			logerror("EPC MDA: io_read at wrong offset:%02x\n", offset);
	}
	LOG(" !!!: %04x <- %02x\n", offset, data);
	return data;
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and intense background.
  The character cell size is 8x14.

- Mode register 1 write: 29
   MSB attribute: blink
   Horizontal px: 320/LR
   Video        : Enabled
   Mode         : Text
   Text columns : 80

 - Mode register 2 write: 00
   Vertical px  : 200
   Character set: 0
   Emulated     : Color

Mode register 1 write: 29
   MSB attribute: blink
   Horizontal px: 320/LR
   Video        : Enabled
   Mode         : Text
   Text columns : 80

 - Mode register 2 write: 04
   Vertical px  : 200
   Character set: 0
   Emulated     : Monochrome

***************************************************************************/

MC6845_UPDATE_ROW( isa8_epc_mda_device::mda_lowres_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f %s\n - y:%d x_count%d\n", machine().time().as_double(), FUNCNAME, y, x_count);

	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 16 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if (y == 0 && i == 0) LOGCHRG(" - Offset: %04x Chr: '%c'[%02x] Attr: %02x Chr_base: %04x\n", offset, chr, chr, attr, chr_base);
		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
			bg = 2;
			fg = 1;
			break;
		case 0xF0:
			bg = 3;
			fg = 0;
			break;
		case 0xF8:
			bg = 3;
			fg = 1;
			break;
		}

		if ( ( i == cursor_x && ( m_framecnt & 0x08 ) ) || ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and blinking characters.
  The character cell size is 9x16, 8x8 or 8x16 depending on mode.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_epc_mda_device::mda_lowres_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f %s\n - y:%d x_count%d\n", machine().time().as_double(), FUNCNAME, y, x_count);

	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 16];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if (y == 0 && i == 0) LOGCHRG(" - Offset: %04x Chr: '%c'[%02x] Attr: %02x Chr_base: %04x\n", offset, chr, chr, attr, chr_base);
		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
		case 0x70:
		case 0xF0:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
		case 0xF8:
			bg = 2;
			fg = 1;
			break;
		}

		if ( ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) )
			{
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

#if 0
WRITE8_MEMBER( isa8_epc_mda_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_LOWRES_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_LOWRES_TEXT_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}
}
#endif

MC6845_UPDATE_ROW( isa8_epc_mda_device::crtc_update_row )
{
	switch (m_update_row_type)
	{
		case MDA_LOWRES_TEXT_INTEN:
			mda_lowres_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case MDA_LOWRES_TEXT_BLINK:
			mda_lowres_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}

//--------------------------------------------------------------------
//  Port definition - Needs refactoring as becoming ridiculously long
//--------------------------------------------------------------------
static INPUT_PORTS_START( epc_mda_dpsw )
	PORT_START("S1")
	PORT_DIPNAME( 0x01, 0x00, "Color emulation") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING( 0x00, "Disabled" )
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "S1:2")
INPUT_PORTS_END

ioport_constructor isa8_epc_mda_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epc_mda_dpsw );
}
