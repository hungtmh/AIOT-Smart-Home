# AIOT Smart Home Backend

Minimal Spring Boot backend for the first real device-control flow:

```text
React LED switch
  -> Spring Boot REST API
  -> MQTT command topic
  -> ESP32/Arduino turns LED on/off
  -> ESP32 publishes reported state
  -> Spring Boot stores state/logs in Supabase PostgreSQL
```

## 1. Supabase

Open Supabase SQL Editor and run:

```sql
-- backend/database/supabase_schema.sql
```

Then set these environment variables before starting Spring Boot:

```powershell
$env:SUPABASE_DB_URL="jdbc:postgresql://aws-0-ap-southeast-1.pooler.supabase.com:5432/postgres"
$env:SUPABASE_DB_USERNAME="postgres.your-project-ref"
$env:SUPABASE_DB_PASSWORD="your-supabase-database-password"
```

Use the Supabase connection pooler URL if your network has trouble with direct IPv6.

## 2. MQTT

For a local broker:

```powershell
$env:MQTT_BROKER_URI="tcp://localhost:1883"
```

For a cloud broker, set the broker URI, username, and password:

```powershell
$env:MQTT_BROKER_URI="ssl://your-broker-host:8883"
$env:MQTT_USERNAME="your-mqtt-username"
$env:MQTT_PASSWORD="your-mqtt-password"
```

Default topics:

```text
Command: aiot/esp32-s3/device/led/set
State:   aiot/esp32-s3/device/led/state
```

Payload is intentionally simple for Arduino: `ON` or `OFF`.

## 3. Run

```powershell
cd backend
.\mvnw.cmd spring-boot:run
```

API:

```http
GET  http://localhost:8080/api/devices/led
POST http://localhost:8080/api/devices/led/command
Content-Type: application/json

{ "state": true }
```
