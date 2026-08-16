import React, { useState, useEffect, useCallback } from 'react';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Badge } from '@/components/ui/badge';
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from '@/components/ui/table';
import { RefreshCw, Play, Pause, Settings } from 'lucide-react';
import { useModbus } from '@/contexts/ModbusContext';
import modbusService from '@polymech/client-ts/modbusService';
import { Collapsible, CollapsibleContent, CollapsibleTrigger } from '@/components/ui/collapsible';

interface ModbusOperation {
  token: number;
  slaveId: number;
  address: number;
  type: number;
  value: number;
  quantity: number;
  status: number;
  retries: number;
  timestamp: number;
  high_priority: boolean;
  in_progress: boolean;
}

interface PendingOpsResponse {
  type: string;
  data: ModbusOperation[];
}

const operationTypeNames: Record<number, string> = {
  1: 'Read Coils',
  2: 'Read Discrete Inputs',
  3: 'Read Holding Registers',
  4: 'Read Input Registers',
  5: 'Write Single Coil',
  6: 'Write Single Register',
  15: 'Write Multiple Coils',
  16: 'Write Multiple Registers',
};

const statusNames: Record<number, string> = {
  0: 'Pending',
  1: 'In Progress',
  2: 'Completed',
  3: 'Failed',
  4: 'Timeout',
  5: 'Cancelled',
};

const statusColors: Record<number, string> = {
  0: 'bg-yellow-500',
  1: 'bg-blue-500',
  2: 'bg-green-500',
  3: 'bg-red-500',
  4: 'bg-orange-500',
  5: 'bg-gray-500',
};

