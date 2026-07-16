const API_BASE_URL = import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:8080'

import { supabase } from './supabase'

function parseJwtPart(token, index) {
  try {
    const base64Url = token.split('.')[index]
    const base64 = base64Url.replaceAll('-', '+').replaceAll('_', '/')
    return JSON.parse(window.atob(base64))
  } catch {
    return null
  }
}

export async function getAuthDebug() {
  const { data } = await supabase.auth.getSession()
  const token = data.session?.access_token

  return {
    hasSession: Boolean(data.session),
    hasToken: Boolean(token),
    email: data.session?.user?.email ?? '',
    header: token ? parseJwtPart(token, 0) : null,
    payload: token ? parseJwtPart(token, 1) : null,
  }
}

async function request(path, options = {}) {
  const { data } = await supabase.auth.getSession()
  const token = data.session?.access_token

  const response = await fetch(`${API_BASE_URL}${path}`, {
    headers: {
      'Content-Type': 'application/json',
      ...(token ? { Authorization: `Bearer ${token}` } : {}),
      ...options.headers,
    },
    ...options,
  })

  if (!response.ok) {
    let body = null
    try {
      body = await response.json()
    } catch {
      body = null
    }

    const error = new Error(`API request failed with status ${response.status}`)
    error.status = response.status
    error.body = body
    throw error
  }

  return response.json()
}

export function getLedState() {
  return request('/api/devices/led')
}

export function commandLed(state) {
  return request('/api/devices/led/command', {
    method: 'POST',
    body: JSON.stringify({ state }),
  })
}
