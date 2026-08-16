import React, { useState, useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Loader2, Plug, Power, RotateCw, Server, Activity, List, HelpCircle, Download } from 'lucide-react';
import { cn } from '@/lib/utils';
import SystemInfoPanel from './SystemInfoPanel';
import ThemeToggle  from '@/components/ThemeToggle'
import { T, getCurrentLang, getTranslationCache, translate, supportedLanguages } from '../i18n';
import CollapsibleSection from './CollapsibleSection';
import DisplayMessagesPanel from './DisplayMessagesPanel';


const ConnectionPanel = () => {
  const { 
    isConnected, 
    connectToServer, 
    disconnectFromServer, 
    refreshData,
    connecting,
    apiUrl,
    setApiUrl,
    systemInfo,
    updateRegister,
    setAutoRefreshSystemInfo
  } = useModbus();
  
  const [serverUrl, setServerUrl] = useState(apiUrl.replace('/api', ''));
  const reconnectIntervalRef = useRef<NodeJS.Timeout | null>(null);
  const initialConnectionAttempted = useRef(false);
  const currentLang = getCurrentLang();
  useEffect(() => {
    if (apiUrl && !initialConnectionAttempted.current) {
      connectToServer();
      initialConnectionAttempted.current = true;
    }
  }, [apiUrl, connectToServer]);

  const handleConnect = async () => {
    let urlToConnect = serverUrl;
    // Ensure /api is not duplicated if user types it, but ensure it is present if they don't.
    // This logic might need refinement based on exact desired behavior if user includes /api
    if (urlToConnect.endsWith('/api')) {
        urlToConnect = urlToConnect.substring(0, urlToConnect.length - '/api'.length);
    }
    // setApiUrl will append /api if it's not there (based on original modbusApiService logic)
    setApiUrl(urlToConnect); 
    if (connectToServer) await connectToServer();
  };

  const handleDisconnect = () => {
    if (disconnectFromServer) disconnectFromServer();
  };

  const handleRefresh = async () => {
    if (refreshData) await refreshData();
  };

  const handleLanguageChange = (langCode: string) => {
    const currentUrl = window.location.href;
    const url = new URL(currentUrl);
    url.searchParams.set('lang', langCode);
    window.location.href = url.toString();
  };

  const handleDownloadEnglishTranslations = () => {
    const englishTranslations = getTranslationCache('en');
    const jsonString = JSON.stringify(englishTranslations, null, 2);
    const blob = new Blob([jsonString], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'translations_en.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    // Optionally, add a toast notification for success
    // toast.success(translate("English translations downloaded."));
  };

  const handleReset = async () => {
    if (updateRegister) {
      try {
        await updateRegister(100, 1);;
      } catch (error) {
        // Optionally, add an error toast message here
        // console.error('Failed to send reset command:', error);
        // toast.error('Failed to send reset command: ' + (error instanceof Error ? error.message : String(error)));
      }
    }
  };

  // Function to open URL in a new tab
  const openApiUrl = (path: string) => {
    // For absolute URLs or hash-based client-side routes, open them directly.
    // For relative API paths, construct the URL based on apiUrl.
    if (path.startsWith('/#')) {
      window.location.href = path; // Navigate in the same window for hash routes
    } else if (path.startsWith('http')) {
      window.open(path, '_blank', 'noopener,noreferrer'); // Open absolute URLs in a new tab
    } else {
      const baseUrl = apiUrl.endsWith('/api') ? apiUrl.substring(0, apiUrl.length - 4) : apiUrl;
      window.open(`${baseUrl}${path}`, '_blank', 'noopener,noreferrer'); // Open API relative paths in a new tab
    }
  };

  const formatUptime = (seconds: number): string => {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const remainingSeconds = seconds % 60;
    
    return `${days}d ${hours}h ${minutes}m ${remainingSeconds}s`;
  };

  // Auto-reconnect logic
  useEffect(() => {
    const clearReconnectInterval = () => {
      if (reconnectIntervalRef.current) {
        clearInterval(reconnectIntervalRef.current);
        reconnectIntervalRef.current = null;
      }
    };

    if (!isConnected && !connecting && connectToServer) {
      // Clear any existing interval before setting a new one
      clearReconnectInterval(); 
      reconnectIntervalRef.current = setInterval(() => {
        // Use the current apiUrl from context directly
        if (apiUrl) { 
          connectToServer(); 
        }
      }, 5000);
    } else {
      clearReconnectInterval();
    }

    // Cleanup function to clear interval on unmount or when dependencies change
    return () => {
      clearReconnectInterval();
    };
  }, [isConnected, connecting, apiUrl, connectToServer]); // Include apiUrl and connectToServer in dependencies

  return (
    <div className="glass-morphism p-3 md:p-5 rounded-lg">
      <div className="flex flex-col space-y-3 md:space-y-4">
        {/* Responsive main header container */}
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-3 sm:gap-2">
          <h2 className="text-lg md:text-xl font-bold text-gradient text-center sm:text-left"><T>PolyMech - Cassandra</T></h2>
          
          {/* Container for controls (language, theme, help) and status indicator */}
          <div className="flex flex-col items-center sm:items-end gap-2">
            {/* Sub-container for language, theme, help - allows wrapping */}
            <div className="flex items-center flex-wrap justify-center sm:justify-end gap-x-1 md:gap-x-2 gap-y-1">
              <div className="flex items-center space-x-1 text-xs md:text-sm">
                {supportedLanguages.map((lang) => (
                  <Button
                    key={lang.code}
                    variant={currentLang === lang.code ? "default" : "ghost"}
                    size="sm"
                    onClick={() => handleLanguageChange(lang.code)}
                    className={cn(
                      "p-1 md:p-1.5 h-auto", // h-auto to allow text to determine height with padding
                      currentLang === lang.code ? "font-bold" : ""
                    )}
                  >
                    {lang.name}
                  </Button>
                ))}
              </div>
              {/* Download English Translations Button */}
              <Button
                variant="ghost"
                size="sm"
                onClick={handleDownloadEnglishTranslations}
                className="p-1 md:p-1.5 h-auto"
                title={translate("Download English Translations")}
              >
                <Download className="h-4 w-4 md:h-5 md:w-5" />
              </Button>
              <div className="flex items-center gap-x-1 md:gap-x-2">
                <ThemeToggle />
                <Button
                  variant="ghost"
                  size="sm"
                  onClick={() => window.open('https://git.polymech.io/polymech/polymech-fw-apps/src/branch/master/docs', '_blank', 'noopener,noreferrer')}
                  className="p-1 md:p-1.5 h-auto" // h-auto
                  title="Help"
                >
                  <HelpCircle className="h-4 w-4 md:h-5 md:w-5" />
                </Button>
              </div>
            </div>

            {/* Status Indicator */}
            <div className="flex items-center space-x-1 md:space-x-2">
              <div className={cn(
                "w-2.5 h-2.5 md:w-3 md:h-3 rounded-full",
                isConnected ? "bg-green-500 animate-pulse-glow" : "bg-red-500"
              )}></div>
              <span className="text-xs md:text-sm font-mono">
                {isConnected ? <T>ONLINE</T> : <T>OFFLINE</T>}
              </span>
            </div>
          </div>
        </div>
        
        <div className="space-y-2">
          <Label htmlFor="apiUrl" className="text-xs font-mono"><T>API URL</T></Label>
          <div className="relative flex items-center">
            <Input
              id="apiUrl"
              value={serverUrl}
              onChange={(e) => setServerUrl(e.target.value)}
              className="bg-black/30 border-white/10 font-mono text-sm md:text-base dark:bg-black/30 dark:border-white/10 light:bg-white/30 light:border-black/10 pr-10"
              disabled={isConnected || connecting}
              placeholder="http://192.168.1.250"
            />
            <Server className="absolute right-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-muted-foreground" />
          </div>
          {/* Hiding the /api append message as it might be confusing if URL already has it */}
          {/* <p className="text-xs text-muted-foreground mt-1">The path "/api" will be automatically appended</p> */}
        </div>
        
        <CollapsibleSection 
          title={<T>System Information</T>} 
          initiallyOpen={false} 
          storageKey="connPanelSysInfoOpen"
          className="mt-3 md:mt-4" // Added some margin top for spacing
          onStateChange={(isOpen) => {
            setAutoRefreshSystemInfo(isOpen);
          }}
        >
          <SystemInfoPanel systemInfo={systemInfo} formatUptime={formatUptime} />
        </CollapsibleSection>
        
        <DisplayMessagesPanel />
        
        <div className="flex justify-center flex-wrap gap-x-3 gap-y-2 pt-2">
          {isConnected ? (
            <>
              <Button
                variant="outline"
                size="sm"
                onClick={() => openApiUrl('/#/dashboard')}
                className="flex items-center gap-1 md:gap-2 text-xs md:text-sm px-2 md:px-3"
              >
                <List className="h-3.5 w-3.5 md:h-4 md:w-4" /> 
                <span><T>Dashboard</T></span>
              </Button>
              
              <Button
                variant="outline"
                size="sm"
                onClick={() => openApiUrl('/#/')}
                className="flex items-center gap-1 md:gap-2 text-xs md:text-sm px-2 md:px-3"
              >
                <Activity className="h-3.5 w-3.5 md:h-4 md:w-4" />
                <span><T>Plunger</T></span>
              </Button>

              <Button
                variant="outline"
                size="sm"
                onClick={handleReset}
                className="flex items-center gap-1 md:gap-2 text-xs md:text-sm px-2 md:px-3"
              >
                <RotateCw className="h-3.5 w-3.5 md:h-4 md:w-4" />
                <span><T>Reset</T></span>
              </Button>

              <Button 
                variant="destructive" 
                size="sm" 
                onClick={handleDisconnect}
                className="flex items-center gap-1 md:gap-2 text-xs md:text-sm px-2 md:px-3"
              >
                <Power className="h-3.5 w-3.5 md:h-4 md:w-4" />
                <span><T>Disconnect</T></span>
              </Button>
            </>
          ) : (
            <Button 
              variant="default" 
              size="sm" 
              onClick={handleConnect}
              disabled={connecting}
              className="bg-primary text-white flex items-center gap-1 md:gap-2 text-xs md:text-sm px-2 md:px-3"
            >
              {connecting ? (
                <Loader2 className="h-3.5 w-3.5 md:h-4 md:w-4 animate-spin" />
              ) : (
                <Plug className="h-3.5 w-3.5 md:h-4 md:w-4" />
              )}
              <span>{connecting ? <T>Connecting...</T> : <T>Connect</T>}</span>
            </Button>
          )}
        </div>
      </div>
    </div>
  );
};

export default ConnectionPanel;
