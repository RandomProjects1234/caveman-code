import { useEffect, useRef } from 'react'
import type { OutputLine, RunState } from '../types'
import Icon from './Icon'

interface OutputPaneProps {
  lines: OutputLine[]
  state: RunState
  onClear: () => void
  onToggle: () => void
}

const STATE_LABEL: Record<RunState, string> = {
  ready: 'ready',
  running: 'running',
  done: 'done',
  stopped: 'stopped',
}

export default function OutputPane({ lines, state, onClear, onToggle }: OutputPaneProps) {
  const listRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    const list = listRef.current
    if (list) list.scrollTop = list.scrollHeight
  }, [lines])

  return (
    <section className="output">
      <div className="output-header">
        <span className="output-title">
          <Icon name="terminal" size={14} />
          <span>Output</span>
        </span>
        <span className={`run-state ${state}`}>{STATE_LABEL[state]}</span>
        <span className="output-spacer" />
        <button type="button" className="output-button" onClick={onClear} title="Clear output">
          <Icon name="trash" size={13} />
          <span>Clear</span>
        </button>
        <button
          type="button"
          className="icon-button"
          onClick={onToggle}
          title="Toggle output panel (Ctrl+`)"
        >
          <Icon name="chevronDown" size={14} />
        </button>
      </div>
      <div className="output-lines" ref={listRef}>
        {lines.length === 0 ? (
          <div className="out-line out-info">Output is empty. Press F5 to run a program.</div>
        ) : (
          lines.map((line) => (
            <div key={line.id} className={`out-line out-${line.kind}`}>
              {line.kind === 'error' ? <Icon name="warning" size={13} /> : null}
              <span>{line.text}</span>
            </div>
          ))
        )}
      </div>
    </section>
  )
}
