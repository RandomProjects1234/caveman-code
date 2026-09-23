import { useCallback, useEffect, useRef, useState } from 'react'
import type { Cmc, RunResult } from '../wasm/cmc'
import { loadCmc } from '../wasm/cmc'
import type { DrawOp, EngineStatus } from '../types'

export interface CmcHandlers {
  onEmit: (line: string) => void
  onDraw: (op: DrawOp) => void
  onAsk: (prompt: string, done: (answer: string) => void) => void
}

export interface CmcController {
  engine: Cmc | null
  status: EngineStatus
  error: string
  version: string
  running: boolean
  runProgram: (source: string, filename: string) => Promise<RunResult>
  stopProgram: () => void
}

export function useCmc(handlers: CmcHandlers): CmcController {
  const handlersRef = useRef(handlers)
  useEffect(() => {
    handlersRef.current = handlers
  })

  const stopRef = useRef(false)
  const runningRef = useRef(false)
  const engineRef = useRef<Cmc | null>(null)

  const [engine, setEngine] = useState<Cmc | null>(null)
  const [status, setStatus] = useState<EngineStatus>('loading')
  const [error, setError] = useState('')
  const [version, setVersion] = useState('')
  const [running, setRunning] = useState(false)

  useEffect(() => {
    let alive = true
    loadCmc({
      emit: (line) => handlersRef.current.onEmit(line),
      draw: (op, a, b, c, d, text) => handlersRef.current.onDraw({ op, a, b, c, d, text }),
      shouldStop: () => stopRef.current,
      ask: (prompt, done) => handlersRef.current.onAsk(prompt, done),
    })
      .then((instance) => {
        if (!alive) return
        engineRef.current = instance
        setEngine(instance)
        setVersion(instance.version())
        setStatus('ready')
      })
      .catch((reason: unknown) => {
        if (!alive) return
        setError(reason instanceof Error ? reason.message : String(reason))
        setStatus('missing')
      })
    return () => {
      alive = false
    }
  }, [])

  const runProgram = useCallback(async (source: string, filename: string): Promise<RunResult> => {
    const instance = engineRef.current
    if (!instance) {
      return { ok: false, output: [], stopped: false, error: 'Engine not built yet.' }
    }
    if (runningRef.current) {
      return { ok: false, output: [], stopped: false, error: 'A program is already running.' }
    }
    runningRef.current = true
    stopRef.current = false
    setRunning(true)
    try {
      const result = await instance.run(source, filename)
      return result
    } catch (reason) {
      return {
        ok: false,
        output: [],
        stopped: false,
        error: reason instanceof Error ? reason.message : String(reason),
      }
    } finally {
      runningRef.current = false
      setRunning(false)
    }
  }, [])

  const stopProgram = useCallback(() => {
    stopRef.current = true
  }, [])

  return { engine, status, error, version, running, runProgram, stopProgram }
}
