import { navItems } from '../data/mockData'
import { icons } from './icons'

const { Home, LogOut, UserRound } = icons

function Sidebar({ activeNav, email, onNavigate, onSignOut }) {
  return (
    <aside className="sidebar">
      <div className="sidebar-brand">
        <div className="brand-icon compact">
          <Home size={23} aria-hidden="true" />
        </div>
        <div>
          <strong>AIoT</strong>
          <span>Smart Home</span>
        </div>
      </div>

      <nav aria-label="Main navigation">
        {navItems.map((item) => {
          const Icon = icons[item.icon]
          return (
            <button
              className={activeNav === item.id ? 'active' : ''}
              key={item.id}
              type="button"
              onClick={() => onNavigate(item.id)}
            >
              <Icon size={19} aria-hidden="true" />
              {item.label}
            </button>
          )
        })}
      </nav>

      <div className="sidebar-footer">
        <div className="account-chip">
          <UserRound size={18} aria-hidden="true" />
          <span>{email}</span>
        </div>
        <button type="button" onClick={onSignOut}>
          <LogOut size={18} aria-hidden="true" />
          Sign out
        </button>
      </div>
    </aside>
  )
}

export default Sidebar
