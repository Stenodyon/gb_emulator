#pragma once
class Joypad
{
private:
	struct _joypad_register
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t right_A : 1;
				uint8_t left_B : 1;
				uint8_t up_select : 1;
				uint8_t down_start : 1;
				uint8_t select_directions : 1;
				uint8_t select_buttons : 1;
			};
#pragma pack(pop)
		};

		_joypad_register& operator=(uint8_t value)
		{
			this->value = value & 0xF0; return *this;
		}
		operator uint8_t() const
		{
			return this->value | 0x0F; // 0 means button is pressed
		}
	};
public:
	_joypad_register joypad;

	Joypad() {}
};

