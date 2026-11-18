import React from 'react';
import { Radio, AlertOctagon, ShieldCheck, AlertTriangle, Play, Square, Wifi, WifiOff, Activity } from 'lucide-react';
import { CrowdState } from '../types';

interface NavbarProps {
  activeTab: string;
  setActiveTab: (tab: string) => void;
  worstState: CrowdState;
  isWsConnected: boolean;
  isStale: boolean;
  simulatorRunning: boolean;
  onToggleSimulator: () => void;
  alertCount: number;
}

export const Navbar: React.FC<NavbarProps> = ({
  activeTab,
  setActiveTab,
  worstState,
  isWsConnected,
  isStale,
  simulatorRunning,
  onToggleSimulator,
  alertCount
}) => {
  const tabs = [
    { id: 'overview', label: 'Tactical Overview' },
    { id: 'detail', label: 'Zone Telemetry' },
    { id: 'alerts', label: `Alerts ${alertCount > 0 ? `(${alertCount})` : ''}` },
    { id: 'setup', label: 'Sensor Calibration' },
    { id: 'about', label: 'Safety & Physics' }
  ];

  return (
    <header className="border-b border-radar-border/80 bg-radar-base/90 backdrop-blur-md sticky top-0 z-40">
      {/* Top Disclaimer Notice */}
      <div className="bg-amber-950/40 border-b border-amber-800/30 px-4 py-1.5 text-xs text-amber-200/90 flex items-center justify-between">
        <div className="flex items-center gap-2">
          <AlertTriangle className="w-3.5 h-3.5 text-amber-400 shrink-0" />
          <span className="text-[11px]">
            <strong>RESEARCH & LEARNING PROTOTYPE:</strong> Anonymous mmWave radar early-warning demo. Not certified for life-critical safety.
          </span>
        </div>
        <div className="hidden sm:flex items-center gap-3 font-mono text-[10px] text-amber-300/80">
          <span className="flex items-center gap-1.5">
            <span className="w-1.5 h-1.5 rounded-full bg-emerald-400 animate-pulse" />
            24 GHz mmWave
          </span>
          <span>•</span>
          <span>Zero-Camera Privacy</span>
        </div>
      </div>

      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Brand & Worst Status Pill */}
          <div className="flex items-center gap-4">
            <div className="flex items-center gap-3">
              <div className="relative p-2.5 bg-blue-500/10 border border-blue-500/30 rounded-xl text-blue-400 shadow-[0_0_15px_rgba(59,130,246,0.2)]">
                <Radio className="w-5 h-5 animate-pulse" />
                <span className="absolute -top-1 -right-1 w-2.5 h-2.5 bg-blue-500 rounded-full animate-ping" />
              </div>
              <div>
                <h1 className="text-sm sm:text-base font-black tracking-tight text-white flex items-center gap-2 whitespace-nowrap">
                  <span>Crowd Pressure Radar</span>
                  <span className="text-[10px] uppercase font-mono px-1.5 py-0.5 bg-blue-950/80 text-blue-400 border border-blue-800/60 rounded shrink-0">
                    HLK-LD2450
                  </span>
                </h1>
                <p className="text-[11px] text-slate-400 hidden sm:block whitespace-nowrap">
                  Corridor Micro-Doppler Density & Stall Prevention
                </p>
              </div>
            </div>

            {/* Worst State Badge */}
            <div className={`hidden md:flex items-center gap-2 px-3 py-1 rounded-full text-xs font-bold border transition-all ${
              worstState === 'RED'
                ? 'bg-red-950/90 border-red-500 text-red-200 animate-pulse shadow-[0_0_15px_rgba(239,68,68,0.4)]'
                : worstState === 'AMBER'
                ? 'bg-amber-950/90 border-amber-500 text-amber-200 shadow-[0_0_15px_rgba(245,158,11,0.3)]'
                : worstState === 'GREEN'
                ? 'bg-emerald-950/80 border-emerald-500/80 text-emerald-200 shadow-[0_0_15px_rgba(16,185,129,0.2)]'
                : 'bg-slate-900 border-slate-700 text-slate-400'
            }`}>
              {worstState === 'RED' && <AlertOctagon className="w-3.5 h-3.5 text-red-400" />}
              {worstState === 'AMBER' && <AlertTriangle className="w-3.5 h-3.5 text-amber-400" />}
              {worstState === 'GREEN' && <ShieldCheck className="w-3.5 h-3.5 text-emerald-400" />}
              <span>{worstState === 'OFFLINE' ? 'STANDBY' : `SYSTEM: ${worstState}`}</span>
            </div>
          </div>

          {/* Nav Tabs */}
          <nav className="hidden lg:flex items-center gap-1.5">
            {tabs.map(tab => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-3.5 py-1.5 rounded-lg text-xs font-semibold tracking-wide transition-all ${
                  activeTab === tab.id
                    ? 'bg-slate-800/90 text-white border border-slate-600/80 shadow-sm'
                    : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/40'
                }`}
              >
                {tab.label}
              </button>
            ))}
          </nav>

          {/* Right Controls: Simulator Toggle & Connection Badge */}
          <div className="flex items-center gap-3">
            <button
              onClick={onToggleSimulator}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-lg text-xs font-semibold border transition-all ${
                simulatorRunning
                  ? 'bg-amber-500/10 border-amber-500/50 text-amber-300 hover:bg-amber-500/20 shadow-[0_0_10px_rgba(245,158,11,0.2)]'
                  : 'bg-blue-500/10 border-blue-500/50 text-blue-300 hover:bg-blue-500/20 shadow-[0_0_10px_rgba(59,130,246,0.2)]'
              }`}
              title="Toggle software telemetry simulation without hardware"
            >
              {simulatorRunning ? <Square className="w-3.5 h-3.5 fill-current" /> : <Play className="w-3.5 h-3.5 fill-current" />}
              <span>{simulatorRunning ? 'Pause Sim' : 'Live Sim'}</span>
            </button>

            {/* Connection Status Badge */}
            <div
              className={`flex items-center gap-1.5 px-2.5 py-1.5 rounded-lg text-xs font-mono border ${
                !isWsConnected || isStale
                  ? 'bg-red-950/80 border-red-700 text-red-300'
                  : 'bg-emerald-950/60 border-emerald-700/60 text-emerald-300'
              }`}
            >
              {!isWsConnected || isStale ? (
                <>
                  <WifiOff className="w-3.5 h-3.5 text-red-400" />
                  <span className="hidden sm:inline">{!isWsConnected ? 'OFFLINE' : 'STALE (>5s)'}</span>
                </>
              ) : (
                <>
                  <Wifi className="w-3.5 h-3.5 text-emerald-400" />
                  <span className="hidden sm:inline">LIVE (WS)</span>
                </>
              )}
            </div>
          </div>
        </div>

        {/* Mobile Tab Bar */}
        <div className="flex lg:hidden overflow-x-auto py-2 gap-1.5 border-t border-radar-border/80">
          {tabs.map(tab => (
            <button
              key={tab.id}
              onClick={() => setActiveTab(tab.id)}
              className={`px-3 py-1 rounded-md text-xs font-medium whitespace-nowrap ${
                activeTab === tab.id
                  ? 'bg-blue-600 text-white font-semibold'
                  : 'text-slate-400 hover:text-white bg-slate-900/60'
              }`}
            >
              {tab.label}
            </button>
          ))}
        </div>
      </div>
    </header>
  );
};
