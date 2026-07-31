import { useState, useEffect, useCallback } from 'react'
import { getHistoryPage, getHistoryCounts } from '../lib/api'
import { icons } from './icons'

const { BarChart3, Download, Filter, History, Loader } = icons

const TABS = [
  { id: 'sensors', label: 'Sensors', icon: 'BarChart3' },
  { id: 'controls', label: 'Controls', icon: 'SlidersHorizontal' },
  { id: 'voice', label: 'Voice', icon: 'Mic' },
  { id: 'alerts', label: 'Alerts', icon: 'BellRing' },
]

const PAGE_SIZE = 20

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
  const [historyPage, setHistoryPage] = useState(null)
  const [counts, setCounts] = useState({ sensors: 0, controls: 0, voice: 0, alerts: 0 })
  const [currentPage, setCurrentPage] = useState(0)
  const [loading, setLoading] = useState(false)

  const fetchPage = useCallback(async (tab, page) => {
    setLoading(true)
    try {
      const data = await getHistoryPage(tab, page, PAGE_SIZE)
      setHistoryPage(data)
    } catch (error) {
      console.error('Failed to fetch history page:', error)
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    getHistoryCounts()
      .then(setCounts)
      .catch((error) => console.error('Failed to fetch history counts:', error))
  }, [])

  useEffect(() => {
    setCurrentPage(0)
  }, [activeTab])

  useEffect(() => {
    fetchPage(activeTab, currentPage)
  }, [activeTab, currentPage, fetchPage])

  const totalPages = historyPage?.totalPages ?? 1
  const totalRecords = historyPage?.totalRecords ?? 0
  const pageText = `Page ${currentPage + 1} of ${totalPages} (${totalRecords} total records)`

  function goToPage(page) {
    const target = Math.max(0, Math.min(page, totalPages - 1))
    setCurrentPage(target)
  }

  const visiblePageNumbers = []
  const start = Math.max(0, currentPage - 2)
  const end = Math.min(totalPages, start + 5)
  for (let i = start; i < end; i++) {
    visiblePageNumbers.push(i)
  }

  return (
    <section className="history-page" aria-label="Data history">
      <header className="history-page-header">
        <div>
          <div className="eyebrow">
            <History size={16} aria-hidden="true" />
            Live database
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
        {TABS.map((tab) => {
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
              <span>{counts[tab.id] ?? 0}</span>
            </button>
          )
        })}
      </div>

      <article className="history-table-card">
        <div className="history-table-title">
          <div>
            <BarChart3 size={20} aria-hidden="true" />
            <h2>{loading ? 'Loading...' : (historyPage?.title ?? 'Data Unavailable')}</h2>
          </div>
          <span>{pageText}</span>
        </div>

        <div className="history-data-scroll">
          {loading ? (
            <div className="history-loading">
              {Loader ? <Loader size={24} className="spin" aria-hidden="true" /> : null}
              <span>Loading data...</span>
            </div>
          ) : (
            <table className="history-data-table">
              <thead>
                <tr>
                  {(historyPage?.headers ?? []).map((header, index) => (
                    <th key={header}>{index === 0 ? `${header} ↓` : header}</th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {(historyPage?.rows ?? []).length === 0 ? (
                  <tr>
                    <td colSpan={(historyPage?.headers ?? []).length || 1} className="history-empty">
                      No records found
                    </td>
                  </tr>
                ) : (
                  (historyPage?.rows ?? []).map((row, rowIndex) => (
                    <tr key={`row-${rowIndex}`}>
                      {row.map((cell, cellIndex) => (
                        <td key={`${cellIndex}-${cell}`}>{renderCell(cell)}</td>
                      ))}
                    </tr>
                  ))
                )}
              </tbody>
            </table>
          )}
        </div>

        <div className="history-pagination">
          <button type="button" disabled={currentPage === 0} onClick={() => goToPage(0)}>
            First
          </button>
          <button type="button" disabled={currentPage === 0} onClick={() => goToPage(currentPage - 1)}>
            Previous
          </button>
          {visiblePageNumbers.map((pageNum) => (
            <button
              className={pageNum === currentPage ? 'active' : ''}
              key={pageNum}
              type="button"
              onClick={() => goToPage(pageNum)}
            >
              {pageNum + 1}
            </button>
          ))}
          <button
            type="button"
            disabled={currentPage >= totalPages - 1}
            onClick={() => goToPage(currentPage + 1)}
          >
            Next
          </button>
          <label>
            Go
            <input
              defaultValue={totalPages}
              aria-label="Go to page"
              onKeyDown={(e) => {
                if (e.key === 'Enter') {
                  const target = parseInt(e.target.value, 10) - 1
                  if (!isNaN(target)) goToPage(target)
                }
              }}
            />
          </label>
        </div>
      </article>
    </section>
  )
}

export default HistoryPage
