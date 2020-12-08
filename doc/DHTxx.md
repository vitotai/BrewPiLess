# DHT11/DHT21(AM2301)/DHT22 sensors
DHT sensor family uses proprietary onewire protocol. For maximum flexibility, there is no dedidcated pin for it. 
Generic PINs that are used for actuators(cooling, heating, and etc.) can be used for DHTxx sensors.
Once assigned, the humidity reading is then available.

DHT sensors also report temperature, and the temperature sensor is available after a PIN is assigned to the DHTxx sensor.

# Humidity Control
Humidity control runs similar algorithm as temperature control. The parameters are currently predefined and can be changed only by modification of source code..
"Humidifer" and/or "Dehumidifer" must be set-up for humidity control. 

| Parameter   | Usage | Default value       |
| -------------- |:-------------:| :--------------------|
| IdleLow       | maximum difference before Humidifying starts     | 3%			 |
| IdleHigh      | maximum difference before Dehumidifying starts     | 3%		 |
| HumidifyingTargetHigh       | the minimum difference before stopping humidifying     | 5%			 |
| DehumidifyingTargetLow      | the minimum difference before stopping dehumidifying   | 5%		 |
| MinimumHumidifyingRunningTime      |  Minimum humidifying running time    | 60 secs		 |
| MinimumHumidifyingIdleTime      |  Minimum humidifying Idle time    | 60 secs		 |
| MinimumDehumidifyingRunningTime      |  Minimum dehumidifying running time    | 300 secs		 |
| MinimumDehumidifyingIdleTime      |  Minimum dehumidifying idle time    | 300 secs		 |
| MinimumDeadTime      |  Minimum idle time between humidifying and defhumdifying    | 600 secs		 |

# DHT11/DHT21(AM2301)/DHT22 sensors
DHT sensor family uses proprietary onewire protocol. For maximum flexibility, there is no dedidcated pin for it. 
Generic PINs that are used for actuators(cooling, heating, and etc.) can be used for DHTxx sensors.
Once assigned, the humidity reading is then available.

DHT sensors also report temperature, and the temperature sensor is available after a PIN is assigned to the DHTxx sensor.

# Humidity Control
Humidity control runs similar algorithm as temperature control. The parameters are currently predefined and can be changed only by modification of source code..
"Humidifer" and/or "Dehumidifer" must be set-up for humidity control. 

| Parameter   | Usage | Default value       |
| -------------- |:-------------:| :--------------------|
| IdleLow       | maximum difference before Humidifying starts     | 3%			 |
| IdleHigh      | maximum difference before Dehumidifying starts     | 3%		 |
| HumidifyingTargetHigh       | the minimum difference before stopping humidifying     | 5%			 |
| DehumidifyingTargetLow      | the minimum difference before stopping dehumidifying   | 5%		 |
| MinimumHumidifyingRunningTime      |  Minimum humidifying running time    | 60 secs		 |
| MinimumHumidifyingIdleTime      |  Minimum humidifying Idle time    | 60 secs		 |
| MinimumDehumidifyingRunningTime      |  Minimum dehumidifying running time    | 300 secs		 |
| MinimumDehumidifyingIdleTime      |  Minimum dehumidifying idle time    | 300 secs		 |
| MinimumDeadTime      |  Minimum idle time between humidifying and defhumdifying    | 600 secs		 |

Eg. for humdifying,
* humidifier is started when the current humidity is lower than (TargetHumidity - IdleLow)
* humidifier is stopped when the current humidity is higher than (TargetHumidity + HumidifyingTargetHight)

for dehumdifying,
* dehumidifier is started when the current humidity is higher than (TargetHumidity + IdleHigh)
* dehumidifier is stopped when the current humidity is lower than (TargetHumidity - DehumidifyingTargetLow)
