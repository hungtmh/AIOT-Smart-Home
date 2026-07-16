import { settingsGroups } from '../data/mockData'
import { icons } from './icons'

const { Save, Settings, ShieldCheck } = icons

function SettingsPage({ email }) {
  return (
    <section className="config-page" aria-label="System settings">
      <header className="history-page-header">
        <div>
          <div className="eyebrow">
            <Settings size={16} aria-hidden="true" />
            System configuration
          </div>
          <h1>Settings</h1>
          <p className="page-subtitle">Cau hinh mock cho MQTT, ESP32-S3, bao mat va luu tru du lieu.</p>
        </div>
        <button className="save-button" type="button">
          <Save size={17} aria-hidden="true" />
          Save settings
        </button>
      </header>

      <section className="settings-grid">
        {settingsGroups.map((group) => {
          const Icon = icons[group.icon]
          return (
            <article className="settings-card" key={group.title}>
              <div className="settings-card-title">
                <div className="rule-icon">
                  <Icon size={22} aria-hidden="true" />
                </div>
                <h2>{group.title}</h2>
              </div>
              <div className="settings-fields">
                {group.fields.map((field) => (
                  <label key={field.label}>
                    <span>{field.label}</span>
                    <input value={field.value} readOnly />
                  </label>
                ))}
              </div>
            </article>
          )
        })}
      </section>

      <section className="settings-grid compact">
        <article className="settings-card">
          <div className="settings-card-title">
            <div className="rule-icon">
              <ShieldCheck size={22} aria-hidden="true" />
            </div>
            <h2>Account</h2>
          </div>
          <div className="settings-fields">
            <label>
              <span>Email</span>
              <input value={email} readOnly />
            </label>
            <label>
              <span>Role</span>
              <input value="Administrator" readOnly />
            </label>
          </div>
        </article>

        <article className="settings-card">
          <div className="settings-card-title">
            <div className="rule-icon">
              <Settings size={22} aria-hidden="true" />
            </div>
            <h2>Dashboard Preferences</h2>
          </div>
          <div className="settings-toggles">
            <label>
              <input type="checkbox" defaultChecked />
              Show sign-in notifications
            </label>
            <label>
              <input type="checkbox" defaultChecked />
              Highlight fire safety cards
            </label>
            <label>
              <input type="checkbox" />
              Developer debug mode
            </label>
          </div>
        </article>
      </section>
    </section>
  )
}

export default SettingsPage
