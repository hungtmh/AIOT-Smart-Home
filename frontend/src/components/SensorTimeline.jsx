import { useState, useEffect, useMemo } from 'react'
import { getTelemetryHistory } from '../lib/api'
import { useRealtime } from '../realtime/RealtimeContext'
import { icons } from './icons'

const { BarChart3, RefreshCw } = icons

// Helper for calculating y-coordinate dynamically
function getMappedY(valStr, min, max) {
  const v = parseFloat(valStr)
  if (isNaN(v)) return 240 // Fallback to bottom if invalid
  const fraction = Math.max(0, Math.min(1, (v - min) / (max - min)))
  return 240 - fraction * 180 // Bottom is 240, top is 60 (range = 180)
}

function SensorTimeline() {
  const [hoveredPoint, setHoveredPoint] = useState(null)
  const [visibleSeries, setVisibleSeries] = useState({
    temperature: true,
    humidity: true,
    smoke: true,
  })

  function toggleSeries(seriesKey) {
    setVisibleSeries((current) => ({
      ...current,
      [seriesKey]: !current[seriesKey],
    }))
  }

  const [telemetryList, setTelemetryList] = useState([])
  const { telemetry: latestTelemetry } = useRealtime()

  async function fetchHistory() {
    try {
      const data = await getTelemetryHistory(10)
      setTelemetryList(data)
    } catch (e) {
      console.error(e)
    }
  }

  useEffect(() => {
    fetchHistory()
  }, [])

  useEffect(() => {
    if (latestTelemetry) {
      setTelemetryList(prev => {
        // Only append if it's a new measurement
        if (prev.length > 0 && prev[0].measuredAt === latestTelemetry.measuredAt) {
          return prev
        }
        const next = [latestTelemetry, ...prev]
        return next.slice(0, 10)
      })
    }
  }, [latestTelemetry])

  const { derivedChartSeries, derivedTimelineRows } = useMemo(() => {
    const list = [...telemetryList].reverse()
    const pointsX = [70, 160, 250, 340, 430, 520, 610, 700, 790, 900]
    
    const fmtTime = (ts) => {
      if (!ts) return ''
      const d = new Date(ts)
      return `${d.getHours().toString().padStart(2, '0')}:${d.getMinutes().toString().padStart(2, '0')}:${d.getSeconds().toString().padStart(2, '0')}`
    }

    const tPoints = []
    const hPoints = []
    const sPoints = []

    for (let i = 0; i < list.length; i++) {
      const item = list[i]
      const x = pointsX[i] || (70 + i * 90)
      const timeStr = fmtTime(item.measuredAt)
      tPoints.push({ x, value: item.temperature?.toString(), time: timeStr })
      hPoints.push({ x, value: item.humidity?.toString(), time: timeStr })
      sPoints.push({ x, value: item.smokePpm?.toString(), time: timeStr })
    }

    const rows = telemetryList.map(item => ({
      time: fmtTime(item.measuredAt),
      temperature: `${item.temperature} C`,
      humidity: `${item.humidity}%`,
      smoke: `${item.smokePpm} ppm`
    }))

    const cSeries = [
      { key: 'temperature', label: 'Temperature', unit: 'C', color: '#dc2626', points: tPoints },
      { key: 'humidity', label: 'Humidity', unit: '%', color: '#2563eb', points: hPoints },
      { key: 'smoke', label: 'Smoke', unit: 'ppm', color: '#64748b', points: sPoints },
    ]

    return { derivedChartSeries: cSeries, derivedTimelineRows: rows }
  }, [telemetryList])

  // Filter series for specific charts
  const envSeries = derivedChartSeries.filter(s => (s.key === 'temperature' || s.key === 'humidity') && visibleSeries[s.key])
  const smokeSeries = derivedChartSeries.filter(s => s.key === 'smoke' && visibleSeries[s.key])

  // Get latest values for legend
  const currentTemp = telemetryList.length > 0 ? telemetryList[0].temperature : (latestTelemetry?.temperature ?? 0)
  const currentHum = telemetryList.length > 0 ? telemetryList[0].humidity : (latestTelemetry?.humidity ?? 0)
  const currentSmoke = telemetryList.length > 0 ? telemetryList[0].smokePpm : (latestTelemetry?.smokePpm ?? 0)

  const dynamicLegend = [
    { key: 'temperature', label: 'Temperature', value: `${currentTemp} C`, color: '#dc2626' },
    { key: 'humidity', label: 'Humidity', value: `${currentHum}%`, color: '#2563eb' },
    { key: 'smoke', label: 'Smoke', value: `${currentSmoke} ppm`, color: '#64748b' },
  ]

  return (
    <section className="device-section" aria-label="Sensor data timeline">
      <div className="section-heading">
        <div>
          <h2>Sensor Data Timeline</h2>
          <p>Real-time records from PostgreSQL</p>
        </div>
        <button className="refresh-button" type="button" onClick={fetchHistory}>
          <RefreshCw size={16} aria-hidden="true" />
          Refresh
        </button>
      </div>

      <div className="timeline-panel">
        <div className="timeline-toolbar">
          {dynamicLegend.map((item) => (
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

        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(400px, 1fr))', gap: '16px', padding: '16px' }}>
          {/* Chart 1: Environment (Temp/Humidity) */}
          <div className="chart-card" style={{ padding: 0, border: 'none' }}>
            <div className="chart-title">
              <BarChart3 size={18} aria-hidden="true" />
              Environment (Temperature & Humidity)
            </div>
            <svg className="timeline-chart" viewBox="0 0 960 300" role="img" aria-label="Environment chart">
              <g className="grid-lines">
                {[60, 105, 150, 195, 240].map((y) => (
                  <line key={`h-${y}`} x1="58" x2="925" y1={y} y2={y} />
                ))}
                {[140, 260, 380, 500, 620, 740, 860].map((x) => (
                  <line key={`v-${x}`} x1={x} x2={x} y1="45" y2="255" />
                ))}
              </g>
              <g className="axis-labels">
                {/* Temp Axis (Left) */}
                <text x="28" y="63" fill="#dc2626">80</text>
                <text x="28" y="108" fill="#dc2626">62.5</text>
                <text x="28" y="153" fill="#dc2626">45</text>
                <text x="28" y="198" fill="#dc2626">27.5</text>
                <text x="28" y="243" fill="#dc2626">10</text>
                
                {/* Humidity Axis (Right) */}
                <text x="930" y="63" fill="#2563eb">100</text>
                <text x="930" y="108" fill="#2563eb">75</text>
                <text x="930" y="153" fill="#2563eb">50</text>
                <text x="930" y="198" fill="#2563eb">25</text>
                <text x="930" y="243" fill="#2563eb">0</text>
                
                <text x="440" y="292">Time</text>
              </g>
              {envSeries.map((series) => {
                const isTemp = series.key === 'temperature'
                const min = isTemp ? 10 : 0
                const max = isTemp ? 80 : 100
                const pointsStr = series.points.map(p => `${p.x},${getMappedY(p.value, min, max)}`).join(' ')
                
                return (
                  <g key={series.key}>
                    <polyline className="line" points={pointsStr} style={{ stroke: series.color }} />
                    {series.points.map((point) => (
                      <circle
                        className="point"
                        cx={point.x}
                        cy={getMappedY(point.value, min, max)}
                        fill={series.color}
                        key={`${series.key}-${point.time}`}
                        r="5"
                        onMouseEnter={() =>
                          setHoveredPoint({
                            ...point,
                            color: series.color,
                            label: series.label,
                            unit: series.unit,
                            mappedY: getMappedY(point.value, min, max)
                          })
                        }
                        onMouseLeave={() => setHoveredPoint(null)}
                      />
                    ))}
                  </g>
                )
              })}
              {hoveredPoint && (hoveredPoint.label === 'Temperature' || hoveredPoint.label === 'Humidity') && (
                <g className="chart-tooltip" transform={`translate(${Math.min(hoveredPoint.x + 14, 770)} ${Math.max(hoveredPoint.mappedY - 62, 30)})`}>
                  <rect width="172" height="58" rx="8" />
                  <circle cx="15" cy="20" r="5" fill={hoveredPoint.color} />
                  <text x="28" y="24">{hoveredPoint.label}: {hoveredPoint.value}{hoveredPoint.unit ? ` ${hoveredPoint.unit}` : ''}</text>
                  <text x="15" y="44">Time: {hoveredPoint.time}</text>
                </g>
              )}
            </svg>
          </div>

          {/* Chart 2: Smoke Level */}
          <div className="chart-card" style={{ padding: 0, border: 'none' }}>
            <div className="chart-title">
              <BarChart3 size={18} aria-hidden="true" />
              Safety (Smoke Level)
            </div>
            <svg className="timeline-chart" viewBox="0 0 960 300" role="img" aria-label="Smoke chart">
              <g className="grid-lines">
                {[60, 105, 150, 195, 240].map((y) => (
                  <line key={`h-${y}`} x1="58" x2="925" y1={y} y2={y} />
                ))}
                {[140, 260, 380, 500, 620, 740, 860].map((x) => (
                  <line key={`v-${x}`} x1={x} x2={x} y1="45" y2="255" />
                ))}
              </g>
              <g className="axis-labels">
                {/* Smoke Axis (Left) */}
                <text x="28" y="63" fill="#64748b">100</text>
                <text x="28" y="108" fill="#64748b">75</text>
                <text x="28" y="153" fill="#64748b">50</text>
                <text x="28" y="198" fill="#64748b">25</text>
                <text x="28" y="243" fill="#64748b">0</text>
                
                <text x="440" y="292">Time</text>
              </g>
              {smokeSeries.map((series) => {
                const min = 0
                const max = 100
                const pointsStr = series.points.map(p => `${p.x},${getMappedY(p.value, min, max)}`).join(' ')
                
                return (
                  <g key={series.key}>
                    <polyline className="line" points={pointsStr} style={{ stroke: series.color }} />
                    {series.points.map((point) => (
                      <circle
                        className="point"
                        cx={point.x}
                        cy={getMappedY(point.value, min, max)}
                        fill={series.color}
                        key={`${series.key}-${point.time}`}
                        r="5"
                        onMouseEnter={() =>
                          setHoveredPoint({
                            ...point,
                            color: series.color,
                            label: series.label,
                            unit: series.unit,
                            mappedY: getMappedY(point.value, min, max)
                          })
                        }
                        onMouseLeave={() => setHoveredPoint(null)}
                      />
                    ))}
                  </g>
                )
              })}
              {hoveredPoint && hoveredPoint.label === 'Smoke' && (
                <g className="chart-tooltip" transform={`translate(${Math.min(hoveredPoint.x + 14, 770)} ${Math.max(hoveredPoint.mappedY - 62, 30)})`}>
                  <rect width="172" height="58" rx="8" />
                  <circle cx="15" cy="20" r="5" fill={hoveredPoint.color} />
                  <text x="28" y="24">{hoveredPoint.label}: {hoveredPoint.value}{hoveredPoint.unit ? ` ${hoveredPoint.unit}` : ''}</text>
                  <text x="15" y="44">Time: {hoveredPoint.time}</text>
                </g>
              )}
            </svg>
          </div>
        </div>

        <div className="mock-table-wrap">
          <table className="mock-table">
            <thead>
              <tr>
                <th>Time</th>
                <th>Temperature</th>
                <th>Humidity</th>
                <th>Smoke</th>
              </tr>
            </thead>
            <tbody>
              {derivedTimelineRows.map((row) => (
                <tr key={row.time}>
                  <td>{row.time}</td>
                  <td>{row.temperature}</td>
                  <td>{row.humidity}</td>
                  <td>{row.smoke}</td>
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
