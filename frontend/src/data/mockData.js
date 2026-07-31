export const sensorCards = [
  { label: 'LED Light', value: 'ON', unit: '', icon: 'Lightbulb', state: 'Relay output active', tone: 'blue' },
  { label: 'Servo Motor', value: 'Open', unit: '', icon: 'SlidersHorizontal', state: 'Valve angle 90 deg', tone: 'indigo' },
  { label: 'Temperature', value: '28.4', unit: 'C', icon: 'Thermometer', state: 'Normal', tone: 'blue' },
  { label: 'Humidity', value: '64', unit: '%', icon: 'Droplets', state: 'Comfort', tone: 'cyan' },
  { label: 'Smoke Level', value: '18', unit: 'ppm', icon: 'Wind', state: 'Safe', tone: 'green' },
  { label: 'Mini Water Pump', value: 'OFF', unit: '', icon: 'Droplets', state: 'Pump off', tone: 'cyan' },
]

export const initialDevices = [
  { id: 'led', name: 'LED Light', description: 'Relay CH1', icon: 'Lightbulb', status: true, metric: 'ON', tone: 'blue' },
  { id: 'servo', name: 'Servo Motor', description: 'Open / close valve', icon: 'SlidersHorizontal', status: true, metric: 'OPEN', tone: 'indigo' },
  { id: 'pump', name: 'Mini Water Pump', description: 'Mini Water Pump 5VDC', icon: 'Droplets', status: false, metric: 'OFF', tone: 'cyan' },
]

export const seriesLegend = [
  { key: 'temperature', label: 'Temperature', value: '28.4 C', color: '#dc2626' },
  { key: 'humidity', label: 'Humidity', value: '64%', color: '#2563eb' },
  { key: 'smoke', label: 'Smoke', value: '18 ppm', color: '#64748b' },
]



export const automationRules = [
  {
    id: 'high-temperature',
    name: 'High temperature pump assist',
    description: 'Turn on mini water pump when room temperature is too high.',
    sensor: 'Temperature',
    operator: '>',
    threshold: 35,
    unit: 'C',
    action: 'Turn mini water pump ON',
    enabled: true,
  },
  {
    id: 'fire-smoke',
    name: 'Smoke emergency alert',
    description: 'Trigger buzzer and show dashboard alert when smoke exceeds safety limit.',
    sensor: 'Smoke',
    operator: '>',
    threshold: 30,
    unit: 'ppm',
    action: 'Buzzer ON',
    enabled: true,
  },
  {
    id: 'servo-release',
    name: 'Auto water valve release',
    description: 'Open servo valve when fire condition is confirmed.',
    sensor: 'Smoke',
    operator: '>',
    threshold: 45,
    unit: 'ppm',
    action: 'Open servo valve',
    enabled: false,
  },
  {
    id: 'humidity-comfort',
    name: 'Humidity comfort notice',
    description: 'Show notice when humidity drops below comfort level.',
    sensor: 'Humidity',
    operator: '<',
    threshold: 40,
    unit: '%',
    action: 'Notify dashboard',
    enabled: true,
  },
]

export const automationSummary = [
  { label: 'Active rules', value: '3 / 4', icon: 'SlidersHorizontal', tone: 'blue' },
  { label: 'Smoke threshold', value: '30 ppm', icon: 'Wind', tone: 'green' },
  { label: 'Servo action', value: 'Manual confirm', icon: 'ShieldCheck', tone: 'indigo' },
]

export const settingsGroups = [
  {
    title: 'MQTT Connection',
    icon: 'Radio',
    fields: [
      { label: 'Broker', value: 'mqtt://localhost:1883' },
      { label: 'Device topic', value: 'aiot/esp32-s3/device' },
      { label: 'Telemetry topic', value: 'aiot/esp32-s3/telemetry' },
    ],
  },
  {
    title: 'ESP32-S3 Device',
    icon: 'Wifi',
    fields: [
      { label: 'Device ID', value: 'ESP32S3-LAB-01' },
      { label: 'Wi-Fi status', value: 'Connected' },
      { label: 'Telemetry interval', value: '1 second + WebSocket' },
    ],
  },
  {
    title: 'Security',
    icon: 'Lock',
    fields: [
      { label: 'Authentication', value: 'JWT enabled' },
      { label: 'Password hashing', value: 'BCrypt' },
      { label: 'Voice privacy', value: 'Local processing only' },
    ],
  },
  {
    title: 'Database',
    icon: 'Database',
    fields: [
      { label: 'Engine', value: 'PostgreSQL mock' },
      { label: 'Sensor retention', value: '30 days' },
      { label: 'Backup schedule', value: 'Daily at 01:00' },
    ],
  },
]

export const navItems = [
  { id: 'dashboard', label: 'Dashboard', icon: 'Home' },
  { id: 'history', label: 'History', icon: 'History' },
  { id: 'automation', label: 'Automation', icon: 'SlidersHorizontal' },
  { id: 'settings', label: 'Settings', icon: 'Settings' },
]
