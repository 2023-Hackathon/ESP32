## UUID

SERVICE_UUID        "02afd1d9-f889-4f49-acc7-33b34a620adb"
CHARACTERISTIC_UUID "3604da64-2145-422a-89c6-c540392c3a6a"

## Device Name

ESP32-Edge

## Data from ESP32

ESP32 will send data to another device based on the format shown below.

- Format: `1resistance1@resistance2@...`.
- Returns: A string starts with `'1'` (0x31) with information about a list of resistance values. Each resistance value is sent by using this format `resistance@`.
- Example: "45071@45022@45081@" means a list `[45071, 45022, 45081]` of resistance values
- Notes: There are at most 30 resistance information in this string, which means you can only access the resistance values in the last 30 seconds.