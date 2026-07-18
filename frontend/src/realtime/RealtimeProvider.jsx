import { useCallback, useEffect, useMemo, useState } from 'react'
import { API_BASE_URL } from '../lib/api'
import { RealtimeContext } from './RealtimeContext'

function realtimeUrl() {
  const url = new URL('/ws/realtime', API_BASE_URL)
  url.protocol = url.protocol === 'https:' ? 'wss:' : 'ws:'
  return url.toString()
}

function timestamp(value) {
  const parsed = Date.parse(value ?? '')
  return Number.isNaN(parsed) ? 0 : parsed
}

function mergeDeviceState(states, nextState) {
  const currentState = states.find((state) => state.deviceId === nextState.deviceId)
  if (!currentState) return [...states, nextState]

  const desiredIsNewer = timestamp(nextState.updatedAt) >= timestamp(currentState.updatedAt)
  const reportedIsNewer = timestamp(nextState.reportedAt) >= timestamp(currentState.reportedAt)
  const mergedState = {
    ...currentState,
    ...nextState,
    desiredState: desiredIsNewer ? nextState.desiredState : currentState.desiredState,
    updatedAt: desiredIsNewer ? nextState.updatedAt : currentState.updatedAt,
    reportedState: reportedIsNewer ? nextState.reportedState : currentState.reportedState,
    reportedAt: reportedIsNewer ? nextState.reportedAt : currentState.reportedAt,
  }

  return states.map((state) => (state.deviceId === nextState.deviceId ? mergedState : state))
}

export function RealtimeProvider({ session, children }) {
  const [deviceStates, setDeviceStates] = useState([])
  const [telemetry, setTelemetry] = useState(null)
  const [connectionState, setConnectionState] = useState('connecting')
  const [error, setError] = useState('')

  const updateDeviceState = useCallback((nextState) => {
    setDeviceStates((currentStates) => mergeDeviceState(currentStates, nextState))
  }, [])

  useEffect(() => {
    const token = session?.access_token
    if (!token) return undefined

    let stopped = false
    let allowReconnect = true
    let socket = null
    let reconnectTimer = null
    let reconnectAttempt = 0

    function scheduleReconnect() {
      if (stopped || !allowReconnect || reconnectTimer) return

      const delay = Math.min(1000 * 2 ** reconnectAttempt, 10_000)
      reconnectAttempt += 1
      setConnectionState('reconnecting')
      reconnectTimer = window.setTimeout(() => {
        reconnectTimer = null
        connect()
      }, delay)
    }

    function connect() {
      if (stopped) return

      setConnectionState(reconnectAttempt === 0 ? 'connecting' : 'reconnecting')
      socket = new WebSocket(realtimeUrl())

      socket.addEventListener('open', () => {
        socket.send(JSON.stringify({ type: 'AUTH', token }))
      })

      socket.addEventListener('message', (event) => {
        let message
        try {
          message = JSON.parse(event.data)
        } catch {
          return
        }

        if (message.type === 'AUTHENTICATED') {
          reconnectAttempt = 0
          setConnectionState('connected')
          setError('')
        } else if (message.type === 'DEVICE_SNAPSHOT' && Array.isArray(message.data)) {
          setDeviceStates((currentStates) =>
            message.data.reduce((states, deviceState) => mergeDeviceState(states, deviceState), currentStates),
          )
        } else if (message.type === 'DEVICE_STATE' && message.data) {
          updateDeviceState(message.data)
        } else if (message.type === 'TELEMETRY' && message.data) {
          setTelemetry(message.data)
        } else if (message.type === 'AUTH_ERROR') {
          allowReconnect = false
          setConnectionState('error')
          setError(message.message || 'Realtime JWT was rejected by the backend.')
        }
      })

      socket.addEventListener('close', () => {
        socket = null
        scheduleReconnect()
      })

      socket.addEventListener('error', () => {
        setError('Realtime connection was interrupted.')
        if (socket && socket.readyState < WebSocket.CLOSING) socket.close()
      })
    }

    connect()

    return () => {
      stopped = true
      if (reconnectTimer) window.clearTimeout(reconnectTimer)
      if (socket && socket.readyState < WebSocket.CLOSING) socket.close(1000, 'Client closed')
    }
  }, [session?.access_token, updateDeviceState])

  const value = useMemo(
    () => ({ deviceStates, telemetry, connectionState, error, updateDeviceState }),
    [connectionState, deviceStates, error, telemetry, updateDeviceState],
  )

  return <RealtimeContext.Provider value={value}>{children}</RealtimeContext.Provider>
}
