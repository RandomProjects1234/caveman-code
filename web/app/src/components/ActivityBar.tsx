import type { ViewId } from '../types'
import Icon from './Icon'
import type { IconName } from './Icon'

interface ActivityBarProps {
  view: ViewId
  onChange: (view: ViewId) => void
}

const ITEMS: { id: ViewId; icon: IconName; label: string; title: string }[] = [
  { id: 'explorer', icon: 'files', label: 'Explorer', title: 'Explorer' },
  { id: 'blocks', icon: 'blocks', label: 'Blocks', title: 'Blocks (Ctrl+B)' },
  { id: 'draw', icon: 'draw', label: 'Draw', title: 'Draw' },
  { id: 'dictionary', icon: 'book', label: 'Dictionary', title: 'Dictionary' },
]

export default function ActivityBar({ view, onChange }: ActivityBarProps) {
  return (
    <nav className="activitybar" aria-label="Views">
      {ITEMS.map((item) => (
        <button
          key={item.id}
          type="button"
          className={`activity-item${view === item.id ? ' active' : ''}`}
          onClick={() => onChange(item.id)}
          title={item.title}
          aria-pressed={view === item.id}
        >
          <Icon name={item.icon} size={21} />
          <span className="activity-label">{item.label}</span>
        </button>
      ))}
    </nav>
  )
}
