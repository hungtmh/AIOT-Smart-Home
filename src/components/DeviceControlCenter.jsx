import { useState } from 'react'
import { initialDevices } from '../data/mockData'
import { icons } from './icons'

const { Wifi } = icons

function getNextMetric(device, nextStatus) {
  if (device.id === 'servo') return nextStatus ? 'OPEN' : 'CLOSE'
  if (device.id === 'buzzer') return nextStatus ? 'ON' : 'OFF'
  return nextStatus ? 'ON' : 'OFF'
}

function DeviceControlCenter() {
  const [devices, setDevices] = useState(initialDevices)

  function toggleDevice(deviceId) {
    setDevices((currentDevices) =>
      currentDevices.map((device) => {
        if (device.id !== deviceId) return device
        const nextStatus = !device.status
        return {
          ...device,
          status: nextStatus,
          metric: getNextMetric(device, nextStatus),
        }
      }),
    )
  }

  return (
    <section className="device-section" aria-label="Device control center">
      <div className="section-heading">
        <div>
          <h2>Device Control Center</h2>
          <p>Manual mock controls for relay, servo and buzzer outputs</p>
        </div>
        <span>
          <Wifi size={16} aria-hidden="true" />
          MQTT connected
        </span>
      </div>

      <div className="device-grid">
        {devices.map((device) => {
          const Icon = icons[device.icon]
          return (
            <article className="device-card" data-tone={device.tone} key={device.id}>
              <div className="device-top">
                <div className="device-icon">
                  <Icon size={24} aria-hidden="true" />
                </div>
                <button
                  className={`switch ${device.status ? 'on' : ''}`}
                  type="button"
                  aria-label={`Toggle ${device.name}`}
                  aria-pressed={device.status}
                  onClick={() => toggleDevice(device.id)}
                >
                  <span></span>
                </button>
              </div>
              <h3>{device.name}</h3>
              <p>{device.description}</p>
              <div className="device-state">
                <span className={device.status ? 'dot online' : 'dot'}></span>
                <strong>{device.metric}</strong>
              </div>
            </article>
          )
        })}
      </div>
    </section>
  )
}

export default DeviceControlCenter
