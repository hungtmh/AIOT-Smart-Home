import { historyData, historyTabs } from '../data/mockData'
import { icons } from './icons'

const { BarChart3, Download, Filter, History } = icons

function getBadgeClass(value) {
  if (value === 'ON' || value === 'Accepted' || value === 'Success' || value === 'Resolved') return 'table-badge success'
  if (value === 'OFF' || value === 'Pending') return 'table-badge slate'
  if (value === 'Open' || value === 'Close' || value.includes('ppm') || value.includes('%')) return 'table-badge info'
  if (value.includes('warning') || value.includes('fault') || value === 'Buzzer ON') return 'table-badge danger'
  return ''
}

function renderCell(value) {
  const className = getBadgeClass(value)
  if (!className) return value
  return <span className={className}>{value}</span>
}

function HistoryPage({ activeTab, onTabChange }) {
  const currentHistory = historyData[activeTab] ?? historyData.sensors

  return (
    <section className="history-page" aria-label="Data history">
      <header className="history-page-header">
        <div>
          <div className="eyebrow">
            <History size={16} aria-hidden="true" />
            Mock database
          </div>
          <h1>Data History</h1>
        </div>
        <div className="history-actions">
          <button type="button">
            <Filter size={16} aria-hidden="true" />
            Filters
          </button>
          <button type="button">
            <Download size={16} aria-hidden="true" />
            Export JSON
          </button>
          <button type="button">
            <Download size={16} aria-hidden="true" />
            Export CSV
          </button>
        </div>
      </header>

      <div className="history-tabs" role="tablist" aria-label="History categories">
        {historyTabs.map((tab) => {
          const Icon = icons[tab.icon]
          return (
            <button
              className={activeTab === tab.id ? 'active' : ''}
              key={tab.id}
              type="button"
              role="tab"
              aria-selected={activeTab === tab.id}
              onClick={() => onTabChange(tab.id)}
            >
              <Icon size={17} aria-hidden="true" />
              {tab.label}
              <span>{tab.count}</span>
            </button>
          )
        })}
      </div>

      <article className="history-table-card">
        <div className="history-table-title">
          <div>
            <BarChart3 size={20} aria-hidden="true" />
            <h2>{currentHistory.title}</h2>
          </div>
          <span>{currentHistory.pageText}</span>
        </div>

        <div className="history-data-scroll">
          <table className="history-data-table">
            <thead>
              <tr>
                {currentHistory.headers.map((header, index) => (
                  <th key={header}>{index === 0 ? `${header} ↓` : header}</th>
                ))}
              </tr>
            </thead>
            <tbody>
              {currentHistory.rows.map((row) => (
                <tr key={row.join('-')}>
                  {row.map((cell, index) => (
                    <td key={`${cell}-${index}`}>{renderCell(cell)}</td>
                  ))}
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        <div className="history-pagination">
          <button type="button">First</button>
          <button type="button">Previous</button>
          {[1, 2, 3, 4, 5].map((page) => (
            <button className={page === 1 ? 'active' : ''} key={page} type="button">
              {page}
            </button>
          ))}
          <button type="button">Next</button>
          <label>
            Go
            <input defaultValue="95" aria-label="Go to page" />
          </label>
        </div>
      </article>
    </section>
  )
}

export default HistoryPage
