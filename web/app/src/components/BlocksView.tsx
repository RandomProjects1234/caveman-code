import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import type { BlockNode, BlockSpec, BlockSpecs, Cmc } from '../wasm/cmc'
import type { Doc, EngineStatus } from '../types'
import Icon from './Icon'

interface UiBlock {
  id: number
  kind: string
  fields: Record<string, string>
  body: UiBlock[]
  elseBody: UiBlock[]
}

interface InsertTarget {
  id: number
  part: 'body' | 'elseBody'
}

interface BlockActions {
  select: (id: number) => void
  setField: (id: number, key: string, value: string) => void
  move: (id: number, direction: -1 | 1) => void
  duplicate: (id: number) => void
  remove: (id: number) => void
  addHere: (id: number, part: 'body' | 'elseBody') => void
}

interface BlocksViewProps {
  engine: Cmc | null
  engineStatus: EngineStatus
  activeDoc: Doc | null
  onRunSource: (source: string, filename: string) => void
  onSendToText: (source: string) => void
}

const CATEGORY_COLORS: Record<string, string> = {
  Talking: '#ffb340',
  Boxes: '#64b5f6',
  Choices: '#c792ea',
  Loops: '#4cc38a',
  Clumps: '#f38ba8',
  Values: '#d0c4e8',
  Piles: '#8fd694',
  Drawing: '#f9c74f',
}

const FALLBACK_COLOR = '#b39b80'
const BLOCK_TEXT = '#241a12'

let nextBlockId = 1

function createBlock(kind: string, spec: BlockSpec): UiBlock {
  const fields: Record<string, string> = {}
  for (const field of spec.fields) {
    fields[field.key] = spec.defaults[field.key] ?? ''
  }
  return { id: nextBlockId++, kind, fields, body: [], elseBody: [] }
}

function cloneBlock(block: UiBlock): UiBlock {
  return {
    id: nextBlockId++,
    kind: block.kind,
    fields: { ...block.fields },
    body: block.body.map(cloneBlock),
    elseBody: block.elseBody.map(cloneBlock),
  }
}

function toPlain(blocks: UiBlock[]): BlockNode[] {
  return blocks.map((block) => ({
    kind: block.kind,
    fields: block.fields,
    body: toPlain(block.body),
    elseBody: toPlain(block.elseBody),
  }))
}

function fromPlain(nodes: BlockNode[]): UiBlock[] {
  return nodes.map((node) => ({
    id: nextBlockId++,
    kind: node.kind,
    fields: { ...node.fields },
    body: fromPlain(node.body),
    elseBody: fromPlain(node.elseBody),
  }))
}

function updateBlock(
  blocks: UiBlock[],
  id: number,
  update: (block: UiBlock) => UiBlock,
): UiBlock[] {
  return blocks.map((block) => {
    if (block.id === id) return update(block)
    return {
      ...block,
      body: updateBlock(block.body, id, update),
      elseBody: updateBlock(block.elseBody, id, update),
    }
  })
}

function removeBlock(blocks: UiBlock[], id: number): UiBlock[] {
  const out: UiBlock[] = []
  for (const block of blocks) {
    if (block.id === id) continue
    out.push({
      ...block,
      body: removeBlock(block.body, id),
      elseBody: removeBlock(block.elseBody, id),
    })
  }
  return out
}

function insertAfter(
  blocks: UiBlock[],
  id: number,
  block: UiBlock,
): { tree: UiBlock[]; done: boolean } {
  const index = blocks.findIndex((item) => item.id === id)
  if (index >= 0) {
    const tree = blocks.slice()
    tree.splice(index + 1, 0, block)
    return { tree, done: true }
  }
  for (let i = 0; i < blocks.length; i += 1) {
    const item = blocks[i]
    const inBody = insertAfter(item.body, id, block)
    if (inBody.done) {
      const tree = blocks.slice()
      tree[i] = { ...item, body: inBody.tree }
      return { tree, done: true }
    }
    const inElse = insertAfter(item.elseBody, id, block)
    if (inElse.done) {
      const tree = blocks.slice()
      tree[i] = { ...item, elseBody: inElse.tree }
      return { tree, done: true }
    }
  }
  return { tree: blocks, done: false }
}

