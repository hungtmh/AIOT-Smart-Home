import { icons } from './icons'

function MetricGrid({ sensors }) {
  return (
    <section className="sensor-grid" aria-label="Sensor readings">
      {sensors.map((sensor) => {
        const Icon = icons[sensor.icon]
        return (
          <article className="metric-card" data-tone={sensor.tone} key={sensor.label}>
            <div className="metric-icon">
              <Icon size={24} aria-hidden="true" />
            </div>
            <div>
              <span>{sensor.label}</span>
              <strong>
                {sensor.value}
                {sensor.unit && <small> {sensor.unit}</small>}
              </strong>
              <p>{sensor.state}</p>
            </div>
          </article>
        )
      })}
    </section>
  )
}

export default MetricGrid
