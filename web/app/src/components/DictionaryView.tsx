import { useEffect, useMemo, useState } from 'react'
import type { Cmc, WordInfo } from '../wasm/cmc'
import type { EngineStatus } from '../types'
import Icon from './Icon'

interface DictionaryViewProps {
  engine: Cmc | null
  engineStatus: EngineStatus
}

export default function DictionaryView({ engine, engineStatus }: DictionaryViewProps) {
  const [words, setWords] = useState<WordInfo[] | null>(null)
  const [error, setError] = useState('')
  const [query, setQuery] = useState('')
  const [selectedName, setSelectedName] = useState<string | null>(null)

  useEffect(() => {
    if (!engine) return
    try {
      const entries = engine.dictionary()
      setWords(entries)
      setError('')
      if (entries.length > 0) setSelectedName(entries[0].name)
    } catch (reason) {
      setError(reason instanceof Error ? reason.message : String(reason))
    }
  }, [engine])

  const visible = useMemo(() => {
    if (!words) return []
    const needle = query.trim().toLowerCase()
    if (!needle) return words
    return words.filter((word) =>
      [word.name, word.display, word.category, word.summary, ...word.aliases]
        .join(' ')
        .toLowerCase()
        .includes(needle),
    )
  }, [words, query])

  const selected = useMemo(() => {
    if (visible.length === 0) return null
    return visible.find((word) => word.name === selectedName) ?? visible[0]
  }, [visible, selectedName])

  if (engineStatus !== 'ready' || !engine) {
    return (
      <div className="view-empty">
        <span className="view-empty-icon">
          <Icon name="book" size={44} />
        </span>
        <h2>Dictionary needs the engine</h2>
        <p>
          The word list comes straight from the CMC engine, which is{' '}
          {engineStatus === 'loading' ? 'still loading' : 'not built yet'}.
        </p>
      </div>
    )
  }

  if (error) {
    return (
      <div className="view-empty">
        <span className="view-empty-icon">
          <Icon name="warning" size={44} />
        </span>
        <h2>Could not load the dictionary</h2>
        <p>{error}</p>
      </div>
    )
  }

  return (
    <div className="dict">
      <div className="dict-list">
        <label className="dict-search">
          <Icon name="search" size={14} />
          <input
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Search words"
            spellCheck={false}
          />
        </label>
        <div className="dict-items">
          {words === null ? (
            <p className="explorer-empty">Loading words...</p>
          ) : visible.length === 0 ? (
            <p className="explorer-empty">No word matches that.</p>
          ) : (
            visible.map((word) => (
              <button
                key={word.name}
                type="button"
                className={`dict-item${selected?.name === word.name ? ' active' : ''}`}
                onClick={() => setSelectedName(word.name)}
              >
                <span className="dict-item-name">{word.display || word.name}</span>
                <span className="dict-item-category">{word.category}</span>
                <span className="dict-item-summary">{word.summary}</span>
              </button>
            ))
          )}
        </div>
      </div>

      <div className="dict-detail">
        {selected ? (
          <article className="dict-article">
            <header className="dict-header">
              <h1>{selected.display || selected.name}</h1>
              {selected.display && selected.display !== selected.name ? (
                <span className="dict-aka">also called {selected.name}</span>
              ) : null}
              <span className="dict-badge">{selected.category}</span>
            </header>

            <p className="dict-summary">{selected.summary}</p>

            {selected.aliases.length > 0 ? (
              <section className="dict-section">
                <h2>Aliases</h2>
                <div className="chip-row">
                  {selected.aliases.map((alias) => (
                    <span key={alias} className="chip">
                      {alias}
                    </span>
                  ))}
                </div>
              </section>
            ) : null}

            {selected.syntax ? (
              <section className="dict-section">
                <h2>Syntax</h2>
                <pre className="dict-code">{selected.syntax}</pre>
                {selected.extraSyntax.map((extra) => (
                  <pre key={extra} className="dict-code">
                    {extra}
                  </pre>
                ))}
              </section>
            ) : null}

            {selected.doc ? (
              <section className="dict-section">
                <h2>What it does</h2>
                <p className="dict-doc">{selected.doc}</p>
              </section>
            ) : null}

            {selected.example ? (
              <section className="dict-section">
                <h2>Example</h2>
                <pre className="dict-code example">{selected.example}</pre>
              </section>
            ) : null}

            {selected.tips.length > 0 ? (
              <section className="dict-section">
                <h2>Tips</h2>
                <ul className="dict-tips">
                  {selected.tips.map((tip) => (
                    <li key={tip}>{tip}</li>
                  ))}
                </ul>
              </section>
            ) : null}

            {selected.related.length > 0 ? (
              <section className="dict-section">
                <h2>See also</h2>
                <div className="chip-row">
                  {selected.related.map((related) => (
                    <button
                      key={related}
                      type="button"
                      className="chip clickable"
                      onClick={() => {
                        setSelectedName(related)
                        setQuery('')
                      }}
                    >
                      {related}
                    </button>
                  ))}
                </div>
              </section>
            ) : null}
          </article>
        ) : (
          <div className="dict-placeholder">Pick a word to read about it.</div>
        )}
      </div>
    </div>
  )
}
