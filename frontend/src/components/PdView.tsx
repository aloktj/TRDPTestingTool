import { useEffect, useMemo, useState } from 'react'
import '../App.css'

type RawTelegram = Record<string, unknown>

type TelegramRow = {
  id: string
  interfaceName: string
  comId: string
  name: string
  direction: string
  datasetId: string
  lastRxTime: string
  status: 'OK' | 'Timeout' | string
  decodedFields: DecodedField[]
  rawHex: string
}

type DecodedField = {
  name: string
  type: string
  value: string
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

const normalizeDecodedField = (field: unknown): DecodedField | null => {
  if (!field || typeof field !== 'object') return null

  const record = field as Record<string, unknown>

  const name =
    toStringValue(record.field_name) || toStringValue(record.name) || toStringValue(record.label)
  if (!name) return null

  const type =
    toStringValue(record.type) ||
    toStringValue(record.field_type) ||
    toStringValue(record.data_type) ||
    '—'

  const value =
    record.value ?? record.val ?? record.data ?? record.raw_value ?? record.display_value ?? '—'

  return {
    name,
    type,
    value: toStringValue(value) || '—',
  }
}

const normalizeDecodedFields = (fields: unknown): DecodedField[] => {
  if (!Array.isArray(fields)) return []

  return fields
    .map((field) => normalizeDecodedField(field))
    .filter((field): field is DecodedField => Boolean(field))
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

  const datasetId =
    (telegram.datasetId as string | number | undefined) ||
    (telegram.datasetID as string | number | undefined) ||
    (telegram.dataset_id as string | number | undefined) ||
    (telegram.dataset as string | number | undefined) ||
    '—'

  const lastRx = (telegram.last_rx as RawTelegram | undefined) || (telegram.lastRx as RawTelegram | undefined)

  const decodedFields = normalizeDecodedFields(
    (lastRx?.decoded_fields as unknown) || (lastRx?.decodedFields as unknown)
  )

  const rawHex =
    toStringValue((lastRx?.raw_hex as string | undefined) || (lastRx?.rawHex as string | undefined)) || '—'

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
    datasetId: String(datasetId),
    lastRxTime,
    status,
    decodedFields,
    rawHex,
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

      {selectedTelegram ? (
        <div className="pd-detail">
          <div className="pd-detail__header">
            <div>
              <p className="eyebrow">Telegram details</p>
              <h3 className="pd-detail__title">{selectedTelegram.name}</h3>
              <div className="pd-detail__meta">
                <span>ComID {selectedTelegram.comId}</span>
                <span aria-hidden="true">·</span>
                <span>Dataset {selectedTelegram.datasetId}</span>
                <span aria-hidden="true">·</span>
                <span>{selectedTelegram.direction}</span>
              </div>
            </div>
          </div>

          <div className="pd-detail__content">
            <div className="pd-detail__section">
              <div className="pd-detail__section-header">
                <h4>Decoded fields</h4>
                <p className="subtitle">Values parsed from the last received telegram.</p>
              </div>
              <div className="pd-detail__table-wrapper">
                <table className="pd-detail__table">
                  <thead>
                    <tr>
                      <th>Field Name</th>
                      <th>Type</th>
                      <th>Value</th>
                    </tr>
                  </thead>
                  <tbody>
                    {selectedTelegram.decodedFields.length === 0 ? (
                      <tr>
                        <td colSpan={3} className="pd-detail__empty">
                          No decoded fields available.
                        </td>
                      </tr>
                    ) : (
                      selectedTelegram.decodedFields.map((field) => (
                        <tr key={`${selectedTelegram.id}-${field.name}`}>
                          <td>{field.name}</td>
                          <td>{field.type}</td>
                          <td>{field.value}</td>
                        </tr>
                      ))
                    )}
                  </tbody>
                </table>
              </div>
            </div>

            <div className="pd-detail__section">
              <div className="pd-detail__section-header">
                <h4>Raw payload</h4>
                <p className="subtitle">Hex-encoded data received with the last telegram.</p>
              </div>
              <pre className="code-block">{selectedTelegram.rawHex}</pre>
            </div>
          </div>
        </div>
      ) : null}
    </div>
  )
}

export default PdView
