#pragma once

// coupling:
// To determine if the keyboard is enabled, call ez.extensionControl("ezFACES", EXTENSION_CONTROL_QUERY_ENABLED, nullptr)
// poll the ezFACES extension by calling ez.extensionControl("ezFACES", EXTENSION_CONTROL_FACES_POLL, (void*)(String* data))

#define EXTENSION_CONTROL_FACES_POLL	10

class ezFACES {
	public:
		static bool control(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static String poll();
		static bool on();
	private:
		static bool _on;
};
