// license:BSD-3-Clause
// copyright-holders:Fredrik Öhrström

#include "emu.h"
#include "myb3k_kbd.h"

INPUT_PORTS_START( myb3k_keyboard )

	PORT_START("MYB3K_T0")
// CTRL? PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CTRL) PORT_NAME("CTRL") PORT_CHAR(UCHAR_MAMEKEY(CTRL)) // 44
        PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)   PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) // 58
        PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("CAP") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // 71
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("GRAPH") PORT_CHAR(UCHAR_MAMEKEY(LALT)) // 72
        // Bit 0x0010U not used in keyboard.
        PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("Right Shift") PORT_CHAR(UCHAR_SHIFT_2) // 70
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)    PORT_NAME("KP 0") PORT_CHAR('0') // 94
        PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)     PORT_NAME("KP .") PORT_CHAR('.') // 95

	PORT_START("MYB3K_T1")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_NAME("PF1") PORT_CHAR(UCHAR_MAMEKEY(F1)) // 1
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_NAME("PF2") PORT_CHAR(UCHAR_MAMEKEY(F2)) // 2
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_NAME("PF3") PORT_CHAR(UCHAR_MAMEKEY(F3)) // 3
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_NAME("PF4") PORT_CHAR(UCHAR_MAMEKEY(F4)) // 4
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_NAME("PF5") PORT_CHAR(UCHAR_MAMEKEY(F5)) // 5
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_NAME("PF6") PORT_CHAR(UCHAR_MAMEKEY(F6)) // 6
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_NAME("PF7") PORT_CHAR(UCHAR_MAMEKEY(F7)) // 7
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_NAME("PF8") PORT_CHAR(UCHAR_MAMEKEY(F8)) // 8

	PORT_START("MYB3K_T2")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9)     PORT_NAME("Stop") PORT_CHAR(UCHAR_MAMEKEY(F9)) // 9
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10)    PORT_NAME("Copy") PORT_CHAR(UCHAR_MAMEKEY(F10)) // 10
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11)    PORT_NAME("Label") PORT_CHAR(UCHAR_MAMEKEY(F11)) // 11
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12)    PORT_NAME("EC") PORT_CHAR(UCHAR_MAMEKEY(F12)) // 12
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F13)    PORT_NAME("CLR") PORT_CHAR(UCHAR_MAMEKEY(F13)) // 13
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)   PORT_NAME("Home") PORT_CHAR(UCHAR_MAMEKEY(HOME)) // 14
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Ins") PORT_CHAR(UCHAR_MAMEKEY(INSERT)) // 74
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)     PORT_NAME("Up") PORT_CHAR(UCHAR_MAMEKEY(UP)) // 75

	PORT_START("MYB3K_T3")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL)   PORT_NAME("Del") PORT_CHAR(UCHAR_MAMEKEY(DEL)) // 76
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Page Up") PORT_CHAR(UCHAR_MAMEKEY(PGUP)) // 77
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Left") PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // 78
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)  PORT_NAME("Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // 79
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right") PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 80
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Page Down") PORT_CHAR(UCHAR_MAMEKEY(PGDN)) // 81
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)   PORT_NAME("Escape") PORT_CHAR(UCHAR_MAMEKEY(ESC)) // 15
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)     PORT_CHAR('1')  PORT_CHAR('!') // 16

	PORT_START("MYB3K_T4")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')  PORT_CHAR('"') // 17
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')  PORT_CHAR('#') // 18
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')  PORT_CHAR('$') // 19
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')  PORT_CHAR('%') // 20
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')  PORT_CHAR('&') // 21
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')  PORT_CHAR('/') // 22
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')  PORT_CHAR('(') // 23
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')  PORT_CHAR(')') // 24

	PORT_START("MYB3K_T5")
	PORT_START("MYB3K_T6")
	PORT_START("MYB3K_T7")
	PORT_START("MYB3K_T8")
	PORT_START("MYB3K_T9")
	PORT_START("MYB3K_TA")
	PORT_START("MYB3K_TB")

INPUT_PORTS_END

DEFINE_DEVICE_TYPE(MYB3K_KEYBOARD, myb3k_keyboard_device, "myb3k_keyboard", "MyBrain3000 Keyboard")

#define MYB3K_KEY_CTRL 0x02U
#define MYB3K_KEY_GRAPH 0x04U
#define MYB3K_KEY_LSHIFT 0x08U
#define MYB3K_KEY_RSHIFT 0x10U
#define MYB3K_KEY_CAP 0x20U

#define IS_MODIFIER(x,y) (x==0 && y>=0 && y<=5)

#define CTRL_Y 0
#define LSHIFT_Y 1
#define CAP_Y 2
#define GRAPH_Y 3
// x=0 y=4 is not electrically connected in keyboard
#define RSHIFT_Y 5

