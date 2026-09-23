import type { Doc } from '../types'
import Icon from './Icon'

interface EditorTabsProps {
  docs: Doc[]
  activeId: string | null
  onSelect: (id: string) => void
  onClose: (id: string) => void
}

export default function EditorTabs({ docs, activeId, onSelect, onClose }: EditorTabsProps) {
  if (docs.length === 0) {
    return <div className="tabs empty" />
  }

  return (
    <div className="tabs" role="tablist">
      {docs.map((doc) => (
        <div
          key={doc.id}
          className={`tab${doc.id === activeId ? ' active' : ''}`}
          role="tab"
          aria-selected={doc.id === activeId}
          title={doc.file}
          onClick={() => onSelect(doc.id)}
        >
          <Icon name="files" size={14} className="tab-icon" />
          <span className="tab-name">{doc.name}</span>
          {doc.dirty ? <span className="tab-dirty" title="Unsaved changes" /> : null}
          <button
            type="button"
            className="tab-close"
            onClick={(event) => {
              event.stopPropagation()
              onClose(doc.id)
            }}
            title="Close"
          >
            <Icon name="close" size={12} />
          </button>
        </div>
      ))}
    </div>
  )
}
