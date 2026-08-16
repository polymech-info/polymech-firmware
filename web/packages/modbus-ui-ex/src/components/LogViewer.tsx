import { useMemo, useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { ScrollArea, ScrollBar } from "@/components/ui/scroll-area";
import { Button } from './ui/button';
import { Switch } from "@/components/ui/switch"
import { Label } from "@/components/ui/label"
import { Wifi, WifiOff, Loader2, ListFilter, ArrowDownCircle, Trash2, AlertTriangle, Timer, RefreshCw } from 'lucide-react';
import { Input } from './ui/input';

const LogViewer = () => {
  const { 
    wsStatus, 
    wsLogEntries, 
    clearWsLogs,
    autoRefreshLogs,
    logRefreshIntervalMs,
    setAutoRefreshLogs,
    setLogRefreshIntervalMs,
    requestLogs
  } = useModbus();
  const [autoScrollEnabled, setAutoScrollEnabled] = useState(true);
  const [activeTab, setActiveTab] = useState<string>('all');
  const scrollAreaRef = useRef<HTMLDivElement>(null);

  const filteredLogs = useMemo(() => {
    const logs = wsLogEntries;
    return {
        all: logs,
        trace: logs.filter(log => log.level === 'Trace'),
        verbose: logs.filter(log => log.level === 'Verbose'),
        info: logs.filter(log => log.level === 'Info'),
        debug: logs.filter(log => log.level === 'Debug'),
        warning: logs.filter(log => log.level === 'Warning'),
        error: logs.filter(log => log.level === 'Error' || log.level === 'Fatal'),
        unknown: logs.filter(log => log.level === 'Unknown'),
    };
  }, [wsLogEntries]);

  useEffect(() => {
    const viewport = scrollAreaRef.current?.querySelector<HTMLDivElement>('[data-radix-scroll-area-viewport]');

    if (autoScrollEnabled && viewport) {
      requestAnimationFrame(() => {
        viewport.scrollTop = viewport.scrollHeight;
      });
    }
  }, [filteredLogs[activeTab as keyof typeof filteredLogs], activeTab, autoScrollEnabled]);

  const renderLogList = (level: keyof typeof filteredLogs) => (
    <ScrollArea 
      ref={activeTab === level ? scrollAreaRef : undefined}
      className="h-[300px] w-full rounded-md border border-white/10 bg-black/20 p-3 font-mono text-xs dark:border-white/10 dark:bg-black/30 light:border-black/10 light:bg-white/30"
    >
      {filteredLogs[level].length === 0 ? (
        <div className="flex items-center justify-center h-full text-muted-foreground">
          No {level !== 'all' ? `${level} ` : ''}logs available.
        </div>
      ) : (
        <>
          {filteredLogs[level].map((log) => (
            <div key={log.id} className="whitespace-pre-wrap mb-1">
              <span className="text-gray-500 mr-2">
                {new Date(log.timestamp).toLocaleTimeString()}
              </span>
              <span className={`mr-2 font-semibold ${getLogLevelColor(log.level)}`}>
                [{log.level}]
              </span>
              <span>{log.message}</span>
            </div>
          ))}
        </>
      )}
      <ScrollBar orientation="vertical" />
      <ScrollBar orientation="horizontal" />
    </ScrollArea>
  );

  const getLogLevelColor = (level: string) => {
    switch (level) {
      case 'Error':
      case 'Fatal':
        return 'text-red-400';
      case 'Warning':
        return 'text-yellow-400';
      case 'Info':
        return 'text-blue-400';
      case 'Debug':
        return 'text-green-400';
      case 'Verbose':
        return 'text-purple-400';
      case 'Trace':
        return 'text-gray-500';
      default:
        return 'text-muted-foreground';
    }
  };

  const handleIntervalChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    let intervalVal = parseInt(e.target.value, 10);
    intervalVal = isNaN(intervalVal) || intervalVal < 50 ? 50 : intervalVal;
    setLogRefreshIntervalMs(intervalVal);
  };

  const isWsConnected = wsStatus === 'CONNECTED';
  const isWsConnecting = wsStatus === 'CONNECTING' || wsStatus === 'RECONNECTING';

  return (
    <div className="glass-morphism p-5 rounded-lg">
      <div className="flex flex-wrap items-center justify-between gap-4 mb-4">
        <h2 className="text-xl font-bold text-gradient flex items-center gap-2">
          <ListFilter className="h-5 w-5" /> System Logs (WebSocket)
        </h2>
        <div className="flex flex-wrap items-center gap-4">
          <div className="flex items-center gap-1 text-xs font-mono glass-morphism px-2 py-1 rounded">
            {isWsConnecting ? <Loader2 className="h-3 w-3 animate-spin text-blue-400" /> : 
             isWsConnected ? <Wifi className="h-3 w-3 text-green-500" /> : 
             wsStatus === 'ERROR' ? <AlertTriangle className="h-3 w-3 text-red-500" /> :
             <WifiOff className="h-3 w-3 text-muted-foreground" />} 
            <span>{wsStatus}</span>
          </div>
          <div className="flex items-center gap-2 glass-morphism px-2 py-1 rounded">
            <Switch
                id="auto-refresh-switch"
                checked={autoRefreshLogs}
                onCheckedChange={setAutoRefreshLogs}
                disabled={!isWsConnected}
            />
            <Label htmlFor="auto-refresh-switch" className="text-xs font-mono flex items-center gap-1">
                <Timer className="h-3 w-3" /> Auto-Refresh:
            </Label>
            <Input
                type="number"
                id="log-interval-input"
                value={logRefreshIntervalMs}
                onChange={handleIntervalChange}
                min="50"
                step="50"
                disabled={!isWsConnected}
                className="w-20 h-6 text-xs font-mono bg-black/30 border-white/10 dark:bg-black/40 dark:border-white/20 light:bg-white/40 light:border-black/20 disabled:opacity-50"
            />
            <span className="text-xs font-mono text-muted-foreground">ms</span>
          </div>
          <Button
              variant="outline"
              size="sm"
              onClick={requestLogs}
              disabled={!isWsConnected}
              className="bg-black/30 border-white/10 flex items-center gap-2 hover:bg-primary/20 h-7 px-2 py-1 text-xs"
          >
              <RefreshCw className="h-3 w-3" />
              <span>Fetch Now</span>
          </Button>
          <div className="flex items-center space-x-2 glass-morphism px-2 py-1 rounded">
            <Switch 
              id="auto-scroll-switch" 
              checked={autoScrollEnabled} 
              onCheckedChange={setAutoScrollEnabled} 
              disabled={!isWsConnected && wsLogEntries.length === 0}
            />
            <Label htmlFor="auto-scroll-switch" className="text-xs font-mono flex items-center gap-1">
              <ArrowDownCircle className="h-3 w-3" /> Auto Scroll
            </Label>
          </div>
          <Button
            variant="destructive"
            size="sm"
            onClick={clearWsLogs}
            disabled={wsLogEntries.length === 0}
            className="bg-red-800/50 border-red-500/30 hover:bg-red-700/50 flex items-center gap-2 text-red-200 h-7 px-2 py-1 text-xs"
          >
            <Trash2 className="h-3 w-3" />
            <span>Clear Client Logs</span>
          </Button>
        </div>
      </div>

      {!isWsConnected && !isWsConnecting && wsLogEntries.length === 0 && (
         <div className="text-center py-10">
          <p className="text-muted-foreground">WebSocket Disconnected. Waiting for connection...</p>
          {wsStatus === 'ERROR' && <p className="text-red-500 text-sm mt-2">Connection Error. Check console and API URL.</p>} 
        </div>
      )}

      {(isWsConnected || wsLogEntries.length > 0) && (
        <Tabs 
          defaultValue="all" 
          className="w-full" 
          value={activeTab} 
          onValueChange={setActiveTab}
        >
          <TabsList className="grid w-full grid-cols-4 sm:grid-cols-8 mb-2 bg-transparent border border-white/10 p-1 h-auto">
            <TabsTrigger value="all" className="text-xs px-2 py-1">All ({filteredLogs.all.length})</TabsTrigger>
            <TabsTrigger value="trace" className="text-xs px-2 py-1">Trace ({filteredLogs.trace.length})</TabsTrigger>
            <TabsTrigger value="verbose" className="text-xs px-2 py-1">Verbose ({filteredLogs.verbose.length})</TabsTrigger>
            <TabsTrigger value="info" className="text-xs px-2 py-1">Info ({filteredLogs.info.length})</TabsTrigger>
            <TabsTrigger value="debug" className="text-xs px-2 py-1">Debug ({filteredLogs.debug.length})</TabsTrigger>
            <TabsTrigger value="warning" className="text-xs px-2 py-1">Warning ({filteredLogs.warning.length})</TabsTrigger>
            <TabsTrigger value="error" className="text-xs px-2 py-1">Error ({filteredLogs.error.length})</TabsTrigger>
            <TabsTrigger value="unknown" className="text-xs px-2 py-1">Unknown ({filteredLogs.unknown.length})</TabsTrigger>
          </TabsList>
          <TabsContent value="all">{renderLogList('all')}</TabsContent>
          <TabsContent value="trace">{renderLogList('trace')}</TabsContent>
          <TabsContent value="verbose">{renderLogList('verbose')}</TabsContent>
          <TabsContent value="info">{renderLogList('info')}</TabsContent>
          <TabsContent value="debug">{renderLogList('debug')}</TabsContent>
          <TabsContent value="warning">{renderLogList('warning')}</TabsContent>
          <TabsContent value="error">{renderLogList('error')}</TabsContent>
          <TabsContent value="unknown">{renderLogList('unknown')}</TabsContent>
        </Tabs>
       )}
    </div>
  );
};

export default LogViewer; 