import React, { useState, useEffect } from 'react';
import { LineChart, Line, ResponsiveContainer, Tooltip } from 'recharts';
import { Activity } from 'lucide-react';

// Define the structure of the systemInfo prop based on ModbusContext
interface SystemInfo {
  version: string;
  board: string;
  uptime: number;
  timestamp: number;
  freeHeapKb: number;
  maxFreeBlockKb: number;
  cpuTicks: number;
  loopDurationMs?: number;
}

interface SystemInfoPanelProps {
  systemInfo: SystemInfo | null;
  formatUptime: (seconds: number) => string;
}

// Define a type for the history data points
interface HistoryPoint {
  timestamp: number;
  value: number;
}

const MAX_HISTORY_LENGTH = 30; // Keep the last 30 data points for the graphs

const SystemInfoPanel: React.FC<SystemInfoPanelProps> = ({ systemInfo, formatUptime }) => {
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

  if (!systemInfo) {
    return null; // Or a loading/placeholder state
  }

  // Simple Tooltip formatter
  const renderTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      return (
        <div className="bg-black/70 text-white p-1 rounded text-xs border border-white/20">
          {`${payload[0].value.toFixed(2)}`}
        </div>
      );
    }
    return null;
  };

  return (
    <div className="bg-black/20 border border-white/10 rounded-lg p-3">
      <h3 className="text-sm font-bold mb-3 flex items-center gap-2">
        <Activity className="h-4 w-4" />
        System Information
      </h3>
      {/* Revert to 2-column grid */}
      <div className="grid grid-cols-2 gap-x-4 gap-y-2 text-sm">
        {/* Labels */}
        <div className="text-muted-foreground">Version:</div>
        <div className="font-mono text-right">{systemInfo.version}</div>
        
        <div className="text-muted-foreground">Board:</div>
        <div className="font-mono text-right">{systemInfo.board}</div>
        
        <div className="text-muted-foreground">Uptime:</div>
        <div className="font-mono text-right">{formatUptime(systemInfo.uptime)}</div>
        
        {typeof systemInfo.loopDurationMs === 'number' ? (
          <>
            <div className="text-muted-foreground">Loop Duration:</div>
            <div className="font-mono text-right">{systemInfo.loopDurationMs.toFixed(2)} us</div>
          </>
        ) : (
          <>
            <div className="text-muted-foreground">Loop Duration:</div>
            <div className="font-mono text-right">N/A</div>
          </>
        )}
        
        <div className="text-muted-foreground">Timestamp:</div>
        <div className="font-mono text-right">{new Date(systemInfo.timestamp * 1000).toLocaleString()}</div>

        {/* Max Free Block - Moved up */}
        <div className="text-muted-foreground">Max Free Block:</div>
        <div className="font-mono text-right">{systemInfo.maxFreeBlockKb.toFixed(2)} KB</div>

        {/* Heap Info with Sparkline in value column */}
        <div className="text-muted-foreground self-center">Free Heap:</div>
        <div className="flex items-center justify-end gap-3"> {/* Use flex for value + chart */}
          <div className="font-mono text-right">{systemInfo.freeHeapKb.toFixed(2)} KB</div>
          <div className="w-24 h-6"> {/* Fixed width container for sparkline */}
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={heapHistory}>
                <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(255, 255, 255, 0.3)', strokeWidth: 1 }} />
                <Line type="monotone" dataKey="value" stroke="#8884d8" strokeWidth={2} dot={false} isAnimationActive={false}/>
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* CPU Ticks Info with Sparkline in value column */}
        <div className="text-muted-foreground self-center">CPU Ticks:</div>
         <div className="flex items-center justify-end gap-3"> {/* Use flex for value + chart */}
           <div className="font-mono text-right">{systemInfo.cpuTicks}</div>
           <div className="w-24 h-6"> {/* Fixed width container for sparkline */}
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={cpuHistory}>
                <Tooltip content={renderTooltip} cursor={{ stroke: 'rgba(255, 255, 255, 0.3)', strokeWidth: 1 }}/>
                <Line type="monotone" dataKey="value" stroke="#82ca9d" strokeWidth={2} dot={false} isAnimationActive={false}/>
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SystemInfoPanel; 