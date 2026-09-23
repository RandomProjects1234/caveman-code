import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import ActivityBar from './components/ActivityBar'
import AskDialog from './components/AskDialog'
import BlocksView from './components/BlocksView'
import DictionaryView from './components/DictionaryView'
import DrawView from './components/DrawView'
import EditorPane from './components/EditorPane'
import EditorTabs from './components/EditorTabs'
import Explorer from './components/Explorer'
import OutputPane from './components/OutputPane'
import StatusBar from './components/StatusBar'
import TitleBar from './components/TitleBar'
import { useCmc } from './hooks/useCmc'
import type {
  Doc,
  DrawOp,
  Example,
  OutputKind,
  OutputLine,
  RunState,
  ViewId,
} from './types'

interface RawExample {
  name?: unknown
  file?: unknown
  source?: unknown
}

function exampleSource(raw: unknown): string {
  if (typeof raw === 'string') return raw
  if (raw && typeof raw === 'object') {
    const value = (raw as { value?: unknown }).value
    if (typeof value === 'string') return value
  }
  return ''
}

function normalizeExamples(data: unknown): Example[] {
  if (!Array.isArray(data)) return []
  const examples: Example[] = []
  for (const entry of data as RawExample[]) {
    if (!entry || typeof entry !== 'object') continue
    const name = typeof entry.name === 'string' ? entry.name : ''
    const file = typeof entry.file === 'string' ? entry.file : ''
    const source = exampleSource(entry.source)
    if (!name && !file) continue
    examples.push({ name: name || file, file: file || name, source })
  }
  return examples
}

function downloadText(filename: string, text: string, mime: string): void {
  const blob = new Blob([text], { type: mime })
  const url = URL.createObjectURL(blob)
  const link = document.createElement('a')
  link.href = url
  link.download = filename
  document.body.appendChild(link)
  link.click()
  link.remove()
  URL.revokeObjectURL(url)
}

