import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import type { ChangeEvent } from 'react'
import '../App.css'

type ConfigRow = {
  id: string
  filename: string
  uploadedAt: string
  active?: boolean
}

type InterfaceSummary = {
  name: string
  networkId?: number
  hostIp: string
  pdTelegrams: Array<{ name: string; comId?: number; direction: string }>
  mdTelegrams: Array<{ name: string; comId?: number; direction: string }>
}

type DatasetSummary = {
  id?: number
  name: string
  elementCount: number
}

type ConfigSummary = {
  device: { hostName: string; type: string }
  interfaces: InterfaceSummary[]
  datasets: DatasetSummary[]
}

const formatTimestamp = (value: string) => {
  const parsed = new Date(value)
  return Number.isNaN(parsed.getTime()) ? value : parsed.toLocaleString()
}

function SummaryModal({
  summary,
  onClose,
  loading,
  error,
  filename,
}: {
  summary: ConfigSummary | null
  onClose: () => void
  loading: boolean
  error: string
  filename: string | null
}) {
  return (
    <div className="modal" role="dialog" aria-modal="true" aria-label="Configuration summary">
      <div className="modal__dialog">
        <div className="modal__header">
          <div>
            <p className="eyebrow">Configuration summary</p>
            <h2 className="modal__title">{filename ?? 'Configuration'}</h2>
          </div>
          <button className="button button--muted" type="button" onClick={onClose} aria-label="Close dialog">
            Close
          </button>
        </div>

        {loading && <p className="subtitle">Loading summary…</p>}
        {error && <div className="alert alert--error">{error}</div>}

        {!loading && summary && !error && (
          <div className="modal__content">
            <div className="summary-grid">
              <div className="panel">
                <h3>Device</h3>
                <p>
                  <span className="highlight">Host:</span> {summary.device.hostName || '—'}
                </p>
                <p>
                  <span className="highlight">Type:</span> {summary.device.type || '—'}
                </p>
              </div>

              <div className="panel">
                <h3>Interfaces ({summary.interfaces.length})</h3>
                <ul className="summary-list">
                  {summary.interfaces.map((iface) => (
                    <li key={`${iface.name}-${iface.hostIp}`}>
                      <div className="summary-list__title">{iface.name || 'Unnamed interface'}</div>
                      <div className="pill-row">
                        <span className="pill">Network ID: {iface.networkId ?? '—'}</span>
                        <span className="pill">Host: {iface.hostIp || '—'}</span>
                      </div>
                      <div className="summary-columns">
                        <div>
                          <div className="pill pill--muted">PD telegrams</div>
                          <ul className="summary-sublist">
                            {iface.pdTelegrams.length === 0 && <li>None</li>}
                            {iface.pdTelegrams.map((telegram) => (
                              <li key={`pd-${telegram.name}-${telegram.comId}`}>
                                <span className="highlight">{telegram.name || 'Telegram'}</span> (ComId:{' '}
                                {telegram.comId ?? '—'}) — {telegram.direction}
                              </li>
                            ))}
                          </ul>
                        </div>
                        <div>
                          <div className="pill pill--muted">MD telegrams</div>
                          <ul className="summary-sublist">
                            {iface.mdTelegrams.length === 0 && <li>None</li>}
                            {iface.mdTelegrams.map((telegram) => (
                              <li key={`md-${telegram.name}-${telegram.comId}`}>
                                <span className="highlight">{telegram.name || 'Telegram'}</span> (ComId:{' '}
                                {telegram.comId ?? '—'}) — {telegram.direction}
                              </li>
                            ))}
                          </ul>
                        </div>
                      </div>
                    </li>
                  ))}
                </ul>
              </div>

              <div className="panel">
                <h3>Datasets ({summary.datasets.length})</h3>
                <ul className="summary-list">
                  {summary.datasets.length === 0 && <li>No datasets found.</li>}
                  {summary.datasets.map((dataset) => (
                    <li key={`${dataset.id}-${dataset.name}`} className="summary-list__row">
                      <div>
                        <div className="summary-list__title">{dataset.name || 'Dataset'}</div>
                        <div className="subtitle">Dataset ID: {dataset.id ?? '—'}</div>
                      </div>
                      <span className="pill">Elements: {dataset.elementCount}</span>
                    </li>
                  ))}
                </ul>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}

function ConfigurationsView() {
  const [configs, setConfigs] = useState<ConfigRow[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')
  const [uploading, setUploading] = useState(false)
  const [uploadError, setUploadError] = useState('')
  const [successMessage, setSuccessMessage] = useState('')
  const [summary, setSummary] = useState<ConfigSummary | null>(null)
  const [summaryError, setSummaryError] = useState('')
  const [summaryLoading, setSummaryLoading] = useState(false)
  const [modalOpen, setModalOpen] = useState(false)
  const [selectedConfig, setSelectedConfig] = useState<ConfigRow | null>(null)
  const [activatingId, setActivatingId] = useState<string | null>(null)
  const fileInputRef = useRef<HTMLInputElement | null>(null)

  const fetchConfigs = useCallback(async () => {
    setLoading(true)
    setError('')
    try {
      const response = await fetch('/api/configs')
      if (!response.ok) {
        throw new Error(`Request failed with status ${response.status}`)
      }
      const payload = await response.json()
      setConfigs(Array.isArray(payload) ? payload : [])
    } catch (fetchError) {
      setError(fetchError instanceof Error ? fetchError.message : 'Unable to load configurations.')
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    fetchConfigs()
  }, [fetchConfigs])

  const handleUploadClick = () => {
    fileInputRef.current?.click()
  }

  const handleFileChange = async (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0]
    if (!file) return

    setUploading(true)
    setUploadError('')
    setSuccessMessage('')

    const formData = new FormData()
    formData.append('file', file)

    try {
      const response = await fetch('/api/configs/upload', {
        method: 'POST',
        body: formData,
      })

      if (!response.ok) {
        const errorBody = await response.json().catch(() => ({}))
        throw new Error(errorBody.message || `Upload failed with status ${response.status}`)
      }

      await fetchConfigs()
      setSuccessMessage(`${file.name} uploaded successfully.`)
    } catch (uploadErr) {
      setUploadError(uploadErr instanceof Error ? uploadErr.message : 'Upload failed.')
    } finally {
      setUploading(false)
      event.target.value = ''
    }
  }

  const handleViewSummary = async (config: ConfigRow) => {
    setModalOpen(true)
    setSelectedConfig(config)
    setSummary(null)
    setSummaryError('')
    setSummaryLoading(true)

    try {
      const response = await fetch(`/api/configs/${config.id}/summary`)
      if (!response.ok) {
        throw new Error(`Summary request failed with status ${response.status}`)
      }
      const payload = (await response.json()) as ConfigSummary
      setSummary(payload)
    } catch (summaryErr) {
      setSummaryError(
        summaryErr instanceof Error ? summaryErr.message : 'Unable to load configuration summary.'
      )
    } finally {
      setSummaryLoading(false)
    }
  }

  const handleActivate = async (config: ConfigRow) => {
    setActivatingId(config.id)
    setError('')
    setSuccessMessage('')
    try {
      const response = await fetch(`/api/configs/${config.id}/activate`, { method: 'POST' })
      if (!response.ok) {
        const errorBody = await response.json().catch(() => ({}))
        throw new Error(errorBody.message || `Activation failed with status ${response.status}`)
      }

      await fetchConfigs()
      setSuccessMessage(`${config.filename} is now active.`)
    } catch (activationErr) {
      setError(
        activationErr instanceof Error ? activationErr.message : 'Unable to activate configuration.'
      )
    } finally {
      setActivatingId(null)
    }
  }

  const activeConfigId = useMemo(() => configs.find((item) => item.active)?.id ?? null, [configs])

  return (
    <div className="panel">
      <div className="configurations__header">
        <div>
          <p className="eyebrow">Configuration management</p>
          <h2>Configurations</h2>
          <p className="subtitle">
            Upload TRDP XML configuration files, review their summaries, and activate the one you want the
            engine to run.
          </p>
        </div>
        <div className="actions">
          <input
            ref={fileInputRef}
            type="file"
            accept=".xml,text/xml,application/xml"
            className="visually-hidden"
            onChange={handleFileChange}
          />
          <button
            className="button button--primary"
            type="button"
            onClick={handleUploadClick}
            disabled={uploading}
          >
            {uploading ? 'Uploading…' : 'Upload configuration'}
          </button>
        </div>
      </div>

      {uploadError && <div className="alert alert--error">{uploadError}</div>}
      {error && !uploadError && <div className="alert alert--error">{error}</div>}
      {successMessage && <div className="alert">{successMessage}</div>}
      {activeConfigId && !error && !uploadError && (
        <div className="pill pill--success">Active configuration ID: {activeConfigId}</div>
      )}

      <div className="table-wrapper">
        <table className="config-table">
          <thead>
            <tr>
              <th>Name</th>
              <th>Uploaded</th>
              <th>Status</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {loading && (
              <tr>
                <td colSpan={4} className="muted">
                  Loading configurations…
                </td>
              </tr>
            )}
            {!loading && configs.length === 0 && (
              <tr>
                <td colSpan={4} className="muted">
                  No configurations uploaded yet.
                </td>
              </tr>
            )}
            {!loading &&
              configs.map((config) => (
                <tr key={config.id}>
                  <td>
                    <div className="config-name">{config.filename}</div>
                    <div className="muted">ID: {config.id}</div>
                  </td>
                  <td>{formatTimestamp(config.uploadedAt)}</td>
                  <td>
                    {config.active ? (
                      <span className="pill pill--success">Active</span>
                    ) : (
                      <span className="pill pill--muted">Inactive</span>
                    )}
                  </td>
                  <td className="actions">
                    <button
                      className="button button--muted"
                      type="button"
                      onClick={() => handleViewSummary(config)}
                    >
                      View details
                    </button>
                    <button
                      className="button button--success"
                      type="button"
                      onClick={() => handleActivate(config)}
                      disabled={activatingId === config.id}
                    >
                      {activatingId === config.id
                        ? 'Activating…'
                        : config.active
                          ? 'Active'
                          : 'Activate'}
                    </button>
                  </td>
                </tr>
              ))}
          </tbody>
        </table>
      </div>

      {modalOpen && (
        <SummaryModal
          summary={summary}
          onClose={() => setModalOpen(false)}
          loading={summaryLoading}
          error={summaryError}
          filename={selectedConfig?.filename ?? null}
        />
      )}
    </div>
  )
}

export default ConfigurationsView
