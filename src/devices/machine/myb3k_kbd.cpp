// license:BSD-3-Clause
// copyright-holders:Fredrik Öhrström
/**********************************************************************

    myb3k_kbd.cpp

    Matsushita My Brain 3000 -- Panasonic JB-3000 -- Ericsson Step/One
    keyboard emulation.

    The keyboard contains an 8048 uCPU running on 5Mhz.
    Only the matrix key switches marked with X contains a diode.
    The remaining key switches o have no diodes. Three potential matrix
    crossings remain unused.
    ------------------     ----------------------------------------
    |            P24 | --->|            74159 Decoder             |
    |            P25 | --->|                                      |
    |            P26 | --->|                                      |
    |            P27 | --->|                x axis                |
    |                |     |  T0 T1 T2 T3 T4 T5 T6 T7 T8 T9 TA TB |
    |   8048 uCPU    |     ----------------------------------------
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P10 | <------X--o--o--o--o--o--o--o--o--o--o--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P11 | <------X--o--o--o--o--o--o--o--o--o--|--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P12 | <------X--o--o--o--o--o--o--o--o--o--o--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |         y  P13 | <------X--o--o--o--o--o--o--o--o--o--o--o---
    |        axis    |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P14 | <------|--o--o--o--o--o--o--o--o--o--o--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P15 | <------X--o--o--o--o--o--o--o--o--o--o--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P16 | <------o--o--o--o--o--o--o--o--o--o--o--o---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |            P17 | <------o--o--o--o--o--o--o--o--o--o--o--|---
    |                |        |  |  |  |  |  |  |  |  |  |  |  |
    |                |
    |            D0  |-----------------> *KB DATA
    |            D1  |-----------------> *KB CLK
    |                |
    -----------------

    The ROM for the 8048 is not yet extracted therefore the implementation
    of the keyboard is created based on the documentation.
    The serial protocol is not currently emulated, the data byte is sent
    directly to the host computer emulator for storing in a latch.

    This is the serial protocol sent to the host computer:

          _______LSB____ 1 ____ 2 ____ 3 ____ 4 ____ 5 ____ 6 ____MSB______
   KB CLK       |___|  |___|  |___|  |___|  |___|  |___|  |___|  |___|

          _____  __            __     __                              _____
   KB DATA    |__||____________||_____||______________________________|


   Detailed timing for bit 3:
         _____              _______________
   KB CLK    |______________|             |_______

             |              |             |
             |              |             |
                  ______
   KBD DATA_______|    |_____________________

             |    |    |    |
             |18us|18us|18us|             |
             |              |             |
             |     54us     |    54 us    |



   For a key press, two bytes are sent. For a key release two bytes are sent.
   In the host computer, an interrupt is triggered when a byte as been received
   and stored in the latch. A key down and up event therefore generates 4 interrupts
   in the host computer. KBINT is not emulated in the keyboard, it is generated by
   a circuit in the host computer. KBINT goes low when KBCLK appears, then rises after
   8 bits have been transferred. When KBINT goes high, the CPU is interrupted on IR1.


                                Key on/pressed data                     Key off/released data
                    ______ 1st byte_______ 2nd byte_______    ______ 1st byte_______ 2nd byte_______
   KBCLK & KBDATA         ||||||||||      ||||||||||                ||||||||||      ||||||||||
             ______________         _______         ______    _______         _______         ______
   KBINT                  |_________|     |_________|               |_________|     |_________|
                             0.8ms   1~5ms    0.8ms     10ms~15ms

   (The time between on and off can of course be much longer if the key is held down on purpose.)


   In the host computer:

   CPU In from port 0x04 enables a 74LS244 buffer onto the CPU data bus
   and presents the contents of the 74LS164 Serial to parallell converter
   to the CPU.

   The 74LS164 is fed with serial data directly using KBCLK and KBDATA.
   An interrupt circuit generates the IR1 interrupt when 8 bits have been
   stored in the 74LS164.

   Bits in 1st byte:                 Bits in 2nd byte:
   MSB 0                             MSB 1
   7   0                             7 x
   6   CAPS modifier                 6 x  4 bits X-coordinate
   5   RSHIFT                        5 x
   4   LSHIFT                        4 x
   3   GRAPH                         3 y
   2   CTRL                          2 y  3 bits Y-coordinate
   1   1=key on 0=key off            1 y


   Keyboard connect is 6-pin DIN socket.
   1   *KBDATA
   2   *KBCLK
   3   +5V
   4   GND
   5   GND
   6   NC

**********************************************************************/

