import { useRealtime } from '../realtime/RealtimeContext'
import { api } from '../lib/api'
import { icons } from './icons'

const { Flame, AlertTriangle, ShieldCheck, Power } = icons

function AlertPage() {
  const { fireAlert } = useRealtime()

  async function handleTurnOnPump() {
    try {
      await api.post('/api/devices/pump/command', { state: true })
    } catch (error) {
      console.error('Failed to turn on pump:', error)
    }
  }

  return (
    <>
      <header className="dashboard-header">
        <div>
          <div className="eyebrow">
            <AlertTriangle size={16} aria-hidden="true" />
            System Alerts
          </div>
          <h1>Fire Alert Monitor</h1>
        </div>
      </header>

      <section className="status-layout" aria-label="Fire Alert Status">
        {fireAlert ? (
          <article className="panel safety-panel" style={{ borderColor: 'var(--red)', borderWidth: '2px' }}>
            <div className="panel-title">
              <div>
                <Flame size={24} aria-hidden="true" color="var(--red)" />
                <h2 style={{ color: 'var(--red)' }}>FIRE DETECTED</h2>
              </div>
              <span className="safe-pill" style={{ backgroundColor: 'var(--red)', color: 'white' }}>
                DANGER
              </span>
            </div>
            
            <div style={{ margin: '1rem 0', display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '1rem' }}>
              {fireAlert.imageBase64 && (
                <img 
                  src={`data:image/jpeg;base64,${fireAlert.imageBase64}`} 
                  alt="Fire Detection" 
                  style={{ maxWidth: '100%', borderRadius: '8px', border: '1px solid var(--red)' }}
                />
              )}
              
              <div style={{ display: 'flex', flexDirection: 'column', gap: '0.5rem', width: '100%', alignItems: 'center' }}>
                <p style={{ color: 'var(--red)', fontWeight: 'bold' }}>
                  A fire has been detected at {new Date(fireAlert.timestamp).toLocaleString('vi-VN')}
                </p>
                
                <button 
                  className="btn-action" 
                  style={{ backgroundColor: 'var(--red)', color: 'white', padding: '1rem 2rem', fontSize: '1.1rem', marginTop: '1rem', display: 'flex', alignItems: 'center', gap: '0.5rem' }}
                  onClick={handleTurnOnPump}
                >
                  <Power size={20} />
                  Xác nhận bật máy bơm (Turn on Pump)
                </button>
              </div>
            </div>
          </article>
        ) : (
          <article className="panel">
            <div className="panel-title">
              <div>
                <ShieldCheck size={24} aria-hidden="true" color="var(--green)" />
                <h2>System is Safe</h2>
              </div>
              <span className="safe-pill">Safe</span>
            </div>
            <div style={{ padding: '2rem', textAlign: 'center', color: 'var(--text-muted)' }}>
              <p>Hiện không có cảnh báo cháy nổ. (No fire alerts at the moment.)</p>
            </div>
          </article>
        )}
      </section>
    </>
  )
}

export default AlertPage
