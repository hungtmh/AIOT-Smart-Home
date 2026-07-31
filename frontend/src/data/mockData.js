export const sensorCards = [
  { label: 'LED Light', value: 'ON', unit: '', icon: 'Lightbulb', state: 'Relay output active', tone: 'blue' },
  { label: 'Servo Motor', value: 'Open', unit: '', icon: 'SlidersHorizontal', state: 'Valve angle 90 deg', tone: 'indigo' },
  { label: 'Temperature', value: '28.4', unit: 'C', icon: 'Thermometer', state: 'Normal', tone: 'blue' },
  { label: 'Humidity', value: '64', unit: '%', icon: 'Droplets', state: 'Comfort', tone: 'cyan' },
  { label: 'Smoke Level', value: '18', unit: 'ppm', icon: 'Wind', state: 'Safe', tone: 'green' },
  { label: 'Buzzer', value: 'OFF', unit: '', icon: 'BellRing', state: 'No alarm', tone: 'amber' },
]

export const initialDevices = [
  { id: 'led', name: 'LED Light', description: 'Relay CH1', icon: 'Lightbulb', status: true, metric: 'ON', tone: 'blue' },
  { id: 'servo', name: 'Servo Motor', description: 'Open / close valve', icon: 'SlidersHorizontal', status: true, metric: 'OPEN', tone: 'indigo' },
  { id: 'buzzer', name: 'Buzzer Alarm', description: 'Fire alert output', icon: 'BellRing', status: false, metric: 'OFF', tone: 'amber' },
  { id: 'pump', name: 'Mini Water Pump', description: 'Mini Water Pump 5VDC', icon: 'Droplets', status: false, metric: 'OFF', tone: 'cyan' },
]

export const seriesLegend = [
  { key: 'temperature', label: 'Temperature', value: '28.4 C', color: '#dc2626' },
  { key: 'humidity', label: 'Humidity', value: '64%', color: '#2563eb' },
  { key: 'smoke', label: 'Smoke', value: '18 ppm', color: '#64748b' },
]

export const chartSeries = [
  {
    key: 'temperature',
    label: 'Temperature',
    unit: 'C',
    color: '#dc2626',
    points: [
      { x: 70, y: 186, value: '28.0', time: '21:15:30' },
      { x: 160, y: 183, value: '28.2', time: '21:16:00' },
      { x: 250, y: 184, value: '28.1', time: '21:16:30' },
      { x: 340, y: 178, value: '28.5', time: '21:17:00' },
      { x: 430, y: 176, value: '28.7', time: '21:17:30' },
      { x: 520, y: 182, value: '28.3', time: '21:18:00' },
      { x: 610, y: 188, value: '27.9', time: '21:18:30' },
      { x: 700, y: 190, value: '27.8', time: '21:19:00' },
      { x: 790, y: 185, value: '28.1', time: '21:19:30' },
      { x: 900, y: 187, value: '28.0', time: '21:20:00' },
    ],
  },
  {
    key: 'humidity',
    label: 'Humidity',
    unit: '%',
    color: '#2563eb',
    points: [
      { x: 70, y: 82, value: '66', time: '21:15:30' },
      { x: 160, y: 88, value: '65', time: '21:16:00' },
      { x: 250, y: 91, value: '64', time: '21:16:30' },
      { x: 340, y: 95, value: '63', time: '21:17:00' },
      { x: 430, y: 102, value: '61', time: '21:17:30' },
      { x: 520, y: 96, value: '63', time: '21:18:00' },
      { x: 610, y: 100, value: '62', time: '21:18:30' },
      { x: 700, y: 94, value: '64', time: '21:19:00' },
      { x: 790, y: 101, value: '62', time: '21:19:30' },
      { x: 900, y: 98, value: '63', time: '21:20:00' },
    ],
  },
  {
    key: 'smoke',
    label: 'Smoke',
    unit: 'ppm',
    color: '#64748b',
    points: [
      { x: 70, y: 216, value: '14', time: '21:15:30' },
      { x: 160, y: 218, value: '13', time: '21:16:00' },
      { x: 250, y: 213, value: '16', time: '21:16:30' },
      { x: 340, y: 211, value: '17', time: '21:17:00' },
      { x: 430, y: 205, value: '20', time: '21:17:30' },
      { x: 520, y: 198, value: '24', time: '21:18:00' },
      { x: 610, y: 207, value: '19', time: '21:18:30' },
      { x: 700, y: 214, value: '15', time: '21:19:00' },
      { x: 790, y: 210, value: '18', time: '21:19:30' },
      { x: 900, y: 216, value: '14', time: '21:20:00' },
    ],
  },
]

export const timelineRows = [
  { time: '21:20:00', temperature: '28.4 C', humidity: '64%', smoke: '18 ppm' },
  { time: '21:19:30', temperature: '28.2 C', humidity: '63%', smoke: '17 ppm' },
  { time: '21:19:00', temperature: '27.9 C', humidity: '64%', smoke: '19 ppm' },
  { time: '21:18:30', temperature: '29.1 C', humidity: '61%', smoke: '24 ppm' },
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
