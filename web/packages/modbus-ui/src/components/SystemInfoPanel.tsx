import React, { useState, useEffect } from 'react';
import { LineChart, Line, ResponsiveContainer, Tooltip } from 'recharts';
import { Activity, Download } from 'lucide-react';

// Define the structure of the systemInfo prop based on ModbusContext
interface SystemInfo {
  version: string;
  board: string;
  uptime: number;
  timestamp: number;
  freeHeapKb: number;
  maxFreeBlockKb: number;
  cpuTicks: number;
  fragmentationPercent: number;
  loopDurationMs?: number;
}

interface SystemInfoPanelProps {
  systemInfo: SystemInfo | null;
  formatUptime: (seconds: number) => string;
  onDownloadRequest: () => Promise<Record<string, any>>;
}

// Define a type for the history data points
interface HistoryPoint {
  timestamp: number;
  value: number;
}

const MAX_HISTORY_LENGTH = 30; // Keep the last 30 data points for the graphs

const SystemInfoPanel: React.FC<SystemInfoPanelProps> = ({ systemInfo, formatUptime, onDownloadRequest }) => {
  const [heapHistory, setHeapHistory] = useState<HistoryPoint[]>([]);
  const [cpuHistory, setCpuHistory] = useState<HistoryPoint[]>([]);

  useEffect(() => {
    if (systemInfo) {
      const now = Date.now(); // Use current time for chart x-axis

      // Update Heap History
      setHeapHistory(prevHistory => {
        const newHistory = [...prevHistory, { timestamp: now, value: systemInfo.freeHeapKb }];
        if (newHistory.length > MAX_HISTORY_LENGTH) {
          return newHistory.slice(newHistory.length - MAX_HISTORY_LENGTH);
        }
        return newHistory;
      });

      // Update CPU History (calculate delta if needed, or use raw ticks)
      // For now, let's use raw ticks. Delta might be more meaningful but requires previous value.
      setCpuHistory(prevHistory => {
        const newHistory = [...prevHistory, { timestamp: now, value: systemInfo.cpuTicks }];
        if (newHistory.length > MAX_HISTORY_LENGTH) {
          return newHistory.slice(newHistory.length - MAX_HISTORY_LENGTH);
        }
        return newHistory;
      });
    }
  }, [systemInfo]); // Re-run effect when systemInfo prop changes

  const handleDownload = async () => {
    if (!systemInfo) return;

    const featureFlags = await onDownloadRequest();

    const dataToDownload = {
      ...systemInfo,
      featureFlags,
    };

    const jsonString = `data:text/json;charset=utf-8,${encodeURIComponent(
      JSON.stringify(dataToDownload, null, 2)
    )}`;
    const link = document.createElement("a");
    link.href = jsonString;
    link.download = "system-info.json";
    link.click();
  };

  if (!systemInfo) {
    return null; // Or a loading/placeholder state
  }

  // Simple Tooltip formatter
  const renderTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      return (
        <div className="bg-slate-800/90 dark:bg-black/70 text-white p-1 rounded text-xs border border-slate-300/20 dark:border-white/20">
          {`${payload[0].value.toFixed(2)}`}
        </div>
      );
    }
    return null;
  };

  return (
    <div className="p-2">
      <h3 className="text-sm font-bold mb-2 md:mb-3 flex items-center justify-between text-slate-700 dark:text-white">
        <div className="flex items-center gap-2">
          <Activity className="h-4 w-4" />
          <span>System Information</span>
        </div>
        <Download
          className="h-4 w-4 text-slate-500 dark:text-slate-400 hover:text-slate-700 dark:hover:text-white cursor-pointer transition-colors"
          onClick={handleDownload}
        />
      </h3>
      {/* Revert to 2-column grid */}
      <div className="grid grid-cols-2 gap-x-2 md:gap-x-4 gap-y-1 md:gap-y-2 text-xs md:text-sm">
        {/* Labels */}
        <div className="text-slate-500 dark:text-slate-400">Version:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">
          {systemInfo.version.split('|').map((part, index) => (
            <div key={index} className="truncate" title={part}>{part}</div>
          ))}
        </div>

        <div className="text-slate-500 dark:text-slate-400">Board:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.board}</div>

        <div className="text-slate-500 dark:text-slate-400">Uptime:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">{formatUptime(systemInfo.uptime)}</div>

        <div className="text-slate-500 dark:text-slate-400">Fragmentation:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.fragmentationPercent.toFixed(1)}%</div>

        {typeof systemInfo.loopDurationMs === 'number' ? (
          <>
            <div className="text-slate-500 dark:text-slate-400">Loop Duration:</div>
            <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.loopDurationMs.toFixed(2)} us</div>
          </>
        ) : (
          <>
            <div className="text-slate-500 dark:text-slate-400">Loop Duration:</div>
            <div className="font-mono text-right text-slate-700 dark:text-white">N/A</div>
          </>
        )}

        <div className="text-slate-500 dark:text-slate-400">Timestamp:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">{new Date(systemInfo.timestamp * 1000).toLocaleString()}</div>

        {/* Max Free Block - Moved up */}
        <div className="text-slate-500 dark:text-slate-400">Max Free Block:</div>
        <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.maxFreeBlockKb.toFixed(2)} KB</div>

        {/* Heap Info with Sparkline in value column */}
        <div className="text-slate-500 dark:text-slate-400 self-center">Free Heap:</div>
        <div className="flex items-center justify-end gap-1 md:gap-3"> {/* Use flex for value + chart */}
          <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.freeHeapKb.toFixed(2)} KB</div>
          <div className="w-16 md:w-24 h-4 md:h-6"> {/* Fixed width container for sparkline */}
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={heapHistory}>
                <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(100, 116, 139, 0.3)', strokeWidth: 1 }} />
                <Line type="monotone" dataKey="value" stroke="#6366f1" strokeWidth={2} dot={false} isAnimationActive={false} />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* CPU Ticks Info with Sparkline in value column */}
        <div className="text-slate-500 dark:text-slate-400 self-center">CPU Ticks:</div>
        <div className="flex items-center justify-end gap-1 md:gap-3"> {/* Use flex for value + chart */}
          <div className="font-mono text-right text-slate-700 dark:text-white">{systemInfo.cpuTicks}</div>
          <div className="w-16 md:w-24 h-4 md:h-6"> {/* Fixed width container for sparkline */}
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={cpuHistory}>
                <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(100, 116, 139, 0.3)', strokeWidth: 1 }} />
                <Line type="monotone" dataKey="value" stroke="#06b6d4" strokeWidth={2} dot={false} isAnimationActive={false} />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SystemInfoPanel; 