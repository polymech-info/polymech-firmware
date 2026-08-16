import React, { useState, useEffect, useCallback, useMemo } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Badge } from '@/components/ui/badge';
import { RefreshCw, Play, Pause, Settings, BarChart3 } from 'lucide-react';
import { useModbus } from '@/contexts/ModbusContext';
import modbusService from '@polymech/client-ts/modbusService';
import { Collapsible, CollapsibleContent } from '@/components/ui/collapsible';

interface RTUStatsData {
  [key: string]: any;
}

const RTUStatsWidgetComponent: React.FC = () => {
  const { isConnected } = useModbus();
  const [statsData, setStatsData] = useState<RTUStatsData | null>(null);
  const [isPolling, setIsPolling] = useState(false); // Rely on real-time updates primarily
  const [pollInterval, setPollInterval] = useState(2000);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [showSettings, setShowSettings] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);

  // Throttle updates to prevent excessive re-renders
  const lastUpdateRef = React.useRef<number>(0);
  const throttleDelay = 500; // 500ms throttle

  const fetchRTUStats = useCallback(async () => {
    if (!isConnected) {
      setError('Not connected to server');
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const response = await modbusService.getRTUStats({});
      
      if (response) {
        setStatsData(response);
        setLastUpdate(new Date());
      } else {
        setStatsData(null);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch RTU stats');
      setStatsData(null);
    } finally {
      setLoading(false);
    }
  }, [isConnected]);

  // Callback to handle real-time RTU stats updates
  const handleRTUStatsUpdate = useCallback((newStatsData: RTUStatsData) => {
    const now = Date.now();
    
    // Throttle updates to prevent flickering
    if (now - lastUpdateRef.current < throttleDelay) {
      return;
    }
    
    lastUpdateRef.current = now;
    setStatsData(newStatsData);
    setLastUpdate(new Date());
    setError(null);
    setLoading(false); // Clear loading state when data is received
  }, [throttleDelay]);

  // Register the callback when component mounts
  useEffect(() => {
    modbusService.registerDebugHandlers(undefined, undefined, handleRTUStatsUpdate);
    
    // Cleanup on unmount
    return () => {
      modbusService.registerDebugHandlers(undefined, undefined, () => {});
    };
  }, [handleRTUStatsUpdate]);

  // Initial data fetch when component mounts and is connected
  useEffect(() => {
    if (isConnected) {
      fetchRTUStats(); // Get initial data
    }
  }, [isConnected, fetchRTUStats]);

  // Handle page visibility changes - fetch fresh data when page becomes visible
  useEffect(() => {
    const handleVisibilityChange = () => {
      if (!document.hidden && isConnected) {
        fetchRTUStats(); // Fetch fresh data when returning to tab
      }
    };

    document.addEventListener('visibilitychange', handleVisibilityChange);
    
    return () => {
      document.removeEventListener('visibilitychange', handleVisibilityChange);
    };
  }, [isConnected, fetchRTUStats]);

  useEffect(() => {
    let intervalId: NodeJS.Timeout | null = null;

    if (isPolling && isConnected && pollInterval > 0) {
      // Only do initial fetch, rely on real-time updates for ongoing data
      fetchRTUStats();
      
      // Set up polling as backup (optional, since we get real-time updates)
      intervalId = setInterval(fetchRTUStats, pollInterval);
    }

    return () => {
      if (intervalId) {
        clearInterval(intervalId);
      }
    };
  }, [isPolling, isConnected, pollInterval, fetchRTUStats]);

  const handleStartPolling = () => {
    setIsPolling(true);
  };

  const handleStopPolling = () => {
    setIsPolling(false);
  };

  const handleManualRefresh = () => {
    fetchRTUStats();
  };

  // Memoize the stats rendering to prevent unnecessary re-renders
  const renderedStats = useMemo(() => {
    if (!statsData) {
      return (
        <div className="text-center text-muted-foreground py-8">
          No RTU stats data available
        </div>
      );
    }

    // Render stats as key-value pairs in a grid
    const entries = Object.entries(statsData);
    
    if (entries.length === 0) {
      return (
        <div className="text-center text-muted-foreground py-8">
          No stats available
        </div>
      );
    }

    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {entries.map(([key, value]) => (
          <div key={key} className="p-3 border rounded-lg bg-muted/20 transition-all duration-200">
            <div className="text-sm font-medium text-muted-foreground capitalize">
              {key.replace(/_/g, ' ')}
            </div>
            <div className="text-lg font-semibold mt-1 font-mono">
              {typeof value === 'object' ? JSON.stringify(value) : String(value)}
            </div>
          </div>
        ))}
      </div>
    );
  }, [statsData]);

  return (
    <Card className="w-full">
      <CardHeader>
        <div className="flex items-center justify-between">
          <CardTitle className="flex items-center gap-2">
            <BarChart3 className="h-5 w-5" />
            RTU Statistics
            {isPolling && <Badge variant="secondary" className="animate-pulse">Live</Badge>}
          </CardTitle>
          <div className="flex items-center gap-2">
            <Button
              variant="outline"
              size="sm"
              onClick={() => setShowSettings(!showSettings)}
            >
              <Settings className="h-4 w-4" />
            </Button>
            <Button
              variant="outline"
              size="sm"
              onClick={handleManualRefresh}
              disabled={loading}
            >
              <RefreshCw className={`h-4 w-4 ${loading ? 'animate-spin' : ''}`} />
            </Button>
            {isPolling ? (
              <Button variant="outline" size="sm" onClick={handleStopPolling}>
                <Pause className="h-4 w-4" />
                Stop
              </Button>
            ) : (
              <Button variant="default" size="sm" onClick={handleStartPolling} disabled={!isConnected}>
                <Play className="h-4 w-4" />
                Start
              </Button>
            )}
          </div>
        </div>
      </CardHeader>
      <CardContent className="space-y-4">
        <Collapsible open={showSettings} onOpenChange={setShowSettings}>
          <CollapsibleContent className="space-y-4">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4 p-4 border rounded-lg bg-muted/50">
              <div className="space-y-2">
                <Label htmlFor="rtu-stats-poll-interval">Poll Interval (ms)</Label>
                <Input
                  id="rtu-stats-poll-interval"
                  type="number"
                  min="500"
                  max="30000"
                  step="100"
                  value={pollInterval}
                  onChange={(e) => setPollInterval(parseInt(e.target.value) || 2000)}
                />
              </div>
              <div className="space-y-2">
                <Label>Status</Label>
                <div className="flex items-center gap-2 text-sm">
                  <Badge variant={isConnected ? "default" : "secondary"}>
                    {isConnected ? "Connected" : "Disconnected"}
                  </Badge>
                  {isPolling && (
                    <Badge variant="outline">
                      Polling every {pollInterval}ms
                    </Badge>
                  )}
                </div>
              </div>
            </div>
          </CollapsibleContent>
        </Collapsible>

        {error && (
          <div className="p-3 text-sm text-red-600 bg-red-50 border border-red-200 rounded">
            {error}
          </div>
        )}

        {!isConnected && (
          <div className="p-3 text-sm text-yellow-600 bg-yellow-50 border border-yellow-200 rounded">
            Not connected to server. Connect to view RTU statistics.
          </div>
        )}

        <div className="min-h-[200px]">
          {loading ? (
            <div className="flex items-center justify-center py-8">
              <RefreshCw className="h-6 w-6 animate-spin" />
              <span className="ml-2">Loading RTU stats...</span>
            </div>
          ) : (
            renderedStats
          )}
        </div>

        <div className="text-sm text-muted-foreground">
          {lastUpdate && `Last updated: ${lastUpdate.toLocaleTimeString()}`}
          {isPolling && ` • Polling every ${pollInterval}ms`}
        </div>
      </CardContent>
    </Card>
  );
};

// Memoize the component to prevent unnecessary re-renders
export const RTUStatsWidget = React.memo(RTUStatsWidgetComponent);

export default RTUStatsWidget;