function appendTo(
  blocks: UiBlock[],
  id: number,
  part: 'body' | 'elseBody',
  block: UiBlock,
): UiBlock[] {
  return updateBlock(blocks, id, (item) => ({ ...item, [part]: [...item[part], block] }))
}

function moveBlock(blocks: UiBlock[], id: number, direction: -1 | 1): UiBlock[] {
  const index = blocks.findIndex((item) => item.id === id)
  if (index >= 0) {
    const target = index + direction
    if (target < 0 || target >= blocks.length) return blocks
    const tree = blocks.slice()
    const [item] = tree.splice(index, 1)
    tree.splice(target, 0, item)
    return tree
  }
  for (let i = 0; i < blocks.length; i += 1) {
    const item = blocks[i]
    const body = moveBlock(item.body, id, direction)
    if (body !== item.body) {
      const tree = blocks.slice()
      tree[i] = { ...item, body }
      return tree
    }
    const elseBody = moveBlock(item.elseBody, id, direction)
    if (elseBody !== item.elseBody) {
      const tree = blocks.slice()
      tree[i] = { ...item, elseBody }
      return tree
    }
  }
  return blocks
}

function duplicateBlock(blocks: UiBlock[], id: number): UiBlock[] {
  const index = blocks.findIndex((item) => item.id === id)
  if (index >= 0) {
    const tree = blocks.slice()
    tree.splice(index + 1, 0, cloneBlock(tree[index]))
    return tree
  }
  for (let i = 0; i < blocks.length; i += 1) {
    const item = blocks[i]
    const body = duplicateBlock(item.body, id)
    if (body !== item.body) {
      const tree = blocks.slice()
      tree[i] = { ...item, body }
      return tree
    }
    const elseBody = duplicateBlock(item.elseBody, id)
    if (elseBody !== item.elseBody) {
      const tree = blocks.slice()
      tree[i] = { ...item, elseBody }
      return tree
    }
  }
  return blocks
}

function specFor(specs: Record<string, BlockSpec>, kind: string): BlockSpec {
  return (
    specs[kind] ?? {
      title: kind,
      category: 'Unknown',
      container: '',
      value: false,
      fields: [],
      defaults: {},
    }
  )
}

interface BlockCardProps {
  node: UiBlock
  specs: Record<string, BlockSpec>
  selectedId: number | null
  insertTarget: InsertTarget | null
  actions: BlockActions
}

