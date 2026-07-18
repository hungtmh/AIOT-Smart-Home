import { useMemo, useState } from 'react'
import { initialDevices } from '../data/mockData'
import { commandDevice, getAuthDebug } from '../lib/api'
import { useRealtime } from '../realtime/RealtimeContext'
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
  const { connectionState, deviceStates, error: realtimeError, updateDeviceState } = useRealtime()
  const [pendingDeviceId, setPendingDeviceId] = useState(null)
  const [deviceFeedback, setDeviceFeedback] = useState('')
  const devices = useMemo(() => {
    const stateByDeviceId = new Map(deviceStates.map((deviceState) => [deviceState.deviceId, deviceState]))
    return initialDevices.map((device) => {
      const deviceState = stateByDeviceId.get(device.id)
      return deviceState ? applyDeviceStatus(device, Boolean(deviceState.desiredState)) : device
    })
  }, [deviceStates])
  const realtimeLabel =
    connectionState === 'connected'
      ? 'Realtime connected'
      : connectionState === 'error'
        ? 'Realtime unavailable'
        : 'Realtime reconnecting'

  async function toggleDevice(deviceId) {
    const selectedDevice = devices.find((device) => device.id === deviceId)
    if (!selectedDevice || pendingDeviceId) return

    const nextStatus = !selectedDevice.status
    setPendingDeviceId(deviceId)
    setDeviceFeedback('')

    try {
      const deviceState = await commandDevice(deviceId, nextStatus)
      updateDeviceState(deviceState)
      setDeviceFeedback(deviceState.mqttPublished ? '' : 'Saved to database, but MQTT broker is not connected.')
    } catch (error) {
      if (error.status === 401 || error.status === 403) {
        const authDebug = await getAuthDebug()
        const alg = authDebug.header?.alg ?? 'none'
        const kid = authDebug.header?.kid ?? 'none'
        const subject = authDebug.payload?.sub ? `${authDebug.payload.sub}`.slice(0, 8) : 'none'
        setDeviceFeedback(
          `JWT rejected by backend. session=${authDebug.hasSession} token=${authDebug.hasToken} alg=${alg} kid=${kid} sub=${subject}`,
        )
      } else {
        setDeviceFeedback(`Could not send ${selectedDevice.name} command to backend.`)
      }
    } finally {
      setPendingDeviceId(null)
    }
  }

  return (
    <section className="device-section" aria-label="Device control center">
      <div className="section-heading">
        <div>
          <h2>Device Control Center</h2>
          <p>Manual controls for relay, servo, buzzer and pump outputs</p>
        </div>
        <span>
          <Wifi size={16} aria-hidden="true" />
          {realtimeLabel}
        </span>
      </div>
      {(deviceFeedback || realtimeError) && <div className="device-feedback">{deviceFeedback || realtimeError}</div>}

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
