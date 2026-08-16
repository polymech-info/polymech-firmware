import React, { useState, useEffect, useCallback } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Badge } from '@/components/ui/badge';
import { RefreshCw, Play, Pause, Settings, Info } from 'lucide-react';
import { useModbus } from '@/contexts/ModbusContext';
import modbusService from '@polymech/client-ts/modbusService';
import { Collapsible, CollapsibleContent } from '@/components/ui/collapsible';
import { Alert, AlertDescription } from '@/components/ui/alert';

export const RTUClientQueueWidget: React.FC = () => {
  const { isConnected } = useModbus();
  const [queueData, setQueueData] = useState<any>(null);
  const [isPolling, setIsPolling] = useState(false);
  const [pollInterval, setPollInterval] = useState(2000);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [showSettings, setShowSettings] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);

  // Callback to handle real-time RTU queue updates
  const handleRTUQueueUpdate = useCallback((newQueueData: any) => {
    setQueueData(newQueueData);
    setLastUpdate(new Date());
    setError(null);
  }, []);

  // Register the callback when component mounts
  useEffect(() => {
    modbusService.registerDebugHandlers(undefined, handleRTUQueueUpdate);
    
    // Cleanup on unmount
    return () => {
      modbusService.registerDebugHandlers(undefined, () => {});
    };
  }, [handleRTUQueueUpdate]);

  const fetchRTUQueue = useCallback(async () => {
    if (!isConnected) {
      setError('Not connected to server');
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const response = await modbusService.getRTUClientQueue({});
      
      if (response) {
        setQueueData(response);
      } else {
        setQueueData(null);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch RTU client queue');
      setQueueData(null);
    } finally {
      setLoading(false);
    }
  }, [isConnected]);

  useEffect(() => {
    let intervalId: NodeJS.Timeout | null = null;

    if (isPolling && isConnected && pollInterval > 0) {
      // Initial fetch
      fetchRTUQueue();
      
      // Set up polling
      intervalId = setInterval(fetchRTUQueue, pollInterval);
    }

    return () => {
      if (intervalId) {
        clearInterval(intervalId);
      }
    };
  }, [isPolling, isConnected, pollInterval, fetchRTUQueue]);

  const handleStartPolling = () => {
    setIsPolling(true);
  };

  const handleStopPolling = () => {
    setIsPolling(false);
  };

  const handleManualRefresh = () => {
    fetchRTUQueue();
  };

  const renderQueueData = () => {
    if (!queueData) {
      return (
        <div className="text-center text-muted-foreground py-8">
          No queue data available
        </div>
      );
    }

    // Handle the case where it's just a message (placeholder implementation)
    if (queueData.data && queueData.data.message) {
      return (
        <Alert>
          <Info className="h-4 w-4" />
          <AlertDescription>
            {queueData.data.message}
          </AlertDescription>
        </Alert>
      );
    }

    // If we have actual queue data, render it as JSON for now
    // This can be enhanced once the actual structure is known
    return (
      <div className="space-y-4">
        <div className="text-sm font-medium">Queue Information:</div>
        <pre className="bg-muted p-4 rounded-md text-sm overflow-auto max-h-96">
          {JSON.stringify(queueData, null, 2)}
        </pre>
      </div>
    );
  };

  return (
    <Card className="w-full">
      <CardHeader>
        <div className="flex items-center justify-between">
          <CardTitle className="flex items-center gap-2">
            RTU Client Queue
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
                <Label htmlFor="rtu-poll-interval">Poll Interval (ms)</Label>
                <Input
                  id="rtu-poll-interval"
                  type="number"
                  min="500"
                  max="10000"
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
            Not connected to server. Connect to view RTU client queue.
          </div>
        )}

        <div className="min-h-[200px]">
          {loading ? (
            <div className="flex items-center justify-center py-8">
              <RefreshCw className="h-6 w-6 animate-spin" />
              <span className="ml-2">Loading queue data...</span>
            </div>
          ) : (
            renderQueueData()
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

export default RTUClientQueueWidget;
