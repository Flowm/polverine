## POLVERINE_FULL_BLE_DEMO: BLE Sensors Data Collection

### An easy to use Solution

You can use Serial Bluetooth Terminal app in Android and iOS with custom BLE serial profile to receive the data.

Data is published using BLE serial emulation in topics that include the device's unique SENSORID (specifically, the last six characters of its MAC address).
 
1) polverine/_SENSORID_/bmv080
2) polverine/_SENSORID_/bme690

The message contains the data in JSON format:

1) bmv080 message

```json
{
"topic":"bmv080",
"data":{
		"ID": "54ADD8",
		"R": 9131.2,
		"PM10": 32,
		"PM25": 24,
		"PM1": 11,
		"obst": "no",
		"omr": "no",
		"T": 11.1
	}
}
```

2) bme690 message

```json
{
"topic":"bme690",
"data":{	
	  "ID": "54ADD8",
	  "R": 8556.6,
	  "T": 6.25,
	  "P": 98956.57,
	  "H": 98.75,
	  "IAQ": 73.9,
	  "ACC": 3,
	  "CO2": 1026.85,
	  "VOC": 1.76
	}
}
```