function BlockCard({ node, specs, selectedId, insertTarget, actions }: BlockCardProps) {
  const spec = specFor(specs, node.kind)
  const color = CATEGORY_COLORS[spec.category] ?? FALLBACK_COLOR

  return (
    <div className={`block${selectedId === node.id ? ' selected' : ''}`}>
      <div
        className="block-header"
        style={{ background: color, color: BLOCK_TEXT }}
        onClick={() => actions.select(node.id)}
      >
        <span className="block-category">{spec.category}</span>
        <span className="block-title">{spec.title}</span>
        <span className="block-actions">
          <button
            type="button"
            className="block-action"
            title="Move up"
            onClick={(event) => {
              event.stopPropagation()
              actions.move(node.id, -1)
            }}
          >
            <Icon name="arrowUp" size={12} />
          </button>
          <button
            type="button"
            className="block-action"
            title="Move down"
            onClick={(event) => {
              event.stopPropagation()
              actions.move(node.id, 1)
            }}
          >
            <Icon name="arrowDown" size={12} />
          </button>
          <button
            type="button"
            className="block-action"
            title="Duplicate"
            onClick={(event) => {
              event.stopPropagation()
              actions.duplicate(node.id)
            }}
          >
            <Icon name="copy" size={12} />
          </button>
          <button
            type="button"
            className="block-action danger"
            title="Delete"
            onClick={(event) => {
              event.stopPropagation()
              actions.remove(node.id)
            }}
          >
            <Icon name="close" size={12} />
          </button>
        </span>
      </div>

      {spec.fields.length > 0 ? (
        <div className="block-fields">
          {spec.fields.map((field) => (
            <label key={field.key} className="block-field">
              {field.label ? <span className="block-field-label">{field.label}</span> : null}
              <input
                className={`block-input type-${field.type}`}
                style={{ width: `${field.width}px` }}
                value={node.fields[field.key] ?? ''}
                onChange={(event) => actions.setField(node.id, field.key, event.target.value)}
                onClick={(event) => event.stopPropagation()}
                spellCheck={false}
              />
            </label>
          ))}
        </div>
      ) : null}

      {spec.container ? (
        <div className="block-body">
          {node.body.map((child) => (
            <BlockCard
              key={child.id}
              node={child}
              specs={specs}
              selectedId={selectedId}
              insertTarget={insertTarget}
              actions={actions}
            />
          ))}
          <button
            type="button"
            className={`add-here${
              insertTarget?.id === node.id && insertTarget.part === 'body' ? ' active' : ''
            }`}
            onClick={(event) => {
              event.stopPropagation()
              actions.addHere(node.id, 'body')
            }}
          >
            <Icon name="plus" size={12} />
            <span>
              {insertTarget?.id === node.id && insertTarget.part === 'body'
                ? 'next block goes here'
                : 'add inside'}
            </span>
          </button>
        </div>
      ) : null}

      {spec.container === 'if' ? (
        <div className="block-else">
          <div className="block-else-label">wonga</div>
          {node.elseBody.map((child) => (
            <BlockCard
              key={child.id}
              node={child}
              specs={specs}
              selectedId={selectedId}
              insertTarget={insertTarget}
              actions={actions}
            />
          ))}
          <button
            type="button"
            className={`add-here${
              insertTarget?.id === node.id && insertTarget.part === 'elseBody' ? ' active' : ''
            }`}
            onClick={(event) => {
              event.stopPropagation()
              actions.addHere(node.id, 'elseBody')
            }}
          >
            <Icon name="plus" size={12} />
            <span>
              {insertTarget?.id === node.id && insertTarget.part === 'elseBody'
                ? 'next block goes here'
                : 'add else'}
            </span>
          </button>
        </div>
      ) : null}
    </div>
  )
}

