export type EngineStatus = 'loading' | 'ready' | 'missing'

export type ViewId = 'explorer' | 'blocks' | 'draw' | 'dictionary'

export type OutputKind = 'normal' | 'error' | 'info' | 'success'

export interface OutputLine {
  id: number
  kind: OutputKind
  text: string
}

export type RunState = 'ready' | 'running' | 'done' | 'stopped'

export interface Doc {
  id: string
  name: string
  file: string
  source: string
  dirty: boolean
}

export interface Example {
  name: string
  file: string
  source: string
}

export interface DrawOp {
  op: string
  a: number
  b: number
  c: number
  d: number
  text: string
}

export interface CursorPosition {
  line: number
  column: number
}
