import { useEffect, useMemo, useState } from 'react'
import '../App.css'

type RawTelegram = Record<string, unknown>

type TelegramRow = {
  id: string
  interfaceName: string
  comId: string
  name: string
  direction: string
  lastRxTime: string
  status: 'OK' | 'Timeout' | string
}

const toStringValue = (value: unknown): string => {
  if (value === null || value === undefined) return ''
  if (typeof value === 'string') return value
  return String(value)
}

const normalizeStatus = (value: unknown): 'OK' | 'Timeout' | string => {
  const normalized = toStringValue(value).toLowerCase()

  if (normalized === 'timeout' || normalized === 'timedout') return 'Timeout'
  if (normalized === 'ok' || normalized === 'healthy') return 'OK'

  if (typeof value === 'boolean') {
    return value ? 'OK' : 'Timeout'
  }

  return normalized ? normalized : 'OK'
}

const normalizeDirection = (value: unknown): string => {
  const normalized = toStringValue(value)
  if (!normalized) return 'Unknown'
  const lower = normalized.toLowerCase()

  if (lower === 'tx' || lower === 'outgoing' || lower === 'publisher') return 'Outgoing'
  if (lower === 'rx' || lower === 'incoming' || lower === 'subscriber') return 'Incoming'
  if (lower === 'loopback') return 'Loopback'

  return normalized
}

const formatTimestamp = (value: unknown): string => {
  if (value === null || value === undefined) return '—'

  const asNumber = typeof value === 'number' ? value : Number.NaN
  const stringValue = toStringValue(value)

  const date = Number.isNaN(asNumber) ? new Date(stringValue) : new Date(asNumber)
  if (Number.isNaN(date.getTime())) {
    return stringValue || '—'
  }

  return date.toLocaleString()
}

const buildRowId = (telegram: RawTelegram, fallbackIndex: number): string => {
  const interfaceName =
    (telegram.interface as string | undefined) ||
    (telegram.interfaceName as string | undefined) ||
    (telegram.ifName as string | undefined) ||
    'iface'

  const comId =
    (telegram.comId as string | number | undefined) ||
    (telegram.comID as string | number | undefined) ||
    (telegram.id as string | number | undefined) ||
    fallbackIndex

  return `${interfaceName}-${comId}`
}

const normalizeTelegram = (telegram: RawTelegram, index: number): TelegramRow => {
  const interfaceName =
    (telegram.interface as string | undefined) ||
    (telegram.interfaceName as string | undefined) ||
    (telegram.ifName as string | undefined) ||
    '—'

  const comId =
    (telegram.comId as string | number | undefined) ||
    (telegram.comID as string | number | undefined) ||
    (telegram.id as string | number | undefined) ||
    '—'

  const name = (telegram.name as string | undefined) || (telegram.title as string | undefined) || '—'

  const direction = normalizeDirection(
    (telegram.direction as string | undefined) ||
      (telegram.role as string | undefined) ||
      (telegram.flow as string | undefined)
  )

  const lastRxTime = formatTimestamp(
    telegram.lastRxTime ||
      telegram.lastReceived ||
      telegram.lastRxTimestamp ||
      telegram.lastUpdated ||
      telegram.updatedAt
  )

  const status = normalizeStatus(telegram.status ?? telegram.state ?? telegram.healthy)

  return {
    id: buildRowId(telegram, index),
    interfaceName,
    comId: String(comId),
    name,
    direction,
    lastRxTime,
    status,
  }
}

export function PdView() {
  const [telegrams, setTelegrams] = useState<TelegramRow[]>([])
  const [selectedId, setSelectedId] = useState<string | null>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string>('')

  useEffect(() => {
    let mounted = true

    const fetchTelegrams = async () => {
      try {
        const response = await fetch('/api/pd/telegrams')
        if (!response.ok) {
          throw new Error(`Request failed with status ${response.status}`)
        }

        const payload = await response.json()
        if (!mounted) return

        const normalized = Array.isArray(payload)
          ? payload.map((item, index) => normalizeTelegram(item as RawTelegram, index))
          : []

        setTelegrams(normalized)
        setSelectedId((prev) => {
          if (prev && normalized.some((row) => row.id === prev)) return prev
          return normalized[0]?.id ?? null
        })
        setError('')
      } catch (fetchError) {
        if (!mounted) return
        setError(fetchError instanceof Error ? fetchError.message : 'Unable to load telegrams')
      } finally {
        if (mounted) {
          setLoading(false)
        }
      }
    }

    fetchTelegrams()

    return () => {
      mounted = false
    }
  }, [])

  const selectedTelegram = useMemo(
    () => telegrams.find((row) => row.id === selectedId),
    [selectedId, telegrams]
  )

  return (
    <div className="panel pd-view">
      <div className="pd-view__header">
        <div>
          <p className="eyebrow">Process Data</p>
          <h2>PD telegrams</h2>
          <p className="subtitle">
            Browse telegrams configured on each interface and select one to inspect its runtime status.
          </p>
        </div>
        {selectedTelegram ? (
          <div className="pd-view__selection">
            <p className="eyebrow">Selected telegram</p>
            <div className="pd-view__selection-name">{selectedTelegram.name}</div>
            <div className="pd-view__selection-meta">
              ComID {selectedTelegram.comId} · {selectedTelegram.direction}
            </div>
          </div>
        ) : null}
      </div>

      {error ? <div className="alert alert--error">{error}</div> : null}
      {loading ? <div className="alert">Loading telegrams…</div> : null}

      <div className="pd-table-wrapper">
        <table className="pd-table">
          <thead>
            <tr>
              <th>Interface</th>
              <th>ComID</th>
              <th>Name</th>
              <th>Direction</th>
              <th>Last Rx Time</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {telegrams.length === 0 && !loading ? (
              <tr>
                <td colSpan={6} className="pd-table__empty">
                  No telegrams available.
                </td>
              </tr>
            ) : null}
            {telegrams.map((telegram) => {
              const isSelected = telegram.id === selectedId
              const statusText = normalizeStatus(telegram.status)
              const isTimeout = statusText.toLowerCase() === 'timeout'

              return (
                <tr
                  key={telegram.id}
                  className={isSelected ? 'is-selected' : undefined}
                  onClick={() => setSelectedId(telegram.id)}
                >
                  <td>{telegram.interfaceName}</td>
                  <td>{telegram.comId}</td>
                  <td>{telegram.name}</td>
                  <td>
                    <span className="pill pill--muted">{telegram.direction}</span>
                  </td>
                  <td>{telegram.lastRxTime}</td>
                  <td>
                    <span className={`pill ${isTimeout ? 'pill--danger' : 'pill--success'}`}>
                      {isTimeout ? 'Timeout' : 'OK'}
                    </span>
                  </td>
                </tr>
              )
            })}
          </tbody>
        </table>
      </div>
    </div>
  )
}

export default PdView
