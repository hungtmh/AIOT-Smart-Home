import { useEffect, useState } from 'react'
import './App.css'
import AutomationPage from './components/AutomationPage'
import DashboardPage from './components/DashboardPage'
import HistoryPage from './components/HistoryPage'
import { icons } from './components/icons'
import LoginPage from './components/LoginPage'
import SettingsPage from './components/SettingsPage'
import Sidebar from './components/Sidebar'
import { isSupabaseConfigured, supabase } from './lib/supabase'

const { ShieldCheck, X } = icons

function App() {
  const [session, setSession] = useState(null)
  const [email, setEmail] = useState('')
  const [password, setPassword] = useState('')
  const [activeNav, setActiveNav] = useState('dashboard')
  const [activeHistoryTab, setActiveHistoryTab] = useState('sensors')
  const [autoMode, setAutoMode] = useState(true)
  const [toastVisible, setToastVisible] = useState(false)
  const [isAuthLoading, setIsAuthLoading] = useState(true)
  const [authError, setAuthError] = useState('')

  useEffect(() => {
    let mounted = true

    supabase.auth.getSession().then(({ data }) => {
      if (!mounted) return
      setSession(data.session)
      setEmail(data.session?.user?.email ?? '')
      setIsAuthLoading(false)
    })

    const { data: authListener } = supabase.auth.onAuthStateChange((_event, nextSession) => {
      setSession(nextSession)
      setEmail(nextSession?.user?.email ?? '')
    })

    return () => {
      mounted = false
      authListener.subscription.unsubscribe()
    }
  }, [])

  const lastUpdated = new Intl.DateTimeFormat('vi-VN', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
  }).format(new Date())

  async function handleSignIn(event) {
    event.preventDefault()

    if (!isSupabaseConfigured) {
      setAuthError('Missing VITE_SUPABASE_URL or VITE_SUPABASE_ANON_KEY in frontend/.env')
      return
    }

    setIsAuthLoading(true)
    setAuthError('')

    const { data, error } = await supabase.auth.signInWithPassword({
      email,
      password,
    })

    setIsAuthLoading(false)

    if (error) {
      setAuthError('Email hoac mat khau khong dung.')
      return
    }

    setSession(data.session)
    setToastVisible(true)
  }

  async function handleSignOut() {
    await supabase.auth.signOut()
    setSession(null)
    setToastVisible(false)
    setActiveNav('dashboard')
    setPassword('')
  }

  if (isAuthLoading && !session) {
    return <div className="app-loading">Checking session...</div>
  }

  if (!session) {
    return (
      <LoginPage
        email={email}
        password={password}
        error={authError}
        isLoading={isAuthLoading}
        onEmailChange={setEmail}
        onPasswordChange={setPassword}
        onSignIn={handleSignIn}
      />
    )
  }

  return (
    <div className="app-shell">
      <Sidebar activeNav={activeNav} email={session.user.email} onNavigate={setActiveNav} onSignOut={handleSignOut} />

      <main className="dashboard">
        {toastVisible && (
          <div className="toast" role="status">
            <ShieldCheck size={18} aria-hidden="true" />
            <div>
              <strong>System Notification</strong>
              <span>Successfully signed in</span>
            </div>
            <button type="button" aria-label="Close notification" onClick={() => setToastVisible(false)}>
              <X size={17} aria-hidden="true" />
            </button>
          </div>
        )}

        {activeNav === 'history' && <HistoryPage activeTab={activeHistoryTab} onTabChange={setActiveHistoryTab} />}
        {activeNav === 'automation' && <AutomationPage />}
        {activeNav === 'settings' && <SettingsPage email={email} />}
        {activeNav === 'dashboard' && (
          <DashboardPage
            autoMode={autoMode}
            lastUpdated={lastUpdated}
            onToggleAutoMode={() => setAutoMode((current) => !current)}
          />
        )}
      </main>
    </div>
  )
}

export default App