export default function App() {
  const [examples, setExamples] = useState<Example[]>([])
  const [examplesError, setExamplesError] = useState('')
  const [docs, setDocs] = useState<Doc[]>([])
  const [activeId, setActiveId] = useState<string | null>(null)
  const [view, setView] = useState<ViewId>('explorer')
  const [outputLines, setOutputLines] = useState<OutputLine[]>([])
  const [outputVisible, setOutputVisible] = useState(true)
  const [runState, setRunState] = useState<RunState>('ready')
  const [drawOps, setDrawOps] = useState<DrawOp[]>([])
  const [ask, setAsk] = useState<{
    id: number
    prompt: string
    done: (answer: string) => void
  } | null>(null)
  const [cursor, setCursor] = useState({ line: 1, column: 1 })
  const [statusMessage, setStatusMessage] = useState('Ready')

  const lineIdRef = useRef(0)
  const docIdRef = useRef(0)
  const askIdRef = useRef(0)
  const sawDrawRef = useRef(false)
  const askRef = useRef<{
    id: number
    prompt: string
    done: (answer: string) => void
  } | null>(null)
  useEffect(() => {
    askRef.current = ask
  })

  const append = useCallback((kind: OutputKind, text: string) => {
    lineIdRef.current += 1
    const line: OutputLine = { id: lineIdRef.current, kind, text }
    setOutputLines((current) => {
      const trimmed = current.length >= 2000 ? current.slice(current.length - 1500) : current
      return [...trimmed, line]
    })
  }, [])

  const cmc = useCmc({
    onEmit: (line) => {
      append('normal', line)
    },
    onDraw: (op) => {
      setDrawOps((current) => [...current, op])
      if (!sawDrawRef.current) {
        sawDrawRef.current = true
        setView('draw')
      }
    },
    onAsk: (prompt, done) => {
      askIdRef.current += 1
      setAsk({ id: askIdRef.current, prompt, done })
    },
  })

  useEffect(() => {
    let alive = true
    const url = `${import.meta.env.BASE_URL}examples/examples.json`
    fetch(url)
      .then((response) => {
        if (!response.ok) throw new Error(`HTTP ${response.status}`)
        return response.json() as Promise<unknown>
      })
      .then((data) => {
        if (alive) setExamples(normalizeExamples(data))
      })
      .catch((reason: unknown) => {
        if (!alive) return
        setExamplesError('Could not load examples.json')
        append(
          'error',
          `Could not load examples: ${reason instanceof Error ? reason.message : String(reason)}`,
        )
      })
    return () => {
      alive = false
    }
  }, [append])

  const activeDoc = useMemo(() => docs.find((doc) => doc.id === activeId) ?? null, [docs, activeId])

  const openExample = useCallback(
    (example: Example) => {
      const existing = docs.find((doc) => doc.file === example.file)
      if (existing) {
        setActiveId(existing.id)
        setStatusMessage(`Opened ${example.file}`)
        return
      }
      docIdRef.current += 1
      const doc: Doc = {
        id: `doc-${docIdRef.current}`,
        name: example.name,
        file: example.file,
        source: example.source,
        dirty: false,
      }
      setDocs((current) => [...current, doc])
      setActiveId(doc.id)
      setStatusMessage(`Opened ${example.file}`)
    },
    [docs],
  )

  const newFile = useCallback(() => {
    docIdRef.current += 1
    const index = docs.filter((doc) => doc.name.startsWith('untitled')).length + 1
    const doc: Doc = {
      id: `doc-${docIdRef.current}`,
      name: `untitled-${index}.cmc`,
      file: `untitled-${index}.cmc`,
      source: 'ugg A fresh cave file.\n',
      dirty: false,
    }
    setDocs((current) => [...current, doc])
    setActiveId(doc.id)
    setView('explorer')
    setStatusMessage('Made a new file')
  }, [docs])

  const closeDoc = useCallback(
    (id: string) => {
      const index = docs.findIndex((doc) => doc.id === id)
      if (index < 0) return
      const next = docs.filter((doc) => doc.id !== id)
      setDocs(next)
      if (activeId === id) {
        const fallback = next[Math.min(index, next.length - 1)]
        setActiveId(fallback ? fallback.id : null)
      }
    },
    [docs, activeId],
  )

  const handleEditorChange = useCallback(
    (value: string) => {
      if (!activeId) return
      setDocs((current) =>
        current.map((doc) => (doc.id === activeId ? { ...doc, source: value, dirty: true } : doc)),
      )
    },
    [activeId],
  )

  const handleCursor = useCallback((line: number, column: number) => {
    setCursor({ line, column })
  }, [])

  const handleSave = useCallback(() => {
    if (!activeDoc) {
      setStatusMessage('No file to save')
      return
    }
    downloadText(activeDoc.file, activeDoc.source, 'text/plain')
    setDocs((current) =>
      current.map((doc) => (doc.id === activeDoc.id ? { ...doc, dirty: false } : doc)),
    )
    append('info', `Saved ${activeDoc.file}`)
    setStatusMessage(`Saved ${activeDoc.file}`)
  }, [activeDoc, append])

  const handleExport = useCallback(() => {
    if (!activeDoc) {
      setStatusMessage('No file to export')
      return
    }
    if (!cmc.engine) {
      append('error', 'Engine not built yet. Python export needs the CMC engine.')
      setStatusMessage('Engine not built yet')
      return
    }
    const result = cmc.engine.compile(activeDoc.source, activeDoc.file)
    if (!result.ok) {
      append('error', result.error || 'Could not export Python.')
      setStatusMessage('Export failed')
      return
    }
    const name = activeDoc.file.replace(/\.cmc$/i, '') + '.py'
    downloadText(name, result.python, 'text/x-python')
    append('success', `Exported ${name}`)
    setStatusMessage(`Exported ${name}`)
  }, [activeDoc, append, cmc.engine])

  const handleRun = useCallback(
    async (sourceOverride?: string, filenameOverride?: string) => {
      const source = sourceOverride ?? activeDoc?.source
      const filename = filenameOverride ?? activeDoc?.file ?? '<web>'
      if (source === undefined) {
        append('info', 'Open an example or make a new file first.')
        setStatusMessage('No file to run')
        setView('explorer')
        return
      }
      if (cmc.status === 'missing') {
        setOutputVisible(true)
        append('error', 'Engine not built yet. Build public/wasm/cmc.js to run programs.')
        setStatusMessage('Engine not built yet')
        return
      }
      if (cmc.status === 'loading' || !cmc.engine) {
        setOutputVisible(true)
        append('info', 'The engine is still loading. Try again in a moment.')
        setStatusMessage('Engine loading')
        return
      }
      if (cmc.running) {
        setOutputVisible(true)
        append('info', 'A program is already running. Press Esc to stop it.')
        setStatusMessage('Already running')
        return
      }
      sawDrawRef.current = false
      setOutputVisible(true)
      setRunState('running')
      append('info', `> run ${filename}`)
      setStatusMessage(`Running ${filename}...`)
      const result = await cmc.runProgram(source, filename)
      if (result.stopped) {
        setRunState('stopped')
        append('info', 'Stopped!')
        setStatusMessage('Stopped')
      } else if (result.error) {
        setRunState('done')
        append('error', result.error)
        setStatusMessage('Program had an error')
      } else {
        setRunState('done')
        append('success', 'Program finished!')
        setStatusMessage('Program finished')
      }
    },
    [activeDoc, append, cmc],
  )

  const runActive = useCallback(() => {
    void handleRun()
  }, [handleRun])

  const runSource = useCallback(
    (source: string, filename: string) => {
      void handleRun(source, filename)
    },
    [handleRun],
  )

  const handleStop = useCallback(() => {
    if (!cmc.running) return
    cmc.stopProgram()
    setStatusMessage('Stopping...')
  }, [cmc])

  const handleAskAnswer = useCallback((answer: string) => {
    const current = askRef.current
    setAsk(null)
    current?.done(answer)
  }, [])

  const sendToText = useCallback(
    (source: string) => {
      if (activeDoc) {
        setDocs((current) =>
          current.map((doc) =>
            doc.id === activeDoc.id ? { ...doc, source, dirty: true } : doc,
          ),
        )
        append('info', `Put the blocks into ${activeDoc.file}`)
        setStatusMessage(`Updated ${activeDoc.file} from blocks`)
      } else {
        docIdRef.current += 1
        const doc: Doc = {
          id: `doc-${docIdRef.current}`,
          name: 'blocks.cmc',
          file: 'blocks.cmc',
          source,
          dirty: true,
        }
        setDocs((current) => [...current, doc])
        setActiveId(doc.id)
        append('info', 'Made blocks.cmc from the blocks')
        setStatusMessage('Made blocks.cmc from blocks')
      }
      setView('explorer')
    },
    [activeDoc, append],
  )

  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.defaultPrevented) return
      const key = event.key
      if (key === 'F5') {
        event.preventDefault()
        event.stopPropagation()
        runActive()
        return
      }
      if (key === 'Escape') {
        if (askRef.current) return
        event.preventDefault()
        event.stopPropagation()
        handleStop()
        return
      }
      if (!event.ctrlKey || event.altKey || event.metaKey) return
      const lower = key.toLowerCase()
      if (lower === 's') {
        event.preventDefault()
        event.stopPropagation()
        handleSave()
      } else if (lower === 'e') {
        event.preventDefault()
        event.stopPropagation()
        handleExport()
      } else if (lower === 'b') {
        event.preventDefault()
        event.stopPropagation()
        setView((current) => (current === 'blocks' ? 'explorer' : 'blocks'))
      } else if (key === '`') {
        event.preventDefault()
        event.stopPropagation()
        setOutputVisible((current) => !current)
      }
    }
    window.addEventListener('keydown', onKeyDown, true)
    return () => window.removeEventListener('keydown', onKeyDown, true)
  }, [runActive, handleStop, handleSave, handleExport])

  const clearOutput = useCallback(() => {
    setOutputLines([])
    setRunState('ready')
    setStatusMessage('Cleared the output')
  }, [])

  const clearDraw = useCallback(() => {
    setDrawOps([])
    setStatusMessage('Cleared the drawing')
  }, [])

  return (
    <div className="app">
      <TitleBar
        engineStatus={cmc.status}
        engineError={cmc.error}
        running={cmc.running}
        onRun={runActive}
        onStop={handleStop}
        onExport={handleExport}
        onExamples={() => setView('explorer')}
        onDictionary={() => setView('dictionary')}
      />

      <div className="workbench">
        <ActivityBar view={view} onChange={setView} />

        <div className={`sidebar-host${view === 'explorer' ? '' : ' hidden'}`}>
          <Explorer
            examples={examples}
            examplesError={examplesError}
            activeFile={activeDoc?.file ?? null}
            onOpenExample={openExample}
            onNewFile={newFile}
          />
        </div>

        <main className="main">
          <div className={`view-host${view === 'explorer' ? '' : ' hidden'}`}>
            <EditorTabs
              docs={docs}
              activeId={activeId}
              onSelect={setActiveId}
              onClose={closeDoc}
            />
            <EditorPane
              doc={activeDoc}
              onChange={handleEditorChange}
              onCursor={handleCursor}
              onRun={runActive}
              onStop={handleStop}
            />
          </div>

          <div className={`view-host${view === 'blocks' ? '' : ' hidden'}`}>
            <BlocksView
              engine={cmc.engine}
              engineStatus={cmc.status}
              activeDoc={activeDoc}
              onRunSource={runSource}
              onSendToText={sendToText}
            />
          </div>

          <div className={`view-host${view === 'draw' ? '' : ' hidden'}`}>
            <DrawView ops={drawOps} onClear={clearDraw} />
          </div>

          <div className={`view-host${view === 'dictionary' ? '' : ' hidden'}`}>
            <DictionaryView engine={cmc.engine} engineStatus={cmc.status} />
          </div>
        </main>
      </div>

      {outputVisible ? (
        <OutputPane
          lines={outputLines}
          state={runState}
          onClear={clearOutput}
          onToggle={() => setOutputVisible(false)}
        />
      ) : null}

      <StatusBar
        message={statusMessage}
        line={cursor.line}
        column={cursor.column}
        version={cmc.version}
        engineStatus={cmc.status}
        view={view}
        outputVisible={outputVisible}
        onToggleOutput={() => setOutputVisible((current) => !current)}
      />

      {ask ? <AskDialog key={ask.id} prompt={ask.prompt} onSubmit={handleAskAnswer} /> : null}
    </div>
  )
}
