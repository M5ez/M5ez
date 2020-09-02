#pragma once

#define M5EZ_BLE_DEVICE_NAME "M5ez"

class ezBLE {
	public:
		static bool entry(uint8_t command, void* /* user */);
		static void begin();
		static void menu();
		static void disconnect();
		static class BLEClient* getClient(uint16_t index);
		static uint16_t getClientCount();
	private:
		static void _readFlash();
		static void _writeFlash();
		static const std::vector<std::pair<uint16_t, String>> _gattUuids;
		static bool _on;
		static bool _initialized;
		static std::vector<class BLEClient*> _clients;
		static bool _scan(ezMenu* callingMenu);
		static void _connect(class BLEAdvertisedDevice& device);
		static bool _listClients(ezMenu* callingMenu);
		static bool _showClient(class BLEClient* client);
		static void _cleanup();
		static void _refresh();
		friend class M5ezClientCallback;
};