#include "emu.h"
#include "myb3k_kbd.h"

//#define LOG_GENERAL (1U << 0) //defined in logmacro.h already
#define LOG_MOD     (1U << 1)
#define LOG_BYTES   (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_MOD | LOG_BYTES )
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGMOD(...)   LOGMASKED(LOG_MOD,   __VA_ARGS__)
#define LOGBYTES(...) LOGMASKED(LOG_BYTES, __VA_ARGS__)

// The rightmost commented number in the list below, eg // 58 for Left Shift is the numbering used in the manual
// for reference in the keyboard layout images.

INPUT_PORTS_START( myb3k_keyboard )
	PORT_START("MYB3K_CFG")
	PORT_CONFNAME( 0x0003U, 0x0000U, "Keyboard Layout" )
	PORT_CONFSETTING(       0x0000U, "Swedish Step/One" )
	PORT_CONFSETTING(       0x0001U, "Australian JB3000" )
	PORT_CONFSETTING(       0x0002U, "Japanese MyBrain3000" )

	PORT_START("MYB3K_T0")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("Control") PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) // 44
        PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)   PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) // 58
        PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("Caps lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // 71
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT)     PORT_NAME("Graph") PORT_CHAR(UCHAR_MAMEKEY(LALT)) // 72
        // Bit 0x0010U not used in keyboard.
        PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("Right Shift") PORT_CHAR(UCHAR_SHIFT_1) // 70
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)    PORT_NAME("KP 0") // 94
//        PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)     PORT_NAME("KP .") // 95

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
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)     PORT_NAME("1 !") PORT_CHAR('1')  PORT_CHAR('!') // 16

	PORT_START("MYB3K_T4")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_NAME("2 \"") PORT_CHAR('2')  PORT_CHAR('"') // 17
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_NAME("3 #") PORT_CHAR('3')  PORT_CHAR('#') // 18
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_NAME("4 $") PORT_CHAR('4')  PORT_CHAR('$') // 19
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_NAME("5 %") PORT_CHAR('5')  PORT_CHAR('%') // 20
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_NAME("6 &") PORT_CHAR('6')  PORT_CHAR('&') // 21
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_NAME("7 /") PORT_CHAR('7')  PORT_CHAR('/') // 22
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_NAME("8 (") PORT_CHAR('8')  PORT_CHAR('(') // 23
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_NAME("9 )") PORT_CHAR('9')  PORT_CHAR(')') // 24

	PORT_START("MYB3K_T5")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_NAME("0 =") PORT_CHAR('0')  PORT_CHAR('=') // 25
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_NAME("+ ?") PORT_CHAR('+') PORT_CHAR('?') // 26
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_NAME("é É") PORT_CHAR(0x00E9)  PORT_CHAR(0x00C9) // 27
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x00) // Swedish Step/One
                                                                                       PORT_NAME("< >") PORT_CHAR('<') PORT_CHAR('>') // 28
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x01) // Australian JB3000
                                                                                       PORT_NAME("\\ |") PORT_CHAR('\\') PORT_CHAR('|') // 28

        PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(8)             // 29
        PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)       PORT_NAME("Tab") PORT_CHAR(9)                   // 30
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)         PORT_NAME("q Q") PORT_CHAR('q')  PORT_CHAR('Q') // 31
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_NAME("w W") PORT_CHAR('w')  PORT_CHAR('W') // 32

	PORT_START("MYB3K_T6")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("e E") PORT_CHAR('e')  PORT_CHAR('E') // 33
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("r R") PORT_CHAR('r')  PORT_CHAR('R') // 34
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("t T") PORT_CHAR('t')  PORT_CHAR('T') // 35
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_NAME("y Y") PORT_CHAR('y')  PORT_CHAR('Y') // 36
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_NAME("u U") PORT_CHAR('u')  PORT_CHAR('U') // 37
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_NAME("i I") PORT_CHAR('i')  PORT_CHAR('I') // 38
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_NAME("o O") PORT_CHAR('o')  PORT_CHAR('O') // 39
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_NAME("p P") PORT_CHAR('p')  PORT_CHAR('P') // 40

        PORT_START("MYB3K_T7")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_NAME("å Å") PORT_CHAR(0x00E5) PORT_CHAR(0x00C5) // 41
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("ü Ü") PORT_CHAR(0x00FC) PORT_CHAR(0x00DC) // 42
        PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Enter") PORT_CHAR(13)               // 43
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_NAME("a A") PORT_CHAR('a') PORT_CHAR('A') // 45
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_NAME("s S") PORT_CHAR('s') PORT_CHAR('S') // 46
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_NAME("d D") PORT_CHAR('d') PORT_CHAR('D') // 47
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_NAME("f F") PORT_CHAR('f') PORT_CHAR('F') // 48
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_NAME("g G") PORT_CHAR('g') PORT_CHAR('G') // 49

        PORT_START("MYB3K_T8")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_NAME("h H") PORT_CHAR('h')  PORT_CHAR('H') // 50
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_NAME("j J") PORT_CHAR('j')  PORT_CHAR('J') // 51
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_NAME("k K") PORT_CHAR('k')  PORT_CHAR('K') // 52
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_NAME("l L") PORT_CHAR('l')  PORT_CHAR('L') // 53
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_NAME("ö Ö") PORT_CHAR(0x00F6)  PORT_CHAR(0x00D6) // 54
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)     PORT_NAME("ä Ä") PORT_CHAR(0x00E4)  PORT_CHAR(0x00C4) // 55
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("' *") PORT_CHAR('\'')  PORT_CHAR('*') // 56
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_NAME("z Z") PORT_CHAR('z')  PORT_CHAR('Z') // 59

        PORT_START("MYB3K_T9")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)     PORT_NAME("x X") PORT_CHAR('x')  PORT_CHAR('X') // 60
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)     PORT_NAME("c C") PORT_CHAR('c')  PORT_CHAR('C') // 61
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)     PORT_NAME("g G") PORT_CHAR('v')  PORT_CHAR('V') // 62
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)     PORT_NAME("b B") PORT_CHAR('b')  PORT_CHAR('B') // 63
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)     PORT_NAME("n N") PORT_CHAR('n')  PORT_CHAR('N') // 64
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)     PORT_NAME("m M") PORT_CHAR('m')  PORT_CHAR('M') // 65
        PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x00) // Swedish Step/One
                                                                                   PORT_NAME(", ;") PORT_CHAR(',')  PORT_CHAR(';') // 66
        PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x01) // Australian JB3000
                                                                                   PORT_NAME(", <") PORT_CHAR(',')  PORT_CHAR('<') // 66
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)  PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x00) // Swedish Step/One
                                                                                   PORT_NAME(". :") PORT_CHAR('.')  PORT_CHAR(':') // 67
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)  PORT_CONDITION("MYB3K_CFG", 0x03, EQUALS, 0x01) // Australian JB3000
                                                                                   PORT_NAME(". >") PORT_CHAR('.')  PORT_CHAR('>') // 67

        PORT_START("MYB3K_TA")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("- _") PORT_CHAR('-')  PORT_CHAR('_') // 68
        // Not electrically connected 0x0002U
        PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space") PORT_CHAR(' ') // 73
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("KP 7")              // 82
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("KP 8")              // 83
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("KP 9")              // 84
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("KP -")          // 85
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("KP 4")              // 86

        PORT_START("MYB3K_TB")
        PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("KP 5")              // 87
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("KP 6")              // 88
//	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA_PAD) PORT_NAME("KP ,")          // 89
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("KP 1")              // 90
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("KP 2")              // 91
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("KP 3")              // 92
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("KP Enter")      // 93
        // Not electrically connected 0x0080U

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
	, m_keyboard_cb()
	, m_io_kbd_t(*this, "MYB3K_T%X", 0U)
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
}

