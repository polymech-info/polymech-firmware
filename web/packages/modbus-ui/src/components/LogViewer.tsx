import { useMemo, useState, useEffect, useRef } from 'react';
import { useLocation } from 'react-router-dom';
import { useModbus } from '@/contexts/ModbusContext';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { ScrollArea, ScrollBar } from "@/components/ui/scroll-area";
import { Button } from './ui/button';
import { Switch } from "@/components/ui/switch"
import { Label } from "@/components/ui/label"
import { Wifi, WifiOff, Loader2, ListFilter, ArrowDownCircle, Trash2, AlertTriangle, Download, Filter, ExternalLink } from 'lucide-react';
import { Input } from './ui/input';
import { toast } from 'sonner';
import { 
  DropdownMenu, 
  DropdownMenuTrigger, 
  DropdownMenuContent, 
  DropdownMenuCheckboxItem, 
  DropdownMenuSeparator, 
  DropdownMenuLabel 
} from './ui/dropdown-menu';
import { ENABLE_MODBUS_DEBUG } from '@/constants';
import RTUStatsWidget from './widgets/RTUStatsWidget';
import PendingOpsWidget from './widgets/PendingOpsWidget';
import RTUClientQueueWidget from './widgets/RTUClientQueueWidget';

const LogViewer = () => {
  const { 
    wsStatus, 
    wsLogEntries, 
    clearWsLogs,
    components,
  } = useModbus();
  const location = useLocation();
  const isPopOut = location.pathname.includes('/log-viewer');

  const handlePopOut = () => {
    window.open('/#/log-viewer', '_blank');
  };
  const [autoScrollEnabled, setAutoScrollEnabled] = useState(true);
  const [activeTab, setActiveTab] = useState<string>('all');
  const scrollAreaRef = useRef<HTMLDivElement>(null);
  const messagesEndRef = useRef<HTMLDivElement>(null);
  const [searchTerm, setSearchTerm] = useState(() => sessionStorage.getItem('logViewerSearchTerm') || '');
  const [selectedComponents, setSelectedComponents] = useState<string[]>(() => {
    const saved = sessionStorage.getItem('logViewerSelectedComponents');
    return saved ? JSON.parse(saved) : [];
  });

  const sortedComponents = useMemo(() => 
    [...components].sort((a, b) => a.name.localeCompare(b.name)),
    [components]
  );

  useEffect(() => {
    sessionStorage.setItem('logViewerSearchTerm', searchTerm);
  }, [searchTerm]);

  useEffect(() => {
    sessionStorage.setItem('logViewerSelectedComponents', JSON.stringify(selectedComponents));
  }, [selectedComponents]);

  const filteredLogs = useMemo(() => {
    let logs = wsLogEntries;

    if (selectedComponents.length > 0) {
      const selectedSet = new Set(selectedComponents);
      logs = logs.filter(log => log.name && selectedSet.has(log.name));
    }

    if (searchTerm) {
      const searchTerms = searchTerm.toLowerCase().split(' ').filter(term => term.trim() !== '');
      if (searchTerms.length > 0) {
        logs = logs.filter(log => 
          searchTerms.every(term => 
            (log.message && log.message.toLowerCase().includes(term)) ||
            (log.level && log.level.toLowerCase().includes(term)) ||
            (log.name && log.name.toLowerCase().includes(term)) ||
            (log.id !== undefined && log.id.toString().includes(term))
          )
        );
      }
    }

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
  }, [wsLogEntries, searchTerm, selectedComponents]);

  useEffect(() => {
    if (autoScrollEnabled && messagesEndRef.current) {
      messagesEndRef.current.scrollIntoView({ behavior: 'auto', block: 'end' });
    }
  }, [filteredLogs[activeTab as keyof typeof filteredLogs], activeTab, autoScrollEnabled]);

  const handleDownloadJson = () => {
    if (filteredLogs.all.length === 0) {
      toast.info("No logs to download based on the current filter.");
      return;
    }

    const jsonString = JSON.stringify(filteredLogs.all, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    a.download = `logs-${timestamp}.json`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    toast.success("Filtered logs have been downloaded.");
  };

  const renderLogList = (level: keyof typeof filteredLogs) => (
    <ScrollArea 
      ref={activeTab === level ? scrollAreaRef : undefined}
      className="h-full w-full rounded-md border border-white/10 bg-black/20 p-3 font-mono text-xs dark:border-white/10 dark:bg-black/30 light:border-black/10 light:bg-white/30"
    >
      {filteredLogs[level].length === 0 ? (
        <div className="flex items-center justify-center h-full text-muted-foreground">
          No {level !== 'all' ? `${level} ` : ''}logs available.
        </div>
      ) : (
        <>
          {filteredLogs[level].map((log) => (
            <div key={`${log.id || 'no-id'}-${log.level}-${log.logId || ''}`} className="whitespace-pre-wrap mb-1">
              <span className="text-gray-500 mr-2">
                {new Date(log.timestamp).toLocaleTimeString()}
              </span>
              <span className={`mr-2 font-semibold ${getLogLevelColor(log.level)}`}>
                [{log.level}]
              </span>
              {log.name && (
                <span className="text-orange-400 mr-2">
                  [{log.name}{log.id !== undefined ? `#${log.id}`: ''}]
                </span>
              )}
              <span>{log.message}</span>
            </div>
          ))}
          <div ref={messagesEndRef} />
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
        return 'text-yellow-500';
      case 'Info':
        return 'text-blue-500';
      case 'Debug':
        return 'text-green-500';
      case 'Verbose':
        return 'text-purple-400';
      case 'Trace':
        return 'text-gray-500';
      default:
        return 'text-muted-foreground';
    }
  };

  const isWsConnected = wsStatus === 'CONNECTED';
  const isWsConnecting = wsStatus === 'CONNECTING' || wsStatus === 'RECONNECTING';

  return (
    <div className="glass-morphism p-5 rounded-lg flex flex-col flex-grow" id="logs-display">
      <div className="flex flex-wrap items-center justify-between gap-4 mb-4">
        <h2 className="text-xl font-bold text-gradient flex items-center gap-2">
          <ListFilter className="h-5 w-5" /> System Logs (WebSocket)
        </h2>
        <div className="flex items-center gap-2 w-full sm:w-auto sm:flex-grow max-w-lg">
          <Input
            placeholder="Search by message, level, component..."
            value={searchTerm}
            onChange={e => setSearchTerm(e.target.value)}
            className="w-full bg-black/20 border-white/20 dark:bg-black/20 dark:border-white/20 light:bg-white/40 light:border-black/20"
          />
          <DropdownMenu>
            <DropdownMenuTrigger asChild>
              <Button variant="outline" size="sm" className="h-9">
                <Filter className="h-4 w-4 mr-2" />
                Components ({selectedComponents.length || 'All'})
              </Button>
            </DropdownMenuTrigger>
            <DropdownMenuContent className="w-56 max-h-72 overflow-y-auto">
              <DropdownMenuLabel>Filter by Component</DropdownMenuLabel>
              <DropdownMenuSeparator />
              <DropdownMenuCheckboxItem
                checked={selectedComponents.length === components.length}
                onSelect={() => {
                  if (selectedComponents.length === components.length) {
                    setSelectedComponents([]);
                  } else {
                    setSelectedComponents(components.map(c => c.name));
                  }
                }}
              >
                Select All
              </DropdownMenuCheckboxItem>
              <DropdownMenuSeparator />
              {sortedComponents.map(component => (
                <DropdownMenuCheckboxItem
                  key={component.id}
                  checked={selectedComponents.includes(component.name)}
                  onCheckedChange={(checked) => {
                    if (checked) {
                      setSelectedComponents(prev => [...prev, component.name]);
                    } else {
                      setSelectedComponents(prev => prev.filter(name => name !== component.name));
                    }
                  }}
                >
                  {component.name}
                </DropdownMenuCheckboxItem>
              ))}
            </DropdownMenuContent>
          </DropdownMenu>
        </div>
        <div className="flex flex-wrap items-center gap-4">
          <div className="flex items-center gap-1 text-xs font-mono glass-morphism px-2 py-1 rounded">
            {isWsConnecting ? <Loader2 className="h-3 w-3 animate-spin text-blue-400" /> : 
             isWsConnected ? <Wifi className="h-3 w-3 text-green-500" /> : 
             wsStatus === 'ERROR' ? <AlertTriangle className="h-3 w-3 text-red-500" /> :
             <WifiOff className="h-3 w-3 text-muted-foreground" />} 
            <span>{wsStatus}</span>
          </div>
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
          <Button
            variant="outline"
            size="sm"
            onClick={handleDownloadJson}
            disabled={filteredLogs.all.length === 0}
            className="bg-black/30 border-white/10 flex items-center gap-2 hover:bg-primary/20 h-7 px-2 py-1 text-xs"
          >
            <Download className="h-3 w-3" />
            <span>Download JSON</span>
          </Button>
          {!isPopOut && (
            <Button
              variant="outline"
              size="sm"
              onClick={handlePopOut}
              className="bg-black/30 border-white/10 flex items-center gap-2 hover:bg-primary/20 h-7 px-2 py-1 text-xs"
            >
              <ExternalLink className="h-3 w-3" />
              <span>Pop-out</span>
            </Button>
          )}
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
          className="w-full flex flex-col flex-grow" 
          value={activeTab} 
          onValueChange={setActiveTab}
        >
          <TabsList className={`grid w-full ${ENABLE_MODBUS_DEBUG ? 'grid-cols-4 sm:grid-cols-11' : 'grid-cols-4 sm:grid-cols-8'} mb-2 bg-transparent border border-white/10 p-1 h-auto`}>
            <TabsTrigger value="all" className="text-xs px-2 py-1">All ({filteredLogs.all.length})</TabsTrigger>
            <TabsTrigger value="trace" className="text-xs px-2 py-1">Trace ({filteredLogs.trace.length})</TabsTrigger>
            <TabsTrigger value="verbose" className="text-xs px-2 py-1">Verbose ({filteredLogs.verbose.length})</TabsTrigger>
            <TabsTrigger value="info" className="text-xs px-2 py-1">Info ({filteredLogs.info.length})</TabsTrigger>
            <TabsTrigger value="debug" className="text-xs px-2 py-1">Debug ({filteredLogs.debug.length})</TabsTrigger>
            <TabsTrigger value="warning" className="text-xs px-2 py-1">Warning ({filteredLogs.warning.length})</TabsTrigger>
            <TabsTrigger value="error" className="text-xs px-2 py-1">Error ({filteredLogs.error.length})</TabsTrigger>
            <TabsTrigger value="unknown" className="text-xs px-2 py-1">Unknown ({filteredLogs.unknown.length})</TabsTrigger>
            {ENABLE_MODBUS_DEBUG && (
              <>
                <TabsTrigger value="rtu-stats" className="text-xs px-2 py-1">RTU Stats</TabsTrigger>
                <TabsTrigger value="pending-ops" className="text-xs px-2 py-1">Pending Ops</TabsTrigger>
                <TabsTrigger value="rtu-queue" className="text-xs px-2 py-1">RTU Queue</TabsTrigger>
              </>
            )}
          </TabsList>
          <TabsContent value="all" className="flex-grow"><div className="h-full">{renderLogList('all')}</div></TabsContent>
          <TabsContent value="trace" className="flex-grow"><div className="h-full">{renderLogList('trace')}</div></TabsContent>
          <TabsContent value="verbose" className="flex-grow"><div className="h-full">{renderLogList('verbose')}</div></TabsContent>
          <TabsContent value="info" className="flex-grow"><div className="h-full">{renderLogList('info')}</div></TabsContent>
          <TabsContent value="debug" className="flex-grow"><div className="h-full">{renderLogList('debug')}</div></TabsContent>
          <TabsContent value="warning" className="flex-grow"><div className="h-full">{renderLogList('warning')}</div></TabsContent>
          <TabsContent value="error" className="flex-grow"><div className="h-full">{renderLogList('error')}</div></TabsContent>
          <TabsContent value="unknown" className="flex-grow"><div className="h-full">{renderLogList('unknown')}</div></TabsContent>
          {ENABLE_MODBUS_DEBUG && (
            <>
              <TabsContent value="rtu-stats" className="flex-grow">
                <div className="h-full overflow-auto">
                  <RTUStatsWidget />
                </div>
              </TabsContent>
              <TabsContent value="pending-ops" className="flex-grow">
                <div className="h-full overflow-auto">
                  <PendingOpsWidget />
                </div>
              </TabsContent>
              <TabsContent value="rtu-queue" className="flex-grow">
                <div className="h-full overflow-auto">
                  <RTUClientQueueWidget />
                </div>
              </TabsContent>
            </>
          )}
        </Tabs>
       )}
    </div>
  );
};

export default LogViewer; 