export const PendingOpsWidget: React.FC = () => {
  const { isConnected } = useModbus();
  const [operations, setOperations] = useState<ModbusOperation[]>([]);
  const [isPolling, setIsPolling] = useState(false);
  const [pollInterval, setPollInterval] = useState(1000);
  const [slaveIdFilter, setSlaveIdFilter] = useState<number | null>(null);
  const [statusFilter, setStatusFilter] = useState<number | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [showSettings, setShowSettings] = useState(false);
  const [lastUpdate, setLastUpdate] = useState<Date | null>(null);

  // Callback to handle real-time pending ops updates
  const handlePendingOpsUpdate = useCallback((newOperations: ModbusOperation[]) => {
    setOperations(newOperations);
    setLastUpdate(new Date());
    setError(null);
  }, []);

  // Register the callback when component mounts
  useEffect(() => {
    modbusService.registerDebugHandlers(handlePendingOpsUpdate, undefined);
    
    // Cleanup on unmount
    return () => {
      modbusService.registerDebugHandlers(() => {}, undefined);
    };
  }, [handlePendingOpsUpdate]);

  const fetchPendingOps = useCallback(async () => {
    if (!isConnected) {
      setError('Not connected to server');
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const payload: any = {};
      if (slaveIdFilter !== null) {
        payload.slaveId = slaveIdFilter;
      }
      if (statusFilter !== null) {
        payload.status = statusFilter;
      }

      const response = await modbusService.getPendingOps(payload);
      
      if (response && Array.isArray(response)) {
        setOperations(response);
      } else if (response && response.data && Array.isArray(response.data)) {
        setOperations(response.data);
      } else {
        setOperations([]);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch pending operations');
      setOperations([]);
    } finally {
      setLoading(false);
    }
  }, [isConnected, slaveIdFilter, statusFilter]);

  useEffect(() => {
    let intervalId: NodeJS.Timeout | null = null;

    if (isPolling && isConnected && pollInterval > 0) {
      // Initial fetch
      fetchPendingOps();
      
      // Set up polling
      intervalId = setInterval(fetchPendingOps, pollInterval);
    }

    return () => {
      if (intervalId) {
        clearInterval(intervalId);
      }
    };
  }, [isPolling, isConnected, pollInterval, fetchPendingOps]);

  const handleStartPolling = () => {
    setIsPolling(true);
  };

  const handleStopPolling = () => {
    setIsPolling(false);
  };

  const handleManualRefresh = () => {
    fetchPendingOps();
  };

  const formatTimestamp = (timestamp: number) => {
    return new Date(timestamp).toLocaleTimeString();
  };

  const getOperationTypeName = (type: number) => {
    return operationTypeNames[type] || `Unknown (${type})`;
  };

  const getStatusName = (status: number) => {
    return statusNames[status] || `Unknown (${status})`;
  };

  const getStatusColor = (status: number) => {
    return statusColors[status] || 'bg-gray-500';
  };

  return (
    <Card className="w-full">
      <CardHeader>
        <div className="flex items-center justify-between">
          <CardTitle className="flex items-center gap-2">
            Pending Modbus Operations
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
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4 p-4 border rounded-lg bg-muted/50">
              <div className="space-y-2">
                <Label htmlFor="poll-interval">Poll Interval (ms)</Label>
                <Input
                  id="poll-interval"
                  type="number"
                  min="100"
                  max="10000"
                  step="100"
                  value={pollInterval}
                  onChange={(e) => setPollInterval(parseInt(e.target.value) || 1000)}
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="slave-filter">Slave ID Filter</Label>
                <Input
                  id="slave-filter"
                  type="number"
                  min="0"
                  max="255"
                  placeholder="All slaves"
                  value={slaveIdFilter || ''}
                  onChange={(e) => setSlaveIdFilter(e.target.value ? parseInt(e.target.value) : null)}
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="status-filter">Status Filter</Label>
                <Select
                  value={statusFilter?.toString() || 'all'}
                  onValueChange={(value) => setStatusFilter(value === 'all' ? null : parseInt(value))}
                >
                  <SelectTrigger>
                    <SelectValue placeholder="All statuses" />
                  </SelectTrigger>
                  <SelectContent>
                    <SelectItem value="all">All Statuses</SelectItem>
                    {Object.entries(statusNames).map(([status, name]) => (
                      <SelectItem key={status} value={status}>
                        {name}
                      </SelectItem>
                    ))}
                  </SelectContent>
                </Select>
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
            Not connected to server. Connect to view pending operations.
          </div>
        )}

        <div className="rounded-md border">
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead>Token</TableHead>
                <TableHead>Slave</TableHead>
                <TableHead>Address</TableHead>
                <TableHead>Type</TableHead>
                <TableHead>Value</TableHead>
                <TableHead>Quantity</TableHead>
                <TableHead>Status</TableHead>
                <TableHead>Retries</TableHead>
                <TableHead>Flags</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {operations.length === 0 ? (
                <TableRow>
                  <TableCell colSpan={10} className="text-center text-muted-foreground">
                    {loading ? 'Loading...' : 'No pending operations'}
                  </TableCell>
                </TableRow>
              ) : (
                operations.map((op, index) => (
                  <TableRow key={`${op.token}-${index}`}>
                    <TableCell className="font-mono">{op.token}</TableCell>
                    <TableCell>{op.slaveId}</TableCell>
                    <TableCell>{op.address}</TableCell>
                    <TableCell className="text-sm">{getOperationTypeName(op.type)}</TableCell>
                    <TableCell>{op.value}</TableCell>
                    <TableCell>{op.quantity}</TableCell>
                    <TableCell>
                      <Badge className={`text-white ${getStatusColor(op.status)}`}>
                        {getStatusName(op.status)}
                      </Badge>
                    </TableCell>
                    <TableCell>{op.retries}</TableCell>
                    <TableCell>
                      <div className="flex gap-1">
                        {op.high_priority && (
                          <Badge variant="outline" className="text-xs">HP</Badge>
                        )}
                        {op.in_progress && (
                          <Badge variant="outline" className="text-xs">IP</Badge>
                        )}
                      </div>
                    </TableCell>
                  </TableRow>
                ))
              )}
            </TableBody>
          </Table>
        </div>

        <div className="text-sm text-muted-foreground">
          Total operations: {operations.length}
          {lastUpdate && ` • Last update: ${lastUpdate.toLocaleTimeString()}`}
          {isPolling && ` • Polling every ${pollInterval}ms`}
        </div>
      </CardContent>
    </Card>
  );
};

export default PendingOpsWidget;
