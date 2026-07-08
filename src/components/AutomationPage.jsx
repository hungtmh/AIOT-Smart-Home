import { useState } from 'react'
import { automationRules, automationSummary } from '../data/mockData'
import { icons } from './icons'

const { Save, Settings, ShieldCheck, SlidersHorizontal } = icons

function AutomationPage() {
  const [rules, setRules] = useState(automationRules)

  function toggleRule(ruleId) {
    setRules((currentRules) =>
      currentRules.map((rule) => (rule.id === ruleId ? { ...rule, enabled: !rule.enabled } : rule)),
    )
  }

  function updateThreshold(ruleId, threshold) {
    setRules((currentRules) =>
      currentRules.map((rule) => (rule.id === ruleId ? { ...rule, threshold } : rule)),
    )
  }

  return (
    <section className="config-page" aria-label="Automation settings">
      <header className="history-page-header">
        <div>
          <div className="eyebrow">
            <SlidersHorizontal size={16} aria-hidden="true" />
            Automatic control
          </div>
          <h1>Automation</h1>
          <p className="page-subtitle">Cai dat nguong cam bien de he thong tu dong dieu khien LED, servo va buzzer.</p>
        </div>
        <button className="save-button" type="button">
          <Save size={17} aria-hidden="true" />
          Save mock config
        </button>
      </header>

      <div className="automation-summary">
        {automationSummary.map((item) => {
          const Icon = icons[item.icon]
          return (
            <article className="metric-card" data-tone={item.tone} key={item.label}>
              <div className="metric-icon">
                <Icon size={24} aria-hidden="true" />
              </div>
              <div>
                <span>{item.label}</span>
                <strong>{item.value}</strong>
                <p>Mock rule engine</p>
              </div>
            </article>
          )
        })}
      </div>

      <section className="automation-rules">
        {rules.map((rule) => (
          <article className="rule-card" key={rule.id}>
            <div className="rule-main">
              <div className="rule-icon">
                <Settings size={22} aria-hidden="true" />
              </div>
              <div>
                <h2>{rule.name}</h2>
                <p>{rule.description}</p>
                <div className="rule-expression">
                  <span>{rule.sensor}</span>
                  <strong>{rule.operator}</strong>
                  <label>
                    Threshold
                    <input
                      type="number"
                      value={rule.threshold}
                      onChange={(event) => updateThreshold(rule.id, Number(event.target.value))}
                    />
                  </label>
                  <span>{rule.unit}</span>
                </div>
              </div>
            </div>

            <div className="rule-side">
              <span className={rule.enabled ? 'rule-status enabled' : 'rule-status'}>{rule.enabled ? 'Enabled' : 'Disabled'}</span>
              <strong>{rule.action}</strong>
              <button
                className={`switch ${rule.enabled ? 'on' : ''}`}
                type="button"
                aria-label={`Toggle ${rule.name}`}
                aria-pressed={rule.enabled}
                onClick={() => toggleRule(rule.id)}
              >
                <span></span>
              </button>
            </div>
          </article>
        ))}
      </section>

      <article className="automation-note">
        <ShieldCheck size={22} aria-hidden="true" />
        <div>
          <h2>Suggested safety logic</h2>
          <p>
            Nen bat buzzer ngay khi khoi vuot nguong canh bao, nhung servo xa nuoc nen co them dieu kien xac nhan chay
            hoac che do manual confirm de tranh kich hoat nham.
          </p>
        </div>
      </article>
    </section>
  )
}

export default AutomationPage
