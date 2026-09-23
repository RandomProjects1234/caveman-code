import type { JSX } from 'react'

export type IconName =
  | 'cave'
  | 'play'
  | 'stop'
  | 'download'
  | 'examples'
  | 'book'
  | 'files'
  | 'blocks'
  | 'draw'
  | 'search'
  | 'plus'
  | 'close'
  | 'chevronDown'
  | 'chevronRight'
  | 'arrowUp'
  | 'arrowDown'
  | 'copy'
  | 'trash'
  | 'undo'
  | 'send'
  | 'import'
  | 'terminal'
  | 'dot'
  | 'check'
  | 'warning'

const ICONS: Record<IconName, JSX.Element> = {
  cave: (
    <>
      <path d="M2.5 13.5V7.4a5.5 5.5 0 0 1 11 0v6.1" />
      <path d="M6 13.5v-2.6a2 2 0 0 1 4 0v2.6" />
    </>
  ),
  play: <path d="M4.5 2.8 12.5 8l-8 5.2z" fill="currentColor" stroke="none" />,
  stop: <rect x="3.5" y="3.5" width="9" height="9" rx="1.2" fill="currentColor" stroke="none" />,
  download: (
    <>
      <path d="M8 2.4v7.8" />
      <path d="m4.8 7.2 3.2 3 3.2-3" />
      <path d="M3 13.4h10" />
    </>
  ),
  examples: <path d="M3 3.6h10M3 8h10M3 12.4h6" />,
  book: (
    <>
      <path d="M3 3.2h4.2c.7 0 1.3.6 1.3 1.3v8.3c0-.6-.6-1.1-1.3-1.1H3z" />
      <path d="M13 3.2H8.8c-.7 0-1.3.6-1.3 1.3v8.3c0-.6.6-1.1 1.3-1.1H13z" />
    </>
  ),
  files: <path d="M2.5 4.2h4.1l1.2 1.5h5.7v7.1H2.5z" />,
  blocks: (
    <>
      <rect x="2.5" y="2.5" width="5" height="5" rx="1" />
      <rect x="8.5" y="2.5" width="5" height="5" rx="1" />
      <rect x="2.5" y="8.5" width="5" height="5" rx="1" />
      <rect x="8.5" y="8.5" width="5" height="5" rx="1" />
    </>
  ),
  draw: (
    <>
      <path d="m2.8 13.2 1-3.1 6.6-6.6 2.1 2.1-6.6 6.6z" />
      <path d="m9.3 4.6 2.1 2.1" />
    </>
  ),
  search: (
    <>
      <circle cx="7" cy="7" r="4" />
      <path d="m10.1 10.1 3.3 3.3" />
    </>
  ),
  plus: <path d="M8 3v10M3 8h10" />,
  close: <path d="m4 4 8 8M12 4l-8 8" />,
  chevronDown: <path d="m4 6 4 4 4-4" />,
  chevronRight: <path d="m6 4 4 4-4 4" />,
  arrowUp: <path d="M8 13.2V3.2M4 7l4-3.8L12 7" />,
  arrowDown: <path d="M8 2.8v10M4 9l4 3.8L12 9" />,
  copy: (
    <>
      <rect x="5.5" y="5.5" width="8" height="8" rx="1.5" />
      <path d="M10.5 5.5v-1a1.5 1.5 0 0 0-1.5-1.5H4A1.5 1.5 0 0 0 2.5 4.5V10A1.5 1.5 0 0 0 4 11.5h1" />
    </>
  ),
  trash: (
    <>
      <path d="M3 4.4h10" />
      <path d="M6.4 4.4V3h3.2v1.4" />
      <path d="m4.6 4.4.7 9.1h5.4l.7-9.1" />
    </>
  ),
  undo: (
    <>
      <path d="M3.4 7.6h6.2a3.1 3.1 0 0 1 0 6.2H6.2" />
      <path d="M6.4 4.6 3.4 7.6l3 3" />
    </>
  ),
  send: (
    <>
      <path d="M2.5 8h10.6" />
      <path d="M9.7 4.5 13.1 8l-3.4 3.5" />
    </>
  ),
  import: (
    <>
      <path d="M8 2.5v7.3" />
      <path d="m4.9 6.7 3.1 3.1 3.1-3.1" />
      <path d="M2.5 12.9h11" />
    </>
  ),
  terminal: (
    <>
      <rect x="2.5" y="3.2" width="11" height="9.6" rx="1.2" />
      <path d="m5.2 6.5 2 1.8-2 1.8" />
      <path d="M8.8 10.1h2.4" />
    </>
  ),
  dot: <circle cx="8" cy="8" r="3" fill="currentColor" stroke="none" />,
  check: <path d="m3 8.6 3 3 7-7.2" />,
  warning: (
    <>
      <path d="M8 2.6 14 13.4H2z" />
      <path d="M8 6.6v3.4" />
      <path d="M8 12.1h.01" />
    </>
  ),
}

interface IconProps {
  name: IconName
  size?: number
  className?: string
}

export default function Icon({ name, size = 16, className }: IconProps) {
  return (
    <svg
      className={className}
      width={size}
      height={size}
      viewBox="0 0 16 16"
      fill="none"
      stroke="currentColor"
      strokeWidth={1.4}
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
      focusable="false"
    >
      {ICONS[name]}
    </svg>
  )
}
