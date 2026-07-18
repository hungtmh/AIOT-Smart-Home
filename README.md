# AIOT Smart Home

Du an demo AIoT Smart Home voi luong dieu khien thiet bi that/gia lap:

```text
React frontend
  -> Spring Boot backend
    -> Supabase Auth JWT
    -> Supabase PostgreSQL
    -> HiveMQ MQTT broker
      -> ESP32/Wokwi subscribe command topic
      -> ESP32/Wokwi dieu khien LED, servo, buzzer, pump
      -> ESP32/Wokwi publish state topic ve backend
```

## 1. Cau truc thu muc

```text
backend/   Spring Boot API, JWT security, Supabase PostgreSQL, MQTT bridge
frontend/  React + Vite dashboard
wokwi/     Wokwi ESP32 MQTT simulator
arduino/   Sketch cho board that
```

## 2. Yeu cau cai dat

Can cai truoc:

```text
Java 17
Node.js 20+ hoac 22+
npm
Git
```

Neu chay thiet bi that thi can them:

```text
Arduino IDE
ESP8266/ESP32 board package
PubSubClient library
```

Neu test gia lap thi dung:

```text
Wokwi
```

## 3. Clone va cai dependency

```powershell
git clone <repo-url>
cd AIOT-Smart-Home

cd frontend
npm install
```

Backend dung Maven Wrapper nen khong can cai Maven rieng.

## 4. Nhan file .env

Nguoi maintain project se gui rieng 2 file:

```text
backend/.env
frontend/.env
```

Dat dung vi tri:

```text
AIOT-Smart-Home/backend/.env
AIOT-Smart-Home/frontend/.env
```

Khong commit 2 file nay len Git.

### backend/.env can co

```env
SUPABASE_DB_URL=jdbc:postgresql://<supabase-pooler-host>:5432/postgres?sslmode=require
SUPABASE_DB_USERNAME=postgres.<project-ref>
SUPABASE_DB_PASSWORD=<database-password>
SUPABASE_JWT_SECRET=<legacy-jwt-secret>
SUPABASE_JWKS_URI=https://<project-ref>.supabase.co/auth/v1/.well-known/jwks.json

MQTT_BROKER_URI=ssl://<hivemq-host>:8883
MQTT_USERNAME=<mqtt-username>
MQTT_PASSWORD=<mqtt-password>
MQTT_COMMAND_TOPIC_PATTERN=aiot/esp32-s3/device/%s/set
MQTT_STATE_TOPIC_PATTERN=aiot/esp32-s3/device/%s/state

CORS_ALLOWED_ORIGINS=http://localhost:5173,http://127.0.0.1:5173
```

### frontend/.env can co

```env
VITE_API_BASE_URL=http://localhost:8080
VITE_SUPABASE_URL=https://<project-ref>.supabase.co
VITE_SUPABASE_ANON_KEY=<supabase-publishable-or-anon-key>
```

Frontend chi dung publishable/anon key. Khong dua secret key vao frontend.

## 5. Tao database schema tren Supabase

Vao Supabase SQL Editor, copy noi dung file:

```text
backend/database/supabase_schema.sql
```

Chay SQL do mot lan de tao:

```text
devices
device_states
control_logs
```

## 6. Tao tai khoan dang nhap

Vao Supabase:

```text
Authentication -> Users -> Add user
```

Tao email/password cho tung nguoi duoc phep vao dashboard.

Chi tai khoan tao trong Supabase Auth moi login frontend duoc. Backend se kiem tra JWT tren moi request `/api/**`.

## 7. Chay backend

Mo terminal 1:

```powershell
cd AIOT-Smart-Home\backend
.\run-dev.ps1
```

Backend chay o:

```text
http://localhost:8080
```

Log thanh cong thuong co:

```text
Tomcat started on port 8080
Connected to MQTT broker ...
HikariPool-1 - Start completed
```

Test API khong co JWT se bi 401, day la dung:

```powershell
Invoke-WebRequest http://localhost:8080/api/devices/led
```

## 8. Chay frontend

Mo terminal 2:

