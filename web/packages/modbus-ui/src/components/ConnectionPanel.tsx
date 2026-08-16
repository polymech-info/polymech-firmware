import { useState, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Server } from 'lucide-react';
import SystemInfoPanel from './SystemInfoPanel';
import { T, translate } from '../i18n';
import CollapsibleSection from './CollapsibleSection';
import DisplayMessagesPanel from './DisplayMessagesPanel';

const ConnectionPanel = () => {
  const {
    apiUrl,
    systemInfo,
    featureFlags,
    getFeatureFlags,
    fetchSystemInfo
  } = useModbus();

  const [serverUrl, setServerUrl] = useState(apiUrl.replace('/api', ''));

  const handleDownloadRequest = async (): Promise<Record<string, any>> => {
    await getFeatureFlags();
    return Promise.resolve(featureFlags);
  };

  const formatUptime = (seconds: number): string => {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const remainingSeconds = seconds % 60;

    return `${days}d ${hours}h ${minutes}m ${remainingSeconds}s`;
  };

  useEffect(() => {
    setServerUrl(apiUrl.replace('/api', ''));
  }, [apiUrl]);

  useEffect(() => {
    fetchSystemInfo();
  }, [fetchSystemInfo]);

  return (
    <div className="connection-panel space-y-2">
      <div className="glass-card p-3 space-y-2">
        <Label htmlFor="apiUrl" className="text-xs font-mono text-slate-600 dark:text-slate-300"><T>API URL</T></Label>
        <div className="relative flex items-center">
          <Input
            id="apiUrl"
            value={serverUrl}
            onChange={(e) => setServerUrl(e.target.value)}
            className="glass-input font-mono text-sm md:text-base pr-10"
            placeholder="http://192.168.239.250/"
            readOnly
          />
          <Server className="absolute right-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-slate-500 dark:text-white/60" />
        </div>
      </div>

      <CollapsibleSection
        title={translate("System Information")}
        initiallyOpen={false}
        storageKey="connPanelSysInfoOpen"
        id="system-info-collapsible"
        minimal={true}
      >
        <SystemInfoPanel systemInfo={systemInfo} formatUptime={formatUptime} onDownloadRequest={handleDownloadRequest} />
      </CollapsibleSection>

      <DisplayMessagesPanel id="display-messages-panel" />

    </div>
  );
};

export default ConnectionPanel; 