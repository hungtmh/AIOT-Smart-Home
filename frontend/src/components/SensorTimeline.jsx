import { useState } from 'react'
import { chartSeries, seriesLegend, timelineRows } from '../data/mockData'
import { icons } from './icons'

const { BarChart3, RefreshCw } = icons

function SensorTimeline() {
  const [hoveredPoint, setHoveredPoint] = useState(null)
  const [visibleSeries, setVisibleSeries] = useState({
    temperature: true,
    humidity: true,
    smoke: true,
    led: false,
    servo: false,
    buzzer: false,
  })

  function toggleSeries(seriesKey) {
    setVisibleSeries((current) => ({
      ...current,
      [seriesKey]: !current[seriesKey],
    }))
  }

  return (
    <section className="device-section" aria-label="Sensor data timeline">
      <div className="section-heading">
        <div>
          <h2>Sensor Data Timeline</h2>
          <p>Mock records in the latest monitoring window</p>
        </div>
        <button className="refresh-button" type="button">
          <RefreshCw size={16} aria-hidden="true" />
          Refresh
        </button>
      </div>

      <div className="timeline-panel">
        <div className="timeline-toolbar">
          {seriesLegend.map((item) => (
            <label className="series-toggle" key={item.label}>
              <input
                type="checkbox"
                checked={Boolean(visibleSeries[item.key])}
                onChange={() => toggleSeries(item.key)}
              />
              <span style={{ '--series-color': item.color }}></span>
              {item.label}
              <strong>{item.value}</strong>
            </label>
          ))}
        </div>

        <div className="chart-card">
          <div className="chart-title">
            <BarChart3 size={18} aria-hidden="true" />
            Sensor Data Timeline
          </div>
          <svg className="timeline-chart" viewBox="0 0 960 300" role="img" aria-label="Mock sensor timeline chart">
            <g className="grid-lines">
              {[60, 105, 150, 195, 240].map((y) => (
                <line key={`h-${y}`} x1="58" x2="925" y1={y} y2={y} />
              ))}
              {[140, 260, 380, 500, 620, 740, 860].map((x) => (
                <line key={`v-${x}`} x1={x} x2={x} y1="45" y2="255" />
              ))}
            </g>
            <g className="axis-labels">
              <text x="28" y="63">70</text>
              <text x="28" y="108">55</text>
              <text x="28" y="153">40</text>
              <text x="28" y="198">25</text>
              <text x="34" y="243">10</text>
              <text x="440" y="292">Time</text>
            </g>
            {chartSeries
              .filter((series) => visibleSeries[series.key])
              .map((series) => (
                <g key={series.key}>
                  <polyline
                    className="line"
                    points={series.points.map((point) => `${point.x},${point.y}`).join(' ')}
                    style={{ stroke: series.color }}
                  />
                  {series.points.map((point) => (
                    <circle
                      className="point"
                      cx={point.x}
                      cy={point.y}
                      fill={series.color}
                      key={`${series.key}-${point.time}`}
                      r="5"
                      onMouseEnter={() =>
                        setHoveredPoint({
                          ...point,
                          color: series.color,
                          label: series.label,
                          unit: series.unit,
                        })
                      }
                      onMouseLeave={() => setHoveredPoint(null)}
                    />
                  ))}
                </g>
              ))}
            {hoveredPoint && (
              <g
                className="chart-tooltip"
                transform={`translate(${Math.min(hoveredPoint.x + 14, 770)} ${Math.max(hoveredPoint.y - 62, 30)})`}
              >
                <rect width="172" height="58" rx="8" />
                <circle cx="15" cy="20" r="5" fill={hoveredPoint.color} />
                <text x="28" y="24">
                  {hoveredPoint.label}: {hoveredPoint.value}
                  {hoveredPoint.unit ? ` ${hoveredPoint.unit}` : ''}
                </text>
                <text x="15" y="44">Time: {hoveredPoint.time}</text>
              </g>
            )}
          </svg>
        </div>

        <div className="mock-table-wrap">
          <table className="mock-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>Temperature</th>
                <th>Humidity</th>
                <th>Smoke</th>
                <th>LED</th>
                <th>Servo</th>
                <th>Buzzer</th>
              </tr>
            </thead>
            <tbody>
              {timelineRows.map((row) => (
                <tr key={row.time}>
                  <td>{row.time}</td>
                  <td>{row.temperature}</td>
                  <td>{row.humidity}</td>
                  <td>{row.smoke}</td>
                  <td>{row.led}</td>
                  <td>{row.servo}</td>
                  <td>{row.buzzer}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>
    </section>
  )
}

export default SensorTimeline
