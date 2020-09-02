#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "../../M5ez.h"
#include "ezBLE.h"


bool ezBLE::control(uint8_t command, void* /* user */) {
	switch(command) {
		case EXTENSION_CONTROL_PING:
			return true;
		case EXTENSION_CONTROL_START:
			begin();
			return true;
		case EXTENSION_CONTROL_STOP:
			disconnect();
			return true;
		case EXTENSION_CONTROL_QUERY_ENABLED:
			return _on;
	}
	return false;
}

class M5ezClientCallback : public BLEClientCallbacks {
	void onConnect(BLEClient*) {}
	void onDisconnect(BLEClient*) {
		ezBLE::_cleanup();
	}
} _bleClientCallback;

// https://www.bluetooth.com/specifications/gatt/services/
const std::vector<std::pair<uint16_t, String>> ezBLE::_gattUuids = {
	{ 0x1800, "Generic Access" },
	{ 0x1811, "Alert Notification Service" },
	{ 0x1815, "Automation IO" },
	{ 0x180F, "Battery Service" },
	{ 0x183B, "Binary Sensor" },
	{ 0x1810, "Blood Pressure" },
	{ 0x181B, "Body Composition" },
	{ 0x181E, "Bond Management Service" },
	{ 0x181F, "Continuous Glucose Monitoring" },
	{ 0x1805, "Current Time Service" },
	{ 0x1818, "Cycling Power" },
	{ 0x1816, "Cycling Speed and Cadence" },
	{ 0x180A, "Device Information" },
	{ 0x183C, "Emergency Configuration" },
	{ 0x181A, "Environmental Sensing" },
	{ 0x1826, "Fitness Machine" },
	{ 0x1801, "Generic Attribute" },
	{ 0x1808, "Glucose" },
	{ 0x1809, "Health Thermometer" },
	{ 0x180D, "Heart Rate" },
	{ 0x1823, "HTTP Proxy" },
	{ 0x1812, "Human Interface Device" },
	{ 0x1802, "Immediate Alert" },
	{ 0x1821, "Indoor Positioning" },
	{ 0x183A, "Insulin Delivery" },
	{ 0x1820, "Internet Protocol Support Service" },
	{ 0x1803, "Link Loss" },
	{ 0x1819, "Location and Navigation" },
	{ 0x1827, "Mesh Provisioning Service" },
	{ 0x1828, "Mesh Proxy Service" },
	{ 0x1807, "Next DST Change Service" },
	{ 0x1825, "Object Transfer Service" },
	{ 0x180E, "Phone Alert Status Service" },
	{ 0x1822, "Pulse Oximeter Service" },
	{ 0x1829, "Reconnection Configuration" },
	{ 0x1806, "Reference Time Update Service" },
	{ 0x1814, "Running Speed and Cadence" },
	{ 0x1813, "Scan Parameters" },
	{ 0x1824, "Transport Discovery" },
	{ 0x1804, "Tx Power" },
	{ 0x181C, "User Data" },
	{ 0x181D, "Weight Scale" },
};

bool ezBLE::_on = false;
bool ezBLE::_initialized = false;
std::vector<BLEClient*> ezBLE::_clients;

void ezBLE::begin() {
	ezBLE::_readFlash();
	ez.settings.menuObj.addItem("BLE settings", ezBLE::menu);
	if (_on) {
		_refresh();
	}
}

void ezBLE::menu() {
	bool on_orig = _on;
	while(true) {
		ezMenu blemenu("BLE Settings");
		blemenu.txtSmall();
		blemenu.addItem("on|Turn On\t" + (String)(_on ? "on" : "off"));
		if (_on) {
			blemenu.addItem("Scan and connect", NULL, _scan);
			blemenu.addItem("Clients", NULL, _listClients);
		}
		blemenu.buttons("up#Back#select##down#");
		switch (blemenu.runOnce()) {
			case 1:
				_on = !_on;
				_refresh();
				break;
			case 0:
				if (_on != on_orig) {
					_writeFlash();
				}
				return;
		}
	}
}

void ezBLE::disconnect() {
	for (auto& client : _clients) {
		if (client->isConnected())
			client->disconnect();
	}
	_clients.clear();
}

