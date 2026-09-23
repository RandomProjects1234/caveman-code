import { useMemo, useState } from 'react'
import type { Example } from '../types'
import Icon from './Icon'

interface ExplorerProps {
  examples: Example[]
  examplesError: string
  activeFile: string | null
  onOpenExample: (example: Example) => void
  onNewFile: () => void
}

export default function Explorer({
  examples,
  examplesError,
  activeFile,
  onOpenExample,
  onNewFile,
}: ExplorerProps) {
  const [filter, setFilter] = useState('')

  const visible = useMemo(() => {
    const needle = filter.trim().toLowerCase()
    if (!needle) return examples
    return examples.filter((example) =>
      `${example.name} ${example.file}`.toLowerCase().includes(needle),
    )
  }, [examples, filter])

  return (
    <aside className="sidebar">
      <div className="sidebar-header">
        <span className="sidebar-title">Explorer</span>
        <button type="button" className="icon-button" onClick={onNewFile} title="New file">
          <Icon name="plus" />
        </button>
      </div>

      <div className="sidebar-section">
        <div className="sidebar-section-title">
          <Icon name="examples" size={13} />
          <span>Examples</span>
          <span className="sidebar-count">{examples.length}</span>
        </div>

        <label className="sidebar-search">
          <Icon name="search" size={13} />
          <input
            value={filter}
            onChange={(event) => setFilter(event.target.value)}
            placeholder="Filter examples"
            spellCheck={false}
          />
        </label>

        <div className="explorer-list">
          {examplesError ? (
            <p className="explorer-empty">{examplesError}</p>
          ) : visible.length === 0 ? (
            <p className="explorer-empty">
              {examples.length === 0 ? 'Loading examples...' : 'No matching examples.'}
            </p>
          ) : (
            visible.map((example) => (
              <button
                key={example.file}
                type="button"
                className={`explorer-item${activeFile === example.file ? ' active' : ''}`}
                onClick={() => onOpenExample(example)}
                title={example.file}
              >
                <Icon name="files" size={14} />
                <span className="explorer-item-name">{example.name}</span>
              </button>
            ))
          )}
        </div>
      </div>
    </aside>
  )
}
