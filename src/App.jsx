import { useState } from 'react'
import './App.css'
import AutomationPage from './components/AutomationPage'
import DashboardPage from './components/DashboardPage'
import HistoryPage from './components/HistoryPage'
import { icons } from './components/icons'
import LoginPage from './components/LoginPage'
import SettingsPage from './components/SettingsPage'
import Sidebar from './components/Sidebar'

const { ShieldCheck, X } = icons

function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false)
  const [email, setEmail] = useState('admin@gmail.com')
  const [password, setPassword] = useState('123456')
  const [activeNav, setActiveNav] = useState('dashboard')
  const [activeHistoryTab, setActiveHistoryTab] = useState('sensors')
  const [autoMode, setAutoMode] = useState(true)
  const [toastVisible, setToastVisible] = useState(false)

  const lastUpdated = new Intl.DateTimeFormat('vi-VN', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
  }).format(new Date())

  function handleSignIn(event) {
    event.preventDefault()
    setIsAuthenticated(true)
    setToastVisible(true)
  }

  function handleSignOut() {
    setIsAuthenticated(false)
    setToastVisible(false)
    setActiveNav('dashboard')
  }

  if (!isAuthenticated) {
    return (
      <LoginPage
        email={email}
        password={password}
        onEmailChange={setEmail}
        onPasswordChange={setPassword}
        onSignIn={handleSignIn}
      />
    )
  }

  return (
    <div className="app-shell">
      <Sidebar activeNav={activeNav} email={email} onNavigate={setActiveNav} onSignOut={handleSignOut} />

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
