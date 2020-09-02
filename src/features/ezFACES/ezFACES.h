#pragma once

// coupling:
// To determine if the keyboard is enabled, call ez.tell("ezFACES", FEATURE_MSG_QUERY_ENABLED, nullptr)
// poll the ezFACES feature by calling ez.tell("ezFACES", FEATURE_MSG_FACES_POLL, (void*)(String* data))

#define FEATURE_MSG_FACES_POLL	10

class ezFACES {
	public:
		static bool entry(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static String poll();
		static bool on();
	private:
		static bool _on;
};
