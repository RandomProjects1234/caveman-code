import type { EngineStatus, ViewId } from '../types'
import Icon from './Icon'

interface StatusBarProps {
  message: string
  line: number
  column: number
  version: string
  engineStatus: EngineStatus
  view: ViewId
  outputVisible: boolean
  onToggleOutput: () => void
}

const VIEW_LABEL: Record<ViewId, string> = {
  explorer: 'Explorer',
  blocks: 'Blocks',
  draw: 'Draw',
  dictionary: 'Dictionary',
}

export default function StatusBar({
  message,
  line,
  column,
  version,
  engineStatus,
  view,
  outputVisible,
  onToggleOutput,
}: StatusBarProps) {
  return (
    <footer className="statusbar">
      <div className="status-left">
        <Icon name="cave" size={13} />
        <span className="status-message">{message}</span>
      </div>
      <div className="status-right">
        <button
          type="button"
          className={`status-button${outputVisible ? ' active' : ''}`}
          onClick={onToggleOutput}
          title="Toggle output panel (Ctrl+`)"
        >
          <Icon name="terminal" size={13} />
          <span>Output</span>
        </button>
        <span className="status-item">{VIEW_LABEL[view]}</span>
        <span className="status-item">
          Line {line}, Col {column}
        </span>
        <span className="status-item">
          {engineStatus === 'ready' && version ? `CMC ${version}` : 'engine missing'}
        </span>
      </div>
    </footer>
  )
}
