import { ZoneInfo, TelemetryPoint, AlertRecord, OperatorCommand, HardwareActuatorState, KeepAliveStats } from '../types';

const API_BASE = (import.meta as any).env?.VITE_API_URL || '/api';

export async function fetchZones(): Promise<ZoneInfo[]> {
  const res = await fetch(`${API_BASE}/zones`);
  if (!res.ok) throw new Error('Failed to fetch zones');
  const data = await res.json();
  return data.zones;
}

export async function fetchHardwareStatus(): Promise<HardwareActuatorState> {
  const res = await fetch(`${API_BASE}/esp32/status`);
  if (!res.ok) throw new Error('Failed to fetch hardware status');
  return res.json();
}

export async function simulateSandbox(count: number, cameraOnline?: boolean): Promise<any> {
  const res = await fetch(`${API_BASE}/simulate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ count, cameraOnline })
  });
  if (!res.ok) throw new Error('Failed to send simulation payload');
  return res.json();
}

export async function fetchZoneHistory(zoneId: string, minutes = 10): Promise<TelemetryPoint[]> {
  const res = await fetch(`${API_BASE}/zones/${encodeURIComponent(zoneId)}/history?minutes=${minutes}`);
  if (!res.ok) throw new Error('Failed to fetch zone history');
  const data = await res.json();
  return data.history;
}

export async function fetchAlerts(limit = 100): Promise<AlertRecord[]> {
  const res = await fetch(`${API_BASE}/alerts?limit=${limit}`);
  if (!res.ok) throw new Error('Failed to fetch alerts');
  const data = await res.json();
  return data.alerts;
}

export async function sendOperatorCommand(
  deviceId: string,
  command: OperatorCommand,
  reason: string
): Promise<{ status: string }> {
  const res = await fetch(`${API_BASE}/commands`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ deviceId, command, reason })
  });
  if (!res.ok) throw new Error('Failed to send command');
  return res.json();
}

export async function updateZoneConfig(
  zoneId: string,
  updates: { name?: string; areaSqM?: number; corridorWidthM?: number }
): Promise<ZoneInfo> {
  const res = await fetch(`${API_BASE}/zones/${encodeURIComponent(zoneId)}`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(updates)
  });
  if (!res.ok) throw new Error('Failed to update zone config');
  const data = await res.json();
  return data.zone;
}

export async function controlSimulator(action: 'start' | 'stop' | 'toggle' | 'tick'): Promise<{ simulatorRunning: boolean }> {
  const res = await fetch(`${API_BASE}/simulate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ action })
  });
  if (!res.ok) throw new Error('Failed to control simulator');
  return res.json();
}

export async function fetchKeepAliveStats(): Promise<KeepAliveStats> {
  const res = await fetch(`${API_BASE}/keepalive`);
  if (!res.ok) throw new Error('Failed to fetch keep-alive stats');
  const data = await res.json();
  return data.keepAlive;
}

export async function triggerKeepAlivePing(): Promise<KeepAliveStats> {
  const res = await fetch(`${API_BASE}/keepalive/ping`, {
    method: 'POST'
  });
  if (!res.ok) throw new Error('Failed to trigger keep-alive ping');
  const data = await res.json();
  return data.keepAlive;
}


