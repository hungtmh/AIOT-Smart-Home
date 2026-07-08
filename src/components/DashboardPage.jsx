import { recentActivityRows, sensorCards } from '../data/mockData'
import DeviceControlCenter from './DeviceControlCenter'
import { icons } from './icons'
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

function DashboardPage({ autoMode, lastUpdated, onToggleAutoMode }) {
  return (
    <>
      <header className="dashboard-header">
        <div>
          <div className="eyebrow">
            <Activity size={16} aria-hidden="true" />
            Live mock data
          </div>
          <h1>Sensor Dashboard</h1>
          <p>Last updated: {lastUpdated}</p>
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

      <MetricGrid sensors={sensorCards} />

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
            <span className="safe-pill">Safe</span>
          </div>
          <div className="safety-gauge">
            <div>
              <Gauge size={36} aria-hidden="true" />
              <strong>18 ppm</strong>
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