export default function BlocksView({
  engine,
  engineStatus,
  activeDoc,
  onRunSource,
  onSendToText,
}: BlocksViewProps) {
  const [specs, setSpecs] = useState<BlockSpecs | null>(null)
  const [specsError, setSpecsError] = useState('')
  const [blocks, setBlocks] = useState<UiBlock[]>([])
  const [selectedId, setSelectedId] = useState<number | null>(null)
  const [insertTarget, setInsertTarget] = useState<InsertTarget | null>(null)
  const [problems, setProblems] = useState<string[]>([])
  const [message, setMessage] = useState('')
  const [messageKind, setMessageKind] = useState<'info' | 'warn'>('info')

  const blocksRef = useRef<UiBlock[]>(blocks)
  const historyRef = useRef<UiBlock[][]>([])

  useEffect(() => {
    if (!engine) return
    try {
      setSpecs(engine.blockSpecs())
      setSpecsError('')
    } catch (reason) {
      setSpecsError(reason instanceof Error ? reason.message : String(reason))
    }
  }, [engine])

  useEffect(() => {
    if (!engine || !specs) {
      setProblems([])
      return
    }
    try {
      const result = engine.blocksCheck(toPlain(blocks))
      setProblems(result.problems ?? [])
    } catch (reason) {
      setProblems([reason instanceof Error ? reason.message : String(reason)])
    }
  }, [engine, specs, blocks])

  const applyBlocks = useCallback(
    (updater: (current: UiBlock[]) => UiBlock[], record = true) => {
      const current = blocksRef.current
      const next = updater(current)
      if (next === current) return
      if (record) {
        historyRef.current = [...historyRef.current.slice(-49), current]
      }
      blocksRef.current = next
      setBlocks(next)
    },
    [],
  )

  const actions = useMemo<BlockActions>(
    () => ({
      select: (id) => {
        setSelectedId(id)
        setInsertTarget(null)
      },
      setField: (id, key, value) => {
        applyBlocks(
          (current) =>
            updateBlock(current, id, (block) => ({
              ...block,
              fields: { ...block.fields, [key]: value },
            })),
          false,
        )
      },
      move: (id, direction) => {
        applyBlocks((current) => moveBlock(current, id, direction))
      },
      duplicate: (id) => {
        applyBlocks((current) => duplicateBlock(current, id))
      },
      remove: (id) => {
        applyBlocks((current) => removeBlock(current, id))
        setSelectedId((current) => (current === id ? null : current))
        setInsertTarget((current) => (current?.id === id ? null : current))
      },
      addHere: (id, part) => {
        setSelectedId(id)
        setInsertTarget({ id, part })
      },
    }),
    [applyBlocks],
  )

  const addBlock = useCallback(
    (kind: string) => {
      const spec = specs?.specs[kind]
      if (!spec) return
      const block = createBlock(kind, spec)
      applyBlocks((current) => {
        if (insertTarget) {
          return appendTo(current, insertTarget.id, insertTarget.part, block)
        }
        if (selectedId !== null) {
          const result = insertAfter(current, selectedId, block)
          if (result.done) return result.tree
        }
        return [...current, block]
      })
      setSelectedId(block.id)
      setInsertTarget(null)
      setMessage('')
      setMessageKind('info')
    },
    [applyBlocks, insertTarget, selectedId, specs],
  )

  const palette = useMemo(() => {
    if (!specs) return []
    return specs.categories
      .map((category) => ({
        category,
        entries: specs.order
          .filter((kind) => specs.specs[kind] && specs.specs[kind].category === category)
          .map((kind) => ({ kind, spec: specs.specs[kind] })),
      }))
      .filter((group) => group.entries.length > 0)
  }, [specs])

  const handleUndo = useCallback(() => {
    const previous = historyRef.current.pop()
    if (!previous) {
      setMessage('Nothing to undo.')
      setMessageKind('warn')
      return
    }
    blocksRef.current = previous
    setBlocks(previous)
    setSelectedId(null)
    setInsertTarget(null)
    setMessage('Undid the last change.')
    setMessageKind('info')
  }, [])

  const handleClear = useCallback(() => {
    applyBlocks(() => [])
    setSelectedId(null)
    setInsertTarget(null)
    setMessage('Cleared the block workspace.')
    setMessageKind('info')
  }, [applyBlocks])

  const handleRun = useCallback(() => {
    if (!engine) return
    const result = engine.blocksToSource(toPlain(blocksRef.current))
    if (!result.ok) {
      setMessage(result.error || 'Could not turn the blocks into a program.')
      setMessageKind('warn')
      return
    }
    setMessage('Running blocks...')
    setMessageKind('info')
    onRunSource(result.source, 'blocks.cmc')
  }, [engine, onRunSource])

  const handleToText = useCallback(() => {
    if (!engine) return
    const result = engine.blocksToSource(toPlain(blocksRef.current))
    if (!result.ok) {
      setMessage(result.error || 'Could not turn the blocks into a program.')
      setMessageKind('warn')
      return
    }
    onSendToText(result.source)
    setMessage('Sent the blocks to the active text file.')
    setMessageKind('info')
  }, [engine, onSendToText])

  const handleFromText = useCallback(() => {
    if (!engine) return
    if (!activeDoc) {
      setMessage('No text file is open to load from.')
      setMessageKind('warn')
      return
    }
    const result = engine.blocksFromSource(activeDoc.source)
    if (!result.ok) {
      setMessage(result.error || 'Could not load blocks from the text.')
      setMessageKind('warn')
      return
    }
    const converted = fromPlain(result.blocks)
    historyRef.current = [...historyRef.current.slice(-49), blocksRef.current]
    blocksRef.current = converted
    setBlocks(converted)
    setSelectedId(null)
    setInsertTarget(null)
    setMessage(
      `Loaded ${converted.length} block${converted.length === 1 ? '' : 's'} from ${activeDoc.file}.`,
    )
    setMessageKind('info')
  }, [engine, activeDoc])

  if (engineStatus !== 'ready' || !engine) {
    return (
      <div className="view-empty">
        <span className="view-empty-icon">
          <Icon name="blocks" size={44} />
        </span>
        <h2>Blocks need the engine</h2>
        <p>
          The CMC engine is {engineStatus === 'loading' ? 'still loading' : 'not built yet'}. The
          block editor turns blocks into text with the same engine, so it will wake up as soon as
          the engine is ready.
        </p>
      </div>
    )
  }

  if (specsError) {
    return (
      <div className="view-empty">
        <span className="view-empty-icon">
          <Icon name="warning" size={44} />
        </span>
        <h2>Could not load blocks</h2>
        <p>{specsError}</p>
      </div>
    )
  }

  const problemText = problems.length > 0 ? problems.join('  ') : ''

  return (
    <div className="blocks">
      <div className="blocks-toolbar">
        <button type="button" className="btn primary" onClick={handleRun}>
          <Icon name="play" size={13} />
          <span>Run blocks</span>
        </button>
        <button type="button" className="btn" onClick={handleToText} title="Write blocks as text">
          <Icon name="send" size={13} />
          <span>Send to Text</span>
        </button>
        <button
          type="button"
          className="btn"
          onClick={handleFromText}
          title="Load the active text file as blocks"
        >
          <Icon name="import" size={13} />
          <span>Load from Text</span>
        </button>
        <span className="toolbar-divider" />
        <button type="button" className="btn" onClick={handleUndo}>
          <Icon name="undo" size={13} />
          <span>Undo</span>
        </button>
        <button type="button" className="btn" onClick={handleClear}>
          <Icon name="trash" size={13} />
          <span>Clear</span>
        </button>
        <span className="toolbar-spacer" />
        <span className="blocks-hint">
          Pick a block on the left to add it. Select a block to add after it.
        </span>
      </div>

      <div className="blocks-main">
        <div className="blocks-palette">
          {specs ? (
            palette.map((group) => (
              <div key={group.category} className="palette-group">
                <div
                  className="palette-group-title"
                  style={{ color: CATEGORY_COLORS[group.category] ?? FALLBACK_COLOR }}
                >
                  {group.category}
                </div>
                {group.entries.map(({ kind, spec }) => (
                  <button
                    key={kind}
                    type="button"
                    className="palette-block"
                    style={{
                      background: CATEGORY_COLORS[spec.category] ?? FALLBACK_COLOR,
                      color: BLOCK_TEXT,
                    }}
                    onClick={() => addBlock(kind)}
                    title={spec.title}
                  >
                    {spec.title}
                  </button>
                ))}
              </div>
            ))
          ) : (
            <p className="explorer-empty">Loading blocks...</p>
          )}
        </div>

        <div className="blocks-tree" onClick={() => setInsertTarget(null)}>
          {blocks.length === 0 ? (
            <div className="blocks-empty">
              <p>The workspace is empty.</p>
              <p>Click a block from the palette to start building.</p>
            </div>
          ) : (
            blocks.map((block) => (
              <BlockCard
                key={block.id}
                node={block}
                specs={specs?.specs ?? {}}
                selectedId={selectedId}
                insertTarget={insertTarget}
                actions={actions}
              />
            ))
          )}
        </div>
      </div>

      <div className={`blocks-status${problemText || messageKind === 'warn' ? ' warn' : ''}`}>
        {problemText ? (
          <>
            <Icon name="warning" size={13} />
            <span>{problemText}</span>
          </>
        ) : message ? (
          <span>{message}</span>
        ) : (
          <span>
            Blocks look good. {blocks.length} top-level block{blocks.length === 1 ? '' : 's'}.
          </span>
        )}
      </div>
    </div>
  )
}
