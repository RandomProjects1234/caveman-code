import { useEffect, useRef } from 'react'
import type { DrawOp } from '../types'
import { parseCaveColor } from '../drawColors'
import Icon from './Icon'

interface DrawViewProps {
  ops: DrawOp[]
  onClear: () => void
}

const BASE_WIDTH = 500
const BASE_HEIGHT = 400

export default function DrawView({ ops, onClear }: DrawViewProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const context = canvas.getContext('2d')
    if (!context) return

    const prepare = (width: number, height: number) => {
      canvas.width = Math.max(1, Math.round(width))
      canvas.height = Math.max(1, Math.round(height))
      context.fillStyle = '#ffffff'
      context.fillRect(0, 0, canvas.width, canvas.height)
      context.lineCap = 'round'
      context.lineJoin = 'round'
      context.textBaseline = 'top'
    }

    prepare(BASE_WIDTH, BASE_HEIGHT)

    let color = 'rgb(0, 0, 0)'
    for (const op of ops) {
      switch (op.op) {
        case 'clear':
          context.fillStyle = '#ffffff'
          context.fillRect(0, 0, canvas.width, canvas.height)
          break
        case 'size':
          prepare(op.a, op.b)
          break
        case 'color':
          color = parseCaveColor(op.text)
          break
        case 'dot':
          context.fillStyle = color
          context.beginPath()
          context.arc(op.a, op.b, Math.abs(op.c), 0, Math.PI * 2)
          context.fill()
          break
        case 'circle':
          context.strokeStyle = color
          context.lineWidth = 2
          context.beginPath()
          context.arc(op.a, op.b, Math.abs(op.c), 0, Math.PI * 2)
          context.stroke()
          break
        case 'line':
          context.strokeStyle = color
          context.lineWidth = 2
          context.beginPath()
          context.moveTo(op.a, op.b)
          context.lineTo(op.c, op.d)
          context.stroke()
          break
        case 'box':
          context.strokeStyle = color
          context.lineWidth = 2
          context.strokeRect(op.a, op.b, op.c, op.d)
          break
        case 'blob':
          context.fillStyle = color
          context.fillRect(op.a, op.b, op.c, op.d)
          break
        case 'write':
          context.fillStyle = color
          context.font = '14px "Segoe UI", sans-serif'
          context.fillText(op.text, op.a, op.b)
          break
        default:
          break
      }
    }
  }, [ops])

  return (
    <div className="draw">
      <div className="draw-toolbar">
        <span className="draw-title">
          <Icon name="draw" size={14} />
          <span>Draw</span>
        </span>
        <span className="draw-count">
          {ops.length} drawing step{ops.length === 1 ? '' : 's'}
        </span>
        <span className="toolbar-spacer" />
        <button type="button" className="btn" onClick={onClear}>
          <Icon name="trash" size={13} />
          <span>Clear canvas</span>
        </button>
      </div>
      <div className="draw-stage">
        <div className="draw-frame">
          <canvas ref={canvasRef} width={BASE_WIDTH} height={BASE_HEIGHT} className="draw-canvas" />
          {ops.length === 0 ? (
            <div className="draw-overlay">
              Run a program that uses <code>skrib</code> to paint here.
            </div>
          ) : null}
        </div>
      </div>
    </div>
  )
}
