import { useEffect, useRef } from 'react'
import Editor from '@monaco-editor/react'
import type { BeforeMount, OnMount } from '@monaco-editor/react'
import { CMC_LANGUAGE, CMC_THEME, registerCmcLanguage } from '../cmcLanguage'
import type { Doc } from '../types'
import Icon from './Icon'

interface EditorPaneProps {
  doc: Doc | null
  onChange: (value: string) => void
  onCursor: (line: number, column: number) => void
  onRun: () => void
  onStop: () => void
}

const SHORTCUTS: { keys: string; label: string }[] = [
  { keys: 'F5', label: 'Run the program' },
  { keys: 'Esc', label: 'Stop the program' },
  { keys: 'Ctrl+S', label: 'Download the active file' },
  { keys: 'Ctrl+E', label: 'Export the active file to Python' },
  { keys: 'Ctrl+B', label: 'Toggle the blocks editor' },
  { keys: 'Ctrl+`', label: 'Toggle the output panel' },
]

export default function EditorPane({
  doc,
  onChange,
  onCursor,
  onRun,
  onStop,
}: EditorPaneProps) {
  const onRunRef = useRef(onRun)
  const onStopRef = useRef(onStop)
  const onCursorRef = useRef(onCursor)
  useEffect(() => {
    onRunRef.current = onRun
    onStopRef.current = onStop
    onCursorRef.current = onCursor
  })

  const beforeMount: BeforeMount = (monaco) => {
    registerCmcLanguage(monaco)
  }

  const handleMount: OnMount = (editor, monaco) => {
    const report = () => {
      const position = editor.getPosition()
      if (position) onCursorRef.current(position.lineNumber, position.column)
    }
    editor.onDidChangeCursorPosition(report)
    editor.onDidChangeModel(report)
    report()
    editor.addAction({
      id: 'cmc.run',
      label: 'Run Program',
      keybindings: [monaco.KeyCode.F5],
      run: () => onRunRef.current(),
    })
    editor.addAction({
      id: 'cmc.stop',
      label: 'Stop Program',
      keybindings: [monaco.KeyCode.Escape],
      run: () => onStopRef.current(),
    })
  }

  if (!doc) {
    return (
      <div className="editor-host welcome">
        <div className="welcome-inner">
          <span className="welcome-logo">
            <Icon name="cave" size={72} />
          </span>
          <h1>OGABOOGA CODER</h1>
          <p className="welcome-sub">
            Cave Man Code in your browser. Pick an example from the Explorer, or make a new file.
          </p>
          <div className="welcome-keys">
            {SHORTCUTS.map((item) => (
              <div key={item.keys} className="welcome-key-row">
                <kbd>{item.keys}</kbd>
                <span>{item.label}</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    )
  }

  return (
    <div className="editor-host">
      <Editor
        path={doc.id}
        language={CMC_LANGUAGE}
        theme={CMC_THEME}
        value={doc.source}
        beforeMount={beforeMount}
        onMount={handleMount}
        onChange={(value) => onChange(value ?? '')}
        loading={<div className="editor-loading">Loading editor...</div>}
        options={{
          fontFamily: 'Consolas, "Courier New", monospace',
          fontSize: 14,
          lineHeight: 21,
          minimap: { enabled: false },
          scrollBeyondLastLine: false,
          automaticLayout: true,
          tabSize: 4,
          insertSpaces: true,
          renderWhitespace: 'selection',
          smoothScrolling: true,
          cursorBlinking: 'smooth',
          padding: { top: 12, bottom: 12 },
          scrollbar: { verticalScrollbarSize: 12, horizontalScrollbarSize: 12 },
          wordWrap: 'off',
          roundedSelection: false,
          fixedOverflowWidgets: true,
          bracketPairColorization: { enabled: false },
        }}
      />
    </div>
  )
}
