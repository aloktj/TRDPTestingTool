import { describe, expect, it } from 'vitest'
import {
  buildRowId,
  formatTimestamp,
  normalizeDirection,
  normalizeStatus,
  normalizeTelegram,
  toStringValue,
  type RawTelegram,
} from './telegramNormalization'

describe('toStringValue', () => {
  it('converts non-string values to strings', () => {
    expect(toStringValue(42)).toBe('42')
    expect(toStringValue(true)).toBe('true')
  })

  it('returns empty string for nullish values', () => {
    expect(toStringValue(null)).toBe('')
    expect(toStringValue(undefined)).toBe('')
  })
})

describe('normalizeStatus', () => {
  it('normalizes common status strings', () => {
    expect(normalizeStatus('Ok')).toBe('OK')
    expect(normalizeStatus('timeout')).toBe('Timeout')
    expect(normalizeStatus('healthy')).toBe('OK')
  })

  it('derives status from booleans and falls back to strings', () => {
    expect(normalizeStatus(true)).toBe('OK')
    expect(normalizeStatus(false)).toBe('Timeout')
    expect(normalizeStatus('custom')).toBe('custom')
  })
})

describe('normalizeDirection', () => {
  it('maps common direction aliases', () => {
    expect(normalizeDirection('tx')).toBe('Outgoing')
    expect(normalizeDirection('rx')).toBe('Incoming')
    expect(normalizeDirection('loopback')).toBe('Loopback')
  })

  it('handles unknown or empty values', () => {
    expect(normalizeDirection('')).toBe('Unknown')
    expect(normalizeDirection(null)).toBe('Unknown')
  })
})

describe('formatTimestamp', () => {
  it('formats valid timestamps and dates', () => {
    const timestamp = 1717632000000 // 2024-06-06T00:00:00.000Z
    expect(formatTimestamp(timestamp)).toContain('2024')
    expect(formatTimestamp('2024-06-06T12:00:00Z')).toContain('2024')
  })

  it('falls back to raw string or em dash for invalid values', () => {
    expect(formatTimestamp('not-a-date')).toBe('not-a-date')
    expect(formatTimestamp(undefined)).toBe('—')
  })
})

describe('buildRowId and normalizeTelegram', () => {
  const raw: RawTelegram = {
    interfaceName: 'eth0',
    comID: 1001,
    title: 'Speed',
    role: 'subscriber',
    lastReceived: '2024-06-01T10:00:00Z',
    state: 'timeout',
  }

  it('creates stable row ids using fallback index', () => {
    expect(buildRowId(raw, 5)).toBe('eth0-1001')
    expect(buildRowId({}, 2)).toBe('iface-2')
  })

  it('normalizes telegram fields with aliases', () => {
    const result = normalizeTelegram(raw, 0)

    expect(result).toEqual(
      expect.objectContaining({
        id: 'eth0-1001',
        interfaceName: 'eth0',
        comId: '1001',
        name: 'Speed',
        direction: 'Incoming',
        status: 'Timeout',
      })
    )
    expect(result.lastRxTime).toContain('2024')
  })
})