void myb3k_keyboard_device::scan_keys()
{
	// Iterate over all columns (m_x)
	// and for each column iterate over all rows (m_y)
	bool wait_for_timer = false;
	while (!wait_for_timer) {
		ioport_value bit = BIT(m_io_kbd_t[m_x]->read(), m_y);
		ioport_value bit_toggled = bit ^ m_io_kbd_state[m_x][m_y];

		if (bit_toggled) {
			m_io_kbd_state[m_x][m_y] = bit;
			key_changed(m_x, m_y, bit);

			if (IS_MODIFIER(m_x, m_y)) {
				update_modifiers(m_y, bit);
			}

			timer_set(attotime::from_msec(3), TIMER_ID_FIRST_BYTE);
			wait_for_timer = true;
		}

		m_x++;
		if (m_x > 11) {
			m_x = 0;
			m_y++;
			if (m_y > 7) {
				m_y = 0;
				// Done scanning the matrix. Now sleep for a while,
				// then start scanning again. Scan ~50 times per second.
				timer_set(attotime::from_msec(20), TIMER_ID_SCAN_KEYS);
				wait_for_timer = true;
				// (The final switch x=11 y=7 is not electrically connected.
				// Thus if we get here, then there can be no key changed
				// for this coordinate, and it is safe to start the timer here.
				// I.e. we do not need to check if wait for timer is already set.)
			}
		}
	}
}