BLEClient* ezBLE::getClient(uint16_t index) {
	if (index >= _clients.size())
		return nullptr;
	return _clients[index];
}

uint16_t ezBLE::getClientCount() {
	return static_cast<uint16_t>(_clients.size());
}

void ezBLE::_readFlash() {
	Preferences prefs;
	prefs.begin("M5ez", true);	// true: read-only
	_on = prefs.getBool("ble_on", false);
	prefs.end();
}

void ezBLE::_writeFlash() {
	Preferences prefs;
	prefs.begin("M5ez");
	prefs.putBool("ble_on", _on);
	prefs.end();
}

bool ezBLE::_scan(ezMenu* callingMenu) {
	BLEScan* bleScan = BLEDevice::getScan(); //create new scan
	bleScan->setActiveScan(true); //active scan uses more power, but get results faster
	bleScan->setInterval(100);
	bleScan->setWindow(99);  // less or equal setInterval value
	ez.msgBox("Bluetooth", "Scanning ...", "");
	BLEScanResults foundDevices = bleScan->start(5, false);
	ezMenu devicesmenu("Found " + String(foundDevices.getCount()) + " devices");
	devicesmenu.txtSmall();
	for (int i = 0; i < foundDevices.getCount(); i++) {
		const char* name = nullptr;
		if (foundDevices.getDevice(i).getName().size() == 0)
			name = foundDevices.getDevice(i).getAddress().toString().c_str();
		else
			name = foundDevices.getDevice(i).getName().c_str();
		devicesmenu.addItem(name);
	}
	devicesmenu.buttons("up#Back#select##down#");
	if (devicesmenu.runOnce()) {
		BLEAdvertisedDevice device = foundDevices.getDevice(devicesmenu.pick() - 1);
		_connect(device);
	}
	bleScan->clearResults();
	return true;
}

void ezBLE::_connect(class BLEAdvertisedDevice& device) {
	ez.msgBox("Bluetooth", "Connecting ...", "");
	BLEClient* client = BLEDevice::createClient();
	client->setClientCallbacks(&_bleClientCallback);
	if (client->connect(&device)) {
		_clients.push_back(client);
		ez.msgBox("Bluetooth", "Device connected.");
	} else {
		delete client;
		ez.msgBox("Bluetooth", "Connection failed!");
	}
}

bool ezBLE::_listClients(ezMenu* callingMenu) {
	ezMenu clientsmenu("Clients");
	clientsmenu.txtSmall();
	for (auto& client : _clients) {
		clientsmenu.addItem(client->getPeerAddress().toString().c_str());
	}
	clientsmenu.buttons("up#Back#select##down#");
	while (clientsmenu.runOnce()) {
		if (!_showClient(_clients[clientsmenu.pick() - 1]))
			return false;
	}
	return true;
}

bool ezBLE::_showClient(BLEClient* client) {
	ezMenu clientmenu(String(client->getPeerAddress().toString().c_str()));
	clientmenu.txtSmall();
	clientmenu.buttons("up#Back#select##down#");
	auto& services = *client->getServices();
	for (auto& service : services) {
		bool found = false;
		for (auto& pair : _gattUuids) {
			if (BLEUUID(pair.first).equals(service.second->getUUID())) {
				clientmenu.addItem(pair.second);
				found = true;
				break;
			}
		}
		if (!found)
			clientmenu.addItem(service.first.c_str());
	}
	clientmenu.addItem("Disconnect");
	while (clientmenu.runOnce()) {
		if (clientmenu.pickName() == "Disconnect") {
			client->disconnect();
			return false;
		}
	}
	return true;
}

void ezBLE::_cleanup() {
	if (_clients.size() > 0) {
		_clients.erase(std::remove_if(_clients.begin(), _clients.end(), [](BLEClient* client) {
			bool shouldRemove = !client->isConnected();
			if (shouldRemove)
				delete client;
			return shouldRemove;
		}), _clients.end());
	}
}

void ezBLE::_refresh() {
	if (_on) {
		if (!_initialized) {
			_initialized = true;
			BLEDevice::init(M5EZ_BLE_DEVICE_NAME);
		}
	} else {
		disconnect();
		if (_initialized) {
			_initialized = false;
			_cleanup();
			BLEDevice::deinit(false);
		}
	}
}
