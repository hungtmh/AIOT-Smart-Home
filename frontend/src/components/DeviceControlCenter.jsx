import { useEffect, useState } from 'react'
import { initialDevices } from '../data/mockData'
import { commandLed, getLedState } from '../lib/api'
import { icons } from './icons'

const { Wifi } = icons

function getNextMetric(device, nextStatus) {
  if (device.id === 'servo') return nextStatus ? 'OPEN' : 'CLOSE'
  if (device.id === 'buzzer') return nextStatus ? 'ON' : 'OFF'
  return nextStatus ? 'ON' : 'OFF'
}

function applyDeviceStatus(device, nextStatus) {
  return {
    ...device,
    status: nextStatus,
    metric: getNextMetric(device, nextStatus),
  }
}

function DeviceControlCenter() {
  const [devices, setDevices] = useState(initialDevices)
  const [pendingDeviceId, setPendingDeviceId] = useState(null)
  const [ledFeedback, setLedFeedback] = useState('')

  useEffect(() => {
    let cancelled = false

    async function syncLedState() {
      try {
        const ledState = await getLedState()
        if (cancelled) return

        setDevices((currentDevices) =>
          currentDevices.map((device) =>
            device.id === 'led' ? applyDeviceStatus(device, Boolean(ledState.desiredState)) : device,
          ),
        )
        setLedFeedback('')
      } catch {
        if (!cancelled) {
          setLedFeedback('Backend is offline or Supabase schema is not ready.')
        }
      }
    }

    syncLedState()
    const intervalId = window.setInterval(syncLedState, 5000)

    return () => {
      cancelled = true
      window.clearInterval(intervalId)
    }
  }, [])

  async function toggleDevice(deviceId) {
    const selectedDevice = devices.find((device) => device.id === deviceId)
    if (!selectedDevice || pendingDeviceId) return

    const nextStatus = !selectedDevice.status

    if (deviceId === 'led') {
      setPendingDeviceId(deviceId)
      setLedFeedback('')

      try {
        const ledState = await commandLed(nextStatus)
        setDevices((currentDevices) =>
          currentDevices.map((device) =>
            device.id === deviceId ? applyDeviceStatus(device, Boolean(ledState.desiredState)) : device,
          ),
        )
        setLedFeedback(ledState.mqttPublished ? '' : 'Saved to database, but MQTT broker is not connected.')
      } catch {
        setLedFeedback('Could not send LED command to backend.')
      } finally {
        setPendingDeviceId(null)
      }

      return
    }

    setDevices((currentDevices) =>
      currentDevices.map((device) => {
        if (device.id !== deviceId) return device
        return applyDeviceStatus(device, nextStatus)
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
          MQTT via Spring Boot
        </span>
      </div>
      {ledFeedback && <div className="device-feedback">{ledFeedback}</div>}

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
                  disabled={pendingDeviceId === device.id}
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
