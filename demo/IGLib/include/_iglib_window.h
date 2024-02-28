#pragma once
#include "_iglib_base.h"
#include "_iglib_rect.h"
#include "_iglib_canvas.h"
#include <memory>

namespace ig
{



	enum class WindowCallbackReason
	{
		Resized,
		Moved,
		Focused,
		Unfocused,
		Minimized,
		Maxmized,
		Restored,

		DirtyScreen,

		ResizedFramebuffer,
		RescaledContents,

		// when the user presses close/alt+f4 on the window. if the event is triggered, should_close() will always be true
		RequestedClose,
		Closing
	};

	enum class WindowVisibiltyState : uint8_t
	{
		Hidden,
		Minimized,
		Restored,
		Maximized
	};

	enum class KeyCode : int16_t
	{
		Unkown = -1,
		Key_Space = 32,
		Key_Apostrophe = 39,
		Key_Comma = 44,
		Key_Minus = 45,
		Key_Period = 46,
		Key_Slash = 47,
		Key_0 = 48,
		Key_1 = 49,
		Key_2 = 50,
		Key_3 = 51,
		Key_4 = 52,
		Key_5 = 53,
		Key_6 = 54,
		Key_7 = 55,
		Key_8 = 56,
		Key_9 = 57,
		Key_Semicolon = 59,
		Key_Equal = 61,
		Key_A = 65,
		Key_B = 66,
		Key_C = 67,
		Key_D = 68,
		Key_E = 69,
		Key_F = 70,
		Key_G = 71,
		Key_H = 72,
		Key_I = 73,
		Key_J = 74,
		Key_K = 75,
		Key_L = 76,
		Key_M = 77,
		Key_N = 78,
		Key_O = 79,
		Key_P = 80,
		Key_Q = 81,
		Key_R = 82,
		Key_S = 83,
		Key_T = 84,
		Key_U = 85,
		Key_V = 86,
		Key_W = 87,
		Key_X = 88,
		Key_Y = 89,
		Key_Z = 90,
		Key_LeftBracket = 91,
		Key_Backslash = 92,
		Key_RightBracket = 93,
		Key_GraveAccent = 96,
		Key_World1 = 161,
		Key_World2 = 162,
		Key_Escape = 256,
		Key_Enter = 257,
		Key_Tab = 258,
		Key_Backspace = 259,
		Key_Insert = 260,
		Key_Delete = 261,
		Key_Right = 262,
		Key_Left = 263,
		Key_Down = 264,
		Key_Up = 265,
		Key_PageUp = 266,
		Key_PageDown = 267,
		Key_Home = 268,
		Key_End = 269,
		Key_CapsLock = 280,
		Key_ScrollLock = 281,
		Key_NumLock = 282,
		Key_PrintScreen = 283,
		Key_Pause = 284,
		Key_F1 = 290,
		Key_F2 = 291,
		Key_F3 = 292,
		Key_F4 = 293,
		Key_F5 = 294,
		Key_F6 = 295,
		Key_F7 = 296,
		Key_F8 = 297,
		Key_F9 = 298,
		Key_F10 = 299,
		Key_F11 = 300,
		Key_F12 = 301,
		Key_F13 = 302,
		Key_F14 = 303,
		Key_F15 = 304,
		Key_F16 = 305,
		Key_F17 = 306,
		Key_F18 = 307,
		Key_F19 = 308,
		Key_F20 = 309,
		Key_F21 = 310,
		Key_F22 = 311,
		Key_F23 = 312,
		Key_F24 = 313,
		Key_F25 = 314,
		Key_Kp0 = 320,
		Key_Kp1 = 321,
		Key_Kp2 = 322,
		Key_Kp3 = 323,
		Key_Kp4 = 324,
		Key_Kp5 = 325,
		Key_Kp6 = 326,
		Key_Kp7 = 327,
		Key_Kp8 = 328,
		Key_Kp9 = 329,
		Key_KpDecimal = 330,
		Key_KpDivide = 331,
		Key_KpMultiply = 332,
		Key_KpSubtract = 333,
		Key_KpAdd = 334,
		Key_KpEnter = 335,
		Key_KpEqual = 336,
		Key_LeftShift = 340,
		Key_LeftControl = 341,
		Key_LeftAlt = 342,
		Key_LeftSuper = 343,
		Key_RightShift = 344,
		Key_RightControl = 345,
		Key_RightAlt = 346,
		Key_RightSuper = 347,
		Key_Menu = 348
	};

