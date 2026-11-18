import React, { useState, useEffect, useCallback } from 'react';
import { ZoneInfo, AlertRecord, CrowdState, TelemetryPoint } from './types';
import { fetchZones, fetchAlerts, controlSimulator } from './lib/api';
import { GlobalHeader } from './components/GlobalHeader';
import { Navigation } from './components/Navigation';
import { OverviewTab } from './components/OverviewTab';
import { ZoneDetailTab } from './components/ZoneDetailTab';
import { AlertsTab } from './components/AlertsTab';
import { SetupTab } from './components/SetupTab';
import { AboutTab } from './components/AboutTab';

export const App: React.FC = () => {
  const initialTab = typeof window !== 'undefined' ? new URLSearchParams(window.location.search).get('tab') || 'overview' : 'overview';
  const [activeTab, setActiveTab] = useState<string>(initialTab);
  const [selectedZoneId, setSelectedZoneId] = useState<string>('zone-entry-a');
  const [zones, setZones] = useState<ZoneInfo[]>([]);
  const [alerts, setAlerts] = useState<AlertRecord[]>([]);
  const [isWsConnected, setIsWsConnected] = useState<boolean>(false);
  const [lastTelemetryTimestamp, setLastTelemetryTimestamp] = useState<number>(Date.now());
  const [simulatorRunning, setSimulatorRunning] = useState<boolean>(false);
  const [isStale, setIsStale] = useState<boolean>(false);

  // Compute worst state across zones
  const computeWorstState = (): { state: CrowdState; zoneName: string } => {
    if (zones.length === 0) return { state: 'OFFLINE', zoneName: 'No zones' };
    const redZone = zones.find(z => z.currentStatus === 'RED');
    if (redZone) return { state: 'RED', zoneName: redZone.name };
    const amberZone = zones.find(z => z.currentStatus === 'AMBER');
    if (amberZone) return { state: 'AMBER', zoneName: amberZone.name };
    const greenZone = zones.find(z => z.currentStatus === 'GREEN');
    if (greenZone) return { state: 'GREEN', zoneName: greenZone.name };
    return { state: 'OFFLINE', zoneName: 'Offline' };
  };

  const worst = computeWorstState();

  // Load initial data via REST API
  const refreshData = useCallback(async () => {
    try {
      const [fetchedZones, fetchedAlerts] = await Promise.all([
        fetchZones(),
        fetchAlerts(50)
      ]);
      setZones(fetchedZones);
      setAlerts(fetchedAlerts);
    } catch (err) {
      console.warn('[APP] Could not fetch initial REST data:', err);
    }
  }, []);

  useEffect(() => {
    refreshData();
  }, [refreshData]);

  // Handle incoming live telemetry record
  const handleIncomingTelemetry = useCallback((point: TelemetryPoint) => {
    setLastTelemetryTimestamp(Date.now());
    setIsStale(false);

    setZones(prevZones => {
      const existing = prevZones.find(z => z.zoneId === point.zoneId);
      if (existing) {
        return prevZones.map(z => {
          if (z.zoneId === point.zoneId) {
            return {
              ...z,
              currentStatus: point.state,
              score: point.score,
              density: point.density,
              avgSpeed: point.avgSpeed,
              lastSeenMsAgo: 0,
              lastTelemetry: point
            };
          }
          return z;
        });
      } else {
        const newZone: ZoneInfo = {
          zoneId: point.zoneId,
          name: `Zone ${point.zoneId}`,
          areaSqM: 24.0,
          corridorWidthM: 4.0,
          deviceId: point.deviceId,
          currentStatus: point.state,
          score: point.score,
          density: point.density,
          avgSpeed: point.avgSpeed,
          lastSeenMsAgo: 0,
          lastTelemetry: point
        };
        return [...prevZones, newZone];
      }
    });
  }, []);

  // WebSocket Connection Lifecycle
  useEffect(() => {
    let ws: WebSocket | null = null;
    let reconnectTimer: any = null;

    function connect() {
      const isHttps = window.location.protocol === 'https:';
      const wsProtocol = isHttps ? 'wss:' : 'ws:';
      const wsHost = window.location.port === '5173' ? 'localhost:3000' : window.location.host;
      const wsUrl = `${wsProtocol}//${wsHost}`;

      try {
        ws = new WebSocket(wsUrl);

        ws.onopen = () => {
          setIsWsConnected(true);
          console.log('[WS] Connected to Crowd Pressure Radar backend at', wsUrl);
        };

        ws.onmessage = (event) => {
          try {
            const msg = JSON.parse(event.data);
            if (msg.type === 'INITIAL_STATE') {
              if (msg.data.zones) setZones(msg.data.zones);
              if (msg.data.alerts) setAlerts(msg.data.alerts);
              if (typeof msg.data.simulatorRunning === 'boolean') {
                setSimulatorRunning(msg.data.simulatorRunning);
              }
              setLastTelemetryTimestamp(Date.now());
            } else if (msg.type === 'TELEMETRY_UPDATE') {
              handleIncomingTelemetry(msg.data);
            } else if (msg.type === 'SIMULATOR_STATE') {
              setSimulatorRunning(msg.data.isRunning);
            } else if (msg.type === 'ZONE_CONFIG_UPDATED') {
              setZones(prev => prev.map(z => z.zoneId === msg.data.zoneId ? { ...z, ...msg.data } : z));
            }
          } catch (e) {
            console.error('[WS] Error parsing message:', e);
          }
        };

        ws.onerror = () => {
          setIsWsConnected(false);
        };

        ws.onclose = () => {
          setIsWsConnected(false);
          reconnectTimer = setTimeout(connect, 2000);
        };
      } catch (err) {
        setIsWsConnected(false);
        reconnectTimer = setTimeout(connect, 2000);
      }
    }

    connect();

    return () => {
      if (reconnectTimer) clearTimeout(reconnectTimer);
      if (ws) ws.close();
    };
  }, [handleIncomingTelemetry]);

  // Stale Telemetry Timer (> 5s threshold requirement)
  useEffect(() => {
    const timer = setInterval(() => {
      const now = Date.now();
      const age = now - lastTelemetryTimestamp;
      if (age > 5000) {
        setIsStale(true);
      }

      setZones(prev =>
        prev.map(z => ({
          ...z,
          lastSeenMsAgo: z.lastTelemetry ? (now - (z.lastTelemetry.serverTimestamp || z.lastTelemetry.timestamp)) : null
        }))
      );
    }, 1000);

    return () => clearInterval(timer);
  }, [lastTelemetryTimestamp]);

  const handleToggleSimulator = async () => {
    try {
      const res = await controlSimulator('toggle');
      setSimulatorRunning(res.simulatorRunning);
    } catch (err) {
      console.error('Failed to toggle simulator:', err);
    }
  };

  const handleSelectZone = (zoneId: string) => {
    setSelectedZoneId(zoneId);
    setActiveTab('detail');
  };

  const handleZoneUpdated = (updated: ZoneInfo) => {
    setZones(prev => prev.map(z => z.zoneId === updated.zoneId ? { ...z, ...updated } : z));
  };

  return (
    <div className="min-h-screen flex flex-col bg-surface-base text-text-primary antialiased">
      {/* 1. Global Header: Product name, purpose, connection status, worst state pill */}
      <GlobalHeader
        worstState={worst.state}
        worstZoneName={worst.zoneName}
        isWsConnected={isWsConnected}
        isStale={isStale}
        simulatorRunning={simulatorRunning}
        onToggleSimulator={handleToggleSimulator}
      />

      {/* 2. Main Content Body with Desktop Left Navigation and Mobile Top Flow */}
      <div className="flex-1 flex flex-col md:flex-row">
        <Navigation
          activeTab={activeTab}
          setActiveTab={setActiveTab}
          alertCount={alerts.length}
        />

        <main className="flex-1 p-4 sm:p-6 lg:p-8 max-w-7xl mx-auto w-full">
          {activeTab === 'overview' && (
            <OverviewTab
              zones={zones}
              worstState={worst.state}
              onSelectZone={handleSelectZone}
            />
          )}

          {activeTab === 'detail' && (
            <ZoneDetailTab
              zones={zones}
              selectedZoneId={selectedZoneId}
              onSelectZoneId={setSelectedZoneId}
            />
          )}

          {activeTab === 'alerts' && <AlertsTab alerts={alerts} />}

          {activeTab === 'setup' && (
            <SetupTab zones={zones} onZoneUpdated={handleZoneUpdated} />
          )}

          {activeTab === 'about' && <AboutTab />}
        </main>
      </div>
    </div>
  );
};