myb3k_keyboard_device::myb3k_keyboard_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_scan_timer(nullptr)
	, m_keyboard_cb()
	, m_io_kbd_t{ {*this, "MYB3K_T0"}, {*this, "MYB3K_T1"}, {*this, "MYB3K_T2"}, {*this, "MYB3K_T3"},
		      {*this, "MYB3K_T4"}, {*this, "MYB3K_T5"}, {*this, "MYB3K_T6"}, {*this, "MYB3K_T7"},
		      {*this, "MYB3K_T8"}, {*this, "MYB3K_T9"}, {*this, "MYB3K_TA"}, {*this, "MYB3K_TB"} }
{
}

myb3k_keyboard_device::myb3k_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: myb3k_keyboard_device(mconfig, MYB3K_KEYBOARD, tag, owner, clock)
{
}

ioport_constructor myb3k_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(myb3k_keyboard);
}

void myb3k_keyboard_device::device_start()
{
	m_keyboard_cb.bind_relative_to(*owner());
	device_reset();
}

void myb3k_keyboard_device::device_reset()
{
	m_modifier_keys = 0;
	memset(m_io_kbd_state, 0, sizeof(m_io_kbd_state));
	timer_set(attotime::from_msec(100), TIMER_ID_SCAN_KEYS);
	m_modifier_keys = 0;
}

void myb3k_keyboard_device::send_byte(u8 code)
{
	assert(!m_keyboard_cb.isnull());
	m_keyboard_cb(code);
}

// LSB=1 means key on = key pressed
// LSB=0 means key off = key released
#define MYB3K_KEY_ON 0x01

// MSB=0 means first byte in pair
// MSB=1 means second byte in pair
#define MYB3K_SECOND_KEY_BYTE 0x80

void myb3k_keyboard_device::key_changed(int x, int y, bool pressed)
{
	m_first_byte =
		(pressed ? MYB3K_KEY_ON : 0) |
		m_modifier_keys;

	m_second_byte = MYB3K_SECOND_KEY_BYTE |
		(x & 0xf) << 3 |
		(y & 0x7);

	timer_set(attotime::from_msec(3), TIMER_ID_FIRST_BYTE);
}

void myb3k_keyboard_device::scan_keys()
{
	// Iterate over all columns (m_x)
	// and for each column iterate over all rows (m_y)
	bool wait_for_timer = false;
	while (!wait_for_timer) {
		ioport_value bit = 0x01 & (m_io_kbd_t[m_x]->read() >> m_y);
		ioport_value bit_toggled = bit ^ m_io_kbd_state[m_x][m_y];

		if (bit_toggled) {
			m_io_kbd_state[m_x][m_y] = bit;
			key_changed(m_x, m_y, bit);

			if (IS_MODIFIER(m_x, m_y)) {
				update_modifiers(m_y, bit);
			}
			wait_for_timer = true;
		}

		m_x++;
		if (m_x > 11) {
			m_x = 0;
			m_y++;
			if (m_y > 7) {
				m_y = 0;
				// Done scanning the matrix. Now sleep for a while,
				// then start scanning again. Scan ~20 times per second.
				timer_set(attotime::from_msec(50), TIMER_ID_SCAN_KEYS);
				wait_for_timer = true;
			}
		}
	}
}

void myb3k_keyboard_device::update_modifiers(int y, bool down)
{
	u8 bit = 0;
	switch (y) {
	case CTRL_Y:
		printf("CTRL %s\n", down?"PRESSED":"RELEASED");
		bit = MYB3K_KEY_CTRL;
		break;
        case LSHIFT_Y:
		printf("LSHIFT %s\n", down?"PRESSED":"RELEASED");
		bit = MYB3K_KEY_LSHIFT;
		break;
        case CAP_Y:
		printf("CAP %s\n", down?"PRESSED":"RELEASED");
		bit = MYB3K_KEY_CAP;
		break;
        case GRAPH_Y:
		printf("GRAPH %s\n", down?"PRESSED":"RELEASED");
		bit = MYB3K_KEY_GRAPH;
		break;
        case  RSHIFT_Y:
		printf("RSHIFT %s\n", down?"PRESSED":"RELEASED");
		bit = MYB3K_KEY_RSHIFT;
		break;
	}

	u8 mask = ~bit;
	m_modifier_keys = (m_modifier_keys & mask) | (down?bit:0);
}

void myb3k_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_SCAN_KEYS:
		scan_keys();
		break;

	case TIMER_ID_FIRST_BYTE:
		printf("FIRST_BYTE %02x\n", m_first_byte);
		send_byte(m_first_byte);
		timer_set(attotime::from_msec(3), TIMER_ID_SECOND_BYTE);
		break;

        case TIMER_ID_SECOND_BYTE:
		printf("SECOND_BYTE %02x\n\n\n", m_second_byte);
		send_byte(m_second_byte);
		timer_set(attotime::from_msec(10), TIMER_ID_SCAN_KEYS);
		break;
	}
}