	enum class MouseButton : int16_t
	{
		Left = 0,
		Right = 1,
		Middle = 2,

		Button4,
		Button5,
		Button6,
		Button7,
		Button8,
	};

	enum class InputAction : uint8_t
	{
		Released,
		Pressed,
		Repeated
	};

	enum InputModFlags : uint8_t
	{
		InputMod_None = 0,
		InputMod_Shift = 0x01,
		InputMod_Control = 0x02,
		InputMod_Alt = 0x04,
		InputMod_Super = 0x08,
		InputMod_Windows = 0x08,
		InputMod_Capslock = 0x10,
		InputMod_Numlock = 0x20,
	};

	class Window;

	enum class InputEventType
	{
		Key,
		MouseButton,
		MouseScrollWheel,
	};

	union InputEvent
	{
		struct KeyEvent
		{
			KeyCode keycode;
			InputAction action;
			InputModFlags mods;
		} key;

		struct MouseButtonEvent
		{
			MouseButton button;
			InputAction action;
			InputModFlags mods;
		} mouse_button;

		struct MouseScrollEvent
		{
			int16_t x, y;
		} mouse_scroll;
	};

	typedef void(*UpdateCallback_t)();
	typedef void(*WindowCallback_t)(Window &window, WindowCallbackReason reason);
	typedef void(*InputCallback_t)(Window &window, InputEvent event, InputEventType type);
	// TODO: add a callback for when the user drops files over the window

	class Window final
	{
		friend class Renderer;
	public:
		Window() noexcept;
		Window( Vector2i size ) noexcept;
		Window( Vector2i size, std::string title ) noexcept;
		~Window() noexcept;

		Window( Window &&move ) noexcept;
		Window &operator=( Window &&other ) noexcept;

		bool is_valid() const;

		bool should_close() const;

		Vector2i get_mouse_position() const;


		int width() const;
		int height() const;
		Vector2i size() const;

		Vector2i get_position() const;
		const Rect2i &get_rect() const;

		void set_size( Vector2i size );
		void set_position( Vector2i size );

		void set_resizable( bool value );
		bool is_resizable() const;

		void set_decorated( bool value );
		bool is_decorated() const;

		// refreshes the windows position/size values, not the window's content
		// only call if unexpected values are returned from get_width/get_height/get_size/get_position/get_rect
		void refresh_rect();

		const std::string &get_title() const;
		void set_title( const std::string &title );

		WindowCallback_t get_callback() const;
		void set_callback( WindowCallback_t callback );

		bool is_deferred_to_close() const noexcept;
		bool is_focused() const noexcept;

		InputCallback_t get_input_callback() const;
		void set_input_callback( InputCallback_t callback );

		TimeMs_t get_creation_time() const noexcept;
		float get_shader_time() const noexcept;

		void hide();
		void show();

		// won't always return the value at m_visible_state
		WindowVisibiltyState get_visibility_state() const;

		void poll();

		void ping() const noexcept;

		Image to_image( const Rect2i rect ) const;
		Image to_image() const;

		// will remove the window and render this object as invalid
		void close() noexcept;

		// true if the window shouldn't close, otherwise false
		inline operator bool() const noexcept(Window::should_close) {
			return !this->should_close();
		}

	private:
		Window( void *const handle, const std::string &title, bool hidden ) noexcept;

		class WindowCallbackEngine;

		Window &operator=( const Window &other ) = delete;
		Window( const Window &copy ) = delete;

	private:
		void *m_hdl;
		bool m_hidden;
		WindowVisibiltyState m_visible_state;
		bool m_focused = true;
		Rect2i m_rect;
		std::string m_title{};
		Vector2f m_content_scale{ 1.0f, 1.0f };
		bool m_deferred_close{ false };

		const TimeMs_t m_creation_time;
		const float m_stp;

		WindowCallback_t m_callback = nullptr;
		InputCallback_t m_input_callback = nullptr;
	};
}
