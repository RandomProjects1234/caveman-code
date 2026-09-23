import type { EngineStatus } from '../types'
import Icon from './Icon'

interface TitleBarProps {
  engineStatus: EngineStatus
  engineError: string
  running: boolean
  onRun: () => void
  onStop: () => void
  onExport: () => void
  onExamples: () => void
  onDictionary: () => void
}

const ENGINE_LABEL: Record<EngineStatus, string> = {
  loading: 'engine loading',
  ready: 'engine ready',
  missing: 'engine missing',
}

export default function TitleBar({
  engineStatus,
  engineError,
  running,
  onRun,
  onStop,
  onExport,
  onExamples,
  onDictionary,
}: TitleBarProps) {
  return (
    <header className="titlebar">
      <div className="titlebar-left">
        <span className="brand-mark">
          <Icon name="cave" size={19} />
        </span>
        <span className="brand-text">
          OGABOOGA <strong>CODER</strong>
        </span>
        <span className="brand-badge">WEB</span>
      </div>

      <div className="titlebar-actions">
        <button type="button" className="tb-button" onClick={onRun} title="Run program (F5)">
          <Icon name="play" size={14} />
          <span>Run</span>
          <kbd>F5</kbd>
        </button>
        <button
          type="button"
          className="tb-button"
          onClick={onStop}
          disabled={!running}
          title="Stop program (Esc)"
        >
          <Icon name="stop" size={13} />
          <span>Stop</span>
          <kbd>Esc</kbd>
        </button>
        <span className="tb-divider" />
        <button type="button" className="tb-button" onClick={onExport} title="Export Python (Ctrl+E)">
          <Icon name="download" size={14} />
          <span>Export .py</span>
        </button>
        <button type="button" className="tb-button" onClick={onExamples} title="Show examples">
          <Icon name="examples" size={14} />
          <span>Examples</span>
        </button>
        <button type="button" className="tb-button" onClick={onDictionary} title="Open dictionary">
          <Icon name="book" size={14} />
          <span>Dictionary</span>
        </button>
      </div>

      <div className="titlebar-right">
        <span
          className={`engine-chip ${engineStatus}`}
          title={engineError ? `Engine error: ${engineError}` : ENGINE_LABEL[engineStatus]}
        >
          <span className="engine-dot" />
          {ENGINE_LABEL[engineStatus]}
        </span>
      </div>
    </header>
  )
}
