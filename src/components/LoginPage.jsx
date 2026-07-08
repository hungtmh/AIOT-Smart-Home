import { icons } from './icons'

const { Home, KeyRound, Lock, Mic, ShieldCheck, Thermometer, UserRound, Wifi } = icons

function LoginPage({ email, password, onEmailChange, onPasswordChange, onSignIn }) {
  return (
    <main className="login-shell">
      <section className="login-panel" aria-label="Sign in">
        <div className="brand-mark">
          <div className="brand-icon">
            <Home size={28} aria-hidden="true" />
          </div>
          <div>
            <p>AIoT Smart Home</p>
            <span>ESP32-S3 Control Hub</span>
          </div>
        </div>

        <div className="login-copy">
          <h1>Sign in</h1>
          <p>Truy cap bang dieu khien thiet bi, cam bien va canh bao an toan.</p>
        </div>

        <form className="login-form" onSubmit={onSignIn}>
          <label>
            <span>Email</span>
            <div className="input-wrap">
              <UserRound size={18} aria-hidden="true" />
              <input
                value={email}
                onChange={(event) => onEmailChange(event.target.value)}
                type="email"
                autoComplete="email"
                required
              />
            </div>
          </label>
          <label>
            <span>Password</span>
            <div className="input-wrap">
              <Lock size={18} aria-hidden="true" />
              <input
                value={password}
                onChange={(event) => onPasswordChange(event.target.value)}
                type="password"
                autoComplete="current-password"
                required
              />
            </div>
          </label>
          <button className="primary-button" type="submit">
            <KeyRound size={18} aria-hidden="true" />
            Sign in
          </button>
        </form>
      </section>

      <section className="login-preview" aria-label="System preview">
        <div className="preview-topbar">
          <span></span>
          <span></span>
          <span></span>
        </div>
        <div className="preview-grid">
          <div>
            <Thermometer size={22} aria-hidden="true" />
            <p>28.4 C</p>
            <span>Temperature</span>
          </div>
          <div>
            <ShieldCheck size={22} aria-hidden="true" />
            <p>Safe</p>
            <span>Fire monitor</span>
          </div>
          <div>
            <Mic size={22} aria-hidden="true" />
            <p>95%</p>
            <span>Voice accuracy</span>
          </div>
          <div>
            <Wifi size={22} aria-hidden="true" />
            <p>Online</p>
            <span>MQTT</span>
          </div>
        </div>
      </section>
    </main>
  )
}

export default LoginPage
