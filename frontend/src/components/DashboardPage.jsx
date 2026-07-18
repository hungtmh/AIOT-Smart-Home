import { useEffect, useState } from 'react'
import { recentActivityRows, sensorCards } from '../data/mockData'
import DeviceControlCenter from './DeviceControlCenter'
import { icons } from './icons'
import { getDeviceStates, getLatestTelemetry } from '../lib/api'
import MetricGrid from './MetricGrid'
import SensorTimeline from './SensorTimeline'

const {
  Activity,
  AlertTriangle,
  BellRing,
  Clock3,
  Gauge,
  Menu,
  Mic,
  Moon,
  Search,
  SlidersHorizontal,
  Waves,
} = icons

function deviceOn(deviceStates, deviceId) {
  const device = deviceStates.find((state) => state.deviceId === deviceId)
  return Boolean(device?.reportedState)
}

function buildLiveSensorCards(telemetry, deviceStates) {
  const ledOn = deviceOn(deviceStates, 'led')
  const servoOpen = deviceOn(deviceStates, 'servo')
  const buzzerOn = deviceOn(deviceStates, 'buzzer')
  const temperature = telemetry?.temperature ?? 28.4
  const humidity = telemetry?.humidity ?? 64
  const smokePpm = telemetry?.smokePpm ?? 18

  return sensorCards.map((card) => {
    if (card.label === 'LED Light') {
      return { ...card, value: ledOn ? 'ON' : 'OFF', state: ledOn ? 'Relay output active' : 'Relay output off' }
    }

    if (card.label === 'Servo Motor') {
      return { ...card, value: servoOpen ? 'Open' : 'Close', state: servoOpen ? 'Valve angle 90 deg' : 'Valve angle 0 deg' }
    }

    if (card.label === 'Temperature') {
      return { ...card, value: temperature.toFixed(1), state: temperature >= 35 ? 'High' : 'Normal' }
    }

    if (card.label === 'Humidity') {
      return { ...card, value: Math.round(humidity).toString(), state: humidity >= 40 && humidity <= 70 ? 'Comfort' : 'Check' }
    }

    if (card.label === 'Smoke Level') {
      return { ...card, value: Math.round(smokePpm).toString(), state: smokePpm >= 70 ? 'Danger' : 'Safe' }
    }

    if (card.label === 'Buzzer') {
      return { ...card, value: buzzerOn ? 'ON' : 'OFF', state: buzzerOn ? 'Alarm active' : 'No alarm' }
    }

    return card
  })
}

function formatUpdatedAt(value, fallback) {
  if (!value) return fallback

  return new Intl.DateTimeFormat('vi-VN', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
  }).format(new Date(value))
}

function DashboardPage({ autoMode, lastUpdated, onToggleAutoMode }) {
  const [liveSensors, setLiveSensors] = useState(sensorCards)
  const [latestTelemetry, setLatestTelemetry] = useState(null)

  useEffect(() => {
    let cancelled = false

    async function syncDashboardData() {
      try {
        const [telemetry, deviceStates] = await Promise.all([getLatestTelemetry(), getDeviceStates()])
        if (cancelled) return

        setLatestTelemetry(telemetry)
        setLiveSensors(buildLiveSensorCards(telemetry, deviceStates))
      } catch {
        if (!cancelled) {
          setLiveSensors(sensorCards)
        }
      }
    }

    syncDashboardData()
    const intervalId = window.setInterval(syncDashboardData, 5000)

    return () => {
      cancelled = true
      window.clearInterval(intervalId)
    }
  }, [])

  const smokePpm = Math.round(latestTelemetry?.smokePpm ?? 18)
  const smokeSafe = smokePpm < 70
  const displayLastUpdated = formatUpdatedAt(latestTelemetry?.measuredAt, lastUpdated)

  return (
    <>
      <header className="dashboard-header">
        <div>
          <div className="eyebrow">
            <Activity size={16} aria-hidden="true" />
            Live Wokwi telemetry
          </div>
          <h1>Sensor Dashboard</h1>
          <p>Last updated: {displayLastUpdated}</p>
        </div>
        <div className="header-actions">
          <button type="button" aria-label="Search">
            <Search size={18} aria-hidden="true" />
          </button>
          <button type="button" aria-label="Notifications">
            <BellRing size={18} aria-hidden="true" />
          </button>
          <button type="button" className="menu-button" aria-label="Menu">
            <Menu size={19} aria-hidden="true" />
          </button>
        </div>
      </header>

      <MetricGrid sensors={liveSensors} />

      <section className="status-layout" aria-label="System status">
        <article className="panel automation-panel">
          <div className="panel-title">
            <div>
              <SlidersHorizontal size={19} aria-hidden="true" />
              <h2>Automation Control</h2>
            </div>
            <button
              className={`switch ${autoMode ? 'on' : ''}`}
              type="button"
              aria-pressed={autoMode}
              onClick={onToggleAutoMode}
            >
              <span></span>
            </button>
          </div>
          <div className="automation-body">
            <span className="badge success">Auto Mode: {autoMode ? 'ON' : 'OFF'}</span>
            <span className="badge info">User Interaction: Inactive</span>
            <span className="badge neutral">Threshold Profile: Home</span>
          </div>
        </article>

        <article className="panel">
          <div className="panel-title">
            <div>
              <Mic size={19} aria-hidden="true" />
              <h2>Latest Voice Command</h2>
            </div>
            <span className="time-pill">0.8s</span>
          </div>
          <div className="voice-command">
            <strong>Turn on LED light</strong>
            <p>Mapped to LED relay, confidence 97%</p>
          </div>
        </article>
      </section>

      <DeviceControlCenter />
      <SensorTimeline />

      <section className="bottom-layout" aria-label="Safety and history">
        <article className="panel safety-panel">
          <div className="panel-title">
            <div>
              <AlertTriangle size={19} aria-hidden="true" />
              <h2>Fire Safety Monitor</h2>
            </div>
            <span className="safe-pill">{smokeSafe ? 'Safe' : 'Danger'}</span>
          </div>
          <div className="safety-gauge">
            <div>
              <Gauge size={36} aria-hidden="true" />
              <strong>{smokePpm} ppm</strong>
              <span>Smoke threshold 70 ppm</span>
            </div>
            <div>
              <Waves size={36} aria-hidden="true" />
              <strong>Standby</strong>
              <span>Water valve servo</span>
            </div>
            <div>
              <Moon size={36} aria-hidden="true" />
              <strong>Clear</strong>
              <span>Flame sensor</span>
            </div>
          </div>
        </article>

        <article className="panel history-panel">
          <div className="panel-title">
            <div>
              <Clock3 size={19} aria-hidden="true" />
              <h2>Recent Activity</h2>
            </div>
          </div>
          <div className="history-list">
            {recentActivityRows.map((row) => (
              <div className="history-row" key={`${row.time}-${row.event}`}>
                <time>{row.time}</time>
                <div>
                  <strong>{row.event}</strong>
                  <span>{row.detail}</span>
                </div>
              </div>
            ))}
          </div>
        </article>
      </section>
    </>
  )
}

export default DashboardPage
