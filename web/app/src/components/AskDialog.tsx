import { useState } from 'react'
import type { FormEvent } from 'react'

interface AskDialogProps {
  prompt: string
  onSubmit: (answer: string) => void
}

export default function AskDialog({ prompt, onSubmit }: AskDialogProps) {
  const [answer, setAnswer] = useState('')

  const handleSubmit = (event: FormEvent) => {
    event.preventDefault()
    onSubmit(answer)
  }

  return (
    <div className="modal-overlay">
      <form className="modal" onSubmit={handleSubmit}>
        <h2 className="modal-title">The program asks</h2>
        <p className="modal-prompt">{prompt || 'Type an answer'}</p>
        <input
          className="modal-input"
          value={answer}
          onChange={(event) => setAnswer(event.target.value)}
          autoFocus
          spellCheck={false}
        />
        <div className="modal-actions">
          <button type="button" className="btn" onClick={() => onSubmit('')}>
            Cancel
          </button>
          <button type="submit" className="btn primary">
            Answer
          </button>
        </div>
      </form>
    </div>
  )
}
