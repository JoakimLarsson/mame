// license:BSD-3-Clause
// copyright-holders:Fredrik Öhrström
#ifndef MAME_MACHINE_MYB3K_H
#define MAME_MACHINE_MYB3K_H

#pragma once

#define MYB3K_KBD_CB_PUT(cls, fnc)          myb3k_keyboard_device::output_delegate((&cls::fnc), (#cls "::" #fnc), DEVICE_SELF, ((cls *)nullptr))
#define MYB3K_KBD_CB_DEVPUT(tag, cls, fnc)  myb3k_keyboard_device::output_delegate((&cls::fnc), (#cls "::" #fnc), (tag), ((cls *)nullptr))

#define MCFG_MYB3K_KEYBOARD_CB(cb)          myb3k_keyboard_device::set_keyboard_callback(*device, (MYB3K_KBD_CB_##cb));

DECLARE_DEVICE_TYPE(MYB3K_KEYBOARD, myb3k_keyboard_device)

INPUT_PORTS_EXTERN( myb3k_keyboard );

class myb3k_keyboard_device : public device_t
{
public:
	typedef device_delegate<void (u8)> output_delegate;

	myb3k_keyboard_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock);

	enum
	{
		TIMER_ID_SCAN_KEYS,
		TIMER_ID_FIRST_BYTE,
		TIMER_ID_SECOND_BYTE
	};

	template <class Object> static void set_keyboard_callback(device_t &device, Object &&cb) {
		downcast<myb3k_keyboard_device &>(device).m_keyboard_cb = std::forward<Object>(cb);
	}

	virtual ioport_constructor device_input_ports() const override;

protected:
	myb3k_keyboard_device(
			const machine_config &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void send_byte(u8 code);
	void key_changed(int x, int y, bool down);
	void scan_keys();
	void update_modifiers(int y, bool down);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	emu_timer       *m_scan_timer;
	output_delegate m_keyboard_cb;
	required_ioport m_io_kbd_t[12];
	u8              m_io_kbd_state[12][8];

	int m_x, m_y;
	u8 m_first_byte;
	u8 m_second_byte;
	u8 m_modifier_keys;
};

#endif // MAME_MACHINE_MYB3K_H