```powershell
cd AIOT-Smart-Home\frontend
npm run dev
```

Mo URL Vite hien tren terminal, thuong la:

```text
http://localhost:5173
```

Dang nhap bang tai khoan da tao trong Supabase Auth.

## 9. Chay Wokwi simulator

Mo project:

```text
wokwi/esp32_led_mqtt
```

File chinh:

```text
wokwi/esp32_led_mqtt/sketch.ino
wokwi/esp32_led_mqtt/diagram.json
wokwi/esp32_led_mqtt/libraries.txt
```

Can dam bao trong `sketch.ino` MQTT username/password trung voi `backend/.env`.

Khi Wokwi chay thanh cong, Serial Monitor se co dang:

```text
[WiFi] Connected
[MQTT] Connecting to HiveMQ Cloud... connected
[MQTT] Subscribed to device command topics
```

Sau do bam LED/servo/buzzer/pump tren frontend. Wokwi se nhan message:

```text
[MQTT] Message on aiot/esp32-s3/device/led/set: ON
[LED] ON
[MQTT] Published state: ON
```

## 10. Luong dieu khien thiet bi

```text
1. User login frontend bang Supabase Auth
2. Frontend nhan access_token JWT
3. User bam LED/servo/buzzer/pump
4. Frontend goi POST /api/devices/{deviceId}/command kem Authorization: Bearer <JWT>
5. Spring Security verify JWT bang Supabase JWKS/secret
6. DeviceController goi DeviceService
7. DeviceService luu desired_state vao Supabase PostgreSQL
8. DeviceService goi MqttCommandPublisher publish ON/OFF len MQTT command topic
9. ESP32/Wokwi dang subscribe topic do nen nhan ON/OFF
10. ESP32/Wokwi dieu khien dung thiet bi
11. ESP32/Wokwi publish reported state ve MQTT state topic
12. Backend nhan state va cap nhat reported_state
```

## 11. API chinh

Can JWT hop le:

```http
GET /api/auth/me
GET /api/devices
GET /api/devices/led
GET /api/devices/pump
POST /api/devices/{deviceId}/command
Content-Type: application/json

{ "state": true }
```

`desiredState` la trang thai user muon.

`reportedState` la trang thai thiet bi/Wokwi bao ve.

## 12. Loi thuong gap

### Port 8080 bi chiem

Loi:

```text
Identify and stop the process that's listening on port 8080
```

Xu ly:

```powershell
Get-NetTCPConnection -LocalPort 8080 -State Listen | Select-Object LocalAddress,LocalPort,OwningProcess
Stop-Process -Id <PID>
```

### Frontend bao JWT rejected

Thu lam:

```text
1. Ctrl+C frontend dev server
2. npm run dev lai
3. Sign out
4. Login lai bang tai khoan Supabase Auth
```

Neu van loi, xem log:

```text
backend/backend-dev.log
```

Log se co thong tin:

```text
JWT alg=... kid=...
```

Token Supabase moi thuong dung `ES256`, backend da verify bang:

```text
SUPABASE_JWKS_URI=https://<project-ref>.supabase.co/auth/v1/.well-known/jwks.json
```

### Backend connect DB loi

Nen dung Supabase pooler URL thay vi direct host neu mang/may khong ho tro IPv6 tot:

```env
SUPABASE_DB_URL=jdbc:postgresql://<pooler-host>:5432/postgres?sslmode=require
SUPABASE_DB_USERNAME=postgres.<project-ref>
```

### MQTT connected nhung thiet bi khong doi

Kiem tra topic trong 3 noi phai trung nhau:

```text
wokwi/esp32_led_mqtt/sketch.ino
HiveMQ test client neu co
```

Command topic pattern:

```text
aiot/esp32-s3/device/{deviceId}/set
```

State topic pattern:

```text
aiot/esp32-s3/device/{deviceId}/state
```

Device ids hien co:

```text
led
servo
buzzer
pump
```

## 13. Lenh build kiem tra

Backend:

```powershell
cd backend
.\mvnw.cmd -DskipTests package
```

Frontend:

```powershell
cd frontend
npm run build
```
