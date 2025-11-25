export type RawTelegram = Record<string, unknown>

export type TelegramRow = {
  id: string
  interfaceName: string
  comId: string
  name: string
  direction: string
  lastRxTime: string
  status: 'OK' | 'Timeout' | string
}

export const toStringValue = (value: unknown): string => {
  if (value === null || value === undefined) return ''
  if (typeof value === 'string') return value
  return String(value)
}

export const normalizeStatus = (value: unknown): 'OK' | 'Timeout' | string => {
  const normalized = toStringValue(value).toLowerCase()

  if (normalized === 'timeout' || normalized === 'timedout') return 'Timeout'
  if (normalized === 'ok' || normalized === 'healthy') return 'OK'

  if (typeof value === 'boolean') {
    return value ? 'OK' : 'Timeout'
  }

  return normalized ? normalized : 'OK'
}

export const normalizeDirection = (value: unknown): string => {
  const normalized = toStringValue(value)
  if (!normalized) return 'Unknown'
  const lower = normalized.toLowerCase()

  if (lower === 'tx' || lower === 'outgoing' || lower === 'publisher') return 'Outgoing'
  if (lower === 'rx' || lower === 'incoming' || lower === 'subscriber') return 'Incoming'
  if (lower === 'loopback') return 'Loopback'

  return normalized
}

export const formatTimestamp = (value: unknown): string => {
  if (value === null || value === undefined) return '—'

  const asNumber = typeof value === 'number' ? value : Number.NaN
  const stringValue = toStringValue(value)

  const date = Number.isNaN(asNumber) ? new Date(stringValue) : new Date(asNumber)
  if (Number.isNaN(date.getTime())) {
    return stringValue || '—'
  }

  return date.toLocaleString()
}

export const buildRowId = (telegram: RawTelegram, fallbackIndex: number): string => {
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

export const normalizeTelegram = (telegram: RawTelegram, index: number): TelegramRow => {
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