void myb3k_keyboard_device::update_modifiers(int y, bool down)
{
	u8 bit = 0;
	switch (y) {
	case CTRL_Y:
		bit = MYB3K_KEY_CTRL;
		break;
	case LSHIFT_Y:
		bit = MYB3K_KEY_LSHIFT;
		break;
	case CAP_Y:
		bit = MYB3K_KEY_CAP;
		break;
	case GRAPH_Y:
		bit = MYB3K_KEY_GRAPH;
		break;
	case RSHIFT_Y:
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
		LOGBYTES("FIRST_BYTE %02x %s (with modifiers %s%s%s%s%s)\n", m_first_byte,
			   (m_first_byte&MYB3K_KEY_ON)?"Pressed":"Released",
			   (m_first_byte&MYB3K_KEY_CTRL)?"CTRL ":"",
			   (m_first_byte&MYB3K_KEY_GRAPH)?"GRAPH ":"",
			   (m_first_byte&MYB3K_KEY_LSHIFT)?"LSHIFT ":"",
			   (m_first_byte&MYB3K_KEY_RSHIFT)?"RSHIFT ":"",
			   (m_first_byte&MYB3K_KEY_CAP)?"CAP ":"");
		send_byte(m_first_byte);
		timer_set(attotime::from_msec(3), TIMER_ID_SECOND_BYTE);
		break;
	case TIMER_ID_SECOND_BYTE:
		LOGBYTES("SECOND_BYTE %02x x=%d y=%d \n\n\n", m_second_byte, (m_second_byte >> 3)&0xf, (m_second_byte)&0x7);
		send_byte(m_second_byte);
		timer_set(attotime::from_msec(10), TIMER_ID_SCAN_KEYS);
		break;
	}
}
