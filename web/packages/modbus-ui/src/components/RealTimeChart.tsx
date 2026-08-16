import React, { useState, useEffect, useMemo, useRef, useCallback } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import { useModbus } from '@/contexts/ModbusContext';
import { useFavorites } from '@/hooks/useFavorites';
import { T } from '../i18n';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Checkbox } from '@/components/ui/checkbox';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { SeriesSettingsModal, SeriesConfig } from './SeriesSettingsModal';
import { Download, ExternalLink, Trash2, Save, Upload, Star } from 'lucide-react';
import { useLocation, useNavigate } from 'react-router-dom';
import { toast } from 'sonner';

const OLD_CHART_SETTINGS_KEY = 'realTimeChartSettings';
const CHART_PROFILES_KEY = 'realTimeChartProfiles';
const ACTIVE_PROFILE_KEY = 'activeChartProfileName';
const DEFAULT_PROFILE_KEY = 'defaultChartProfileName';
const CHART_DATA_KEY = 'realTimeChartData';

const defaultColors = [
  '#00FF00', '#FFFF00', '#0000FF', '#FF0000', '#00FFFF', '#FF00FF',
  '#C0C0C0', '#808080', '#800000', '#800000', '#008000', '#800080'
];

const createDefaultSeries = (): SeriesConfig[] => {
  return Array.from({ length: 12 }, (_, i) => ({
    id: i + 1,
    enabled: i === 0,
    color: defaultColors[i % defaultColors.length],
    offset: 0,
    scale: 1,
    title: '',
    address: i === 0 ? 4 : 0,
    fileName: i === 0 ? 'Mbpoll1' : '',
    source: 'register',
  }));
};

interface ChartSettings {
  duration: number;
  yLeftMax: number;
  yLeftMin: number;
  showLegend: boolean;
  series: SeriesConfig[];
  refreshRateMs: number;
}

const createDefaultChartSettings = (): ChartSettings => ({
  duration: 600,
  yLeftMax: 100,
  yLeftMin: 0,
  showLegend: true,
  series: createDefaultSeries(),
  refreshRateMs: 1000,
});


const RealTimeChart: React.FC = () => {
  const location = useLocation();
  const navigate = useNavigate();
  const isPopOut = location.pathname.includes('/chart-viewer');
  
  const { registers, coils, isConnected } = useModbus();
  const { favoriteRegisters, favoriteCoils } = useFavorites();
  
  const [profiles, setProfiles] = useState<Record<string, ChartSettings>>(() => {
    const savedProfiles = localStorage.getItem(CHART_PROFILES_KEY);
    if (savedProfiles) {
      return JSON.parse(savedProfiles);
    }

    const oldSettings = localStorage.getItem(OLD_CHART_SETTINGS_KEY);
    if (oldSettings) {
      try {
        const parsed = JSON.parse(oldSettings);
        const migratedSeries = parsed.series.map((s: any) => ({
          ...s,
          source: s.source || 'register',
          scale: s.scale ?? 1,
          useRightYAxis: undefined, 
        }));
        const migratedSettings = {
          ...parsed,
          refreshRateMs: parsed.refreshRateMs || 1000,
          series: migratedSeries,
          yRightMax: undefined,
          yRightMin: undefined,
        };
        localStorage.removeItem(OLD_CHART_SETTINGS_KEY);
        return { 'Default': migratedSettings };
      } catch {
        return { 'Default': createDefaultChartSettings() };
      }
    }
    
    return { 'Default': createDefaultChartSettings() };
  });

  const [activeProfileName, setActiveProfileName] = useState<string>(() => {
    const savedActive = localStorage.getItem(ACTIVE_PROFILE_KEY);
    const defaultProfile = localStorage.getItem(DEFAULT_PROFILE_KEY);
    return savedActive || defaultProfile || 'Default';
  });

  const [newProfileName, setNewProfileName] = useState('');
  const importInputRef = useRef<HTMLInputElement>(null);

  const settings = useMemo(() => {
    return profiles[activeProfileName] || createDefaultChartSettings();
  }, [profiles, activeProfileName]);

  const updateSettings = useCallback((updater: (prev: ChartSettings) => ChartSettings) => {
    const currentSettings = profiles[activeProfileName];
    if (currentSettings) {
      const newSettings = updater(currentSettings);
      setProfiles(p => ({ ...p, [activeProfileName]: newSettings }));
    }
  }, [profiles, activeProfileName]);


  const [isStarted, setIsStarted] = useState(false);
  const [chartData, setChartData] = useState<any[]>(() => {
    const savedData = sessionStorage.getItem(CHART_DATA_KEY);
    return savedData ? JSON.parse(savedData) : [];
  });
  const lastValuesRef = useRef<Record<number, number | boolean>>({});
  const hasAutoStarted = useRef(false);

  useEffect(() => {
    localStorage.setItem(CHART_PROFILES_KEY, JSON.stringify(profiles));
  }, [profiles]);

  useEffect(() => {
    localStorage.setItem(ACTIVE_PROFILE_KEY, activeProfileName);
    // Ensure the active profile exists, otherwise reset
    if (!profiles[activeProfileName]) {
      const defaultProfile = localStorage.getItem(DEFAULT_PROFILE_KEY) || 'Default';
      setActiveProfileName(profiles[defaultProfile] ? defaultProfile : Object.keys(profiles)[0] || 'Default');
    }
  }, [activeProfileName, profiles]);

  useEffect(() => {
    sessionStorage.setItem(CHART_DATA_KEY, JSON.stringify(chartData));
  }, [chartData]);


  // Polling implementation
  useEffect(() => {
    if (!isStarted || !isConnected) return () => {};

    const intervalId = setInterval(() => {
      const now = Date.now();
      const newDataPoint: any = { time: now };
      let hasNewData = false;

      settings.series.forEach(s => {
        if (s.enabled && s.address > 0) {
          let currentValue: number | boolean | undefined;
          let processedValue: number | undefined;

          if (s.source === 'coil') {
            const coil = coils.find(c => c.address === s.address);
            currentValue = coil?.value;
          } else {
            const register = registers.find(r => r.address === s.address);
            currentValue = register?.value;
          }

          if (currentValue !== undefined) {
             hasNewData = true;
             if (s.source === 'coil') {
                processedValue = currentValue ? settings.yLeftMax : settings.yLeftMax * 0.1;
             } else { // register
                processedValue = currentValue === 65535 ? 0 : currentValue as number;
             }
             newDataPoint[`series_${s.id}`] = (processedValue * (s.scale ?? 1)) + (s.offset || 0);
          }
        }
      });
      
      if (hasNewData) {
        setChartData(prevData => {
            const windowMs = (settings.duration || 600) * 1000;
            const cutoff = Date.now() - windowMs;
            const updatedData = [...prevData, newDataPoint];
            return updatedData.filter(d => d.time >= cutoff);
        });
      }
    }, settings.refreshRateMs);

    return () => clearInterval(intervalId);
  }, [isStarted, isConnected, registers, coils, settings]);


  const handleStartStop = () => {
    setIsStarted(!isStarted);
  }

  const handleClearChart = () => {
    setChartData([]);
    sessionStorage.removeItem(CHART_DATA_KEY);
    lastValuesRef.current = {};
  }

  const handlePopOut = () => {
    window.open('/#/chart-viewer', '_blank');
  };

  const getLineName = useCallback((s: SeriesConfig) => {
    if (s.title) return s.title;
    if (s.source === 'coil') {
      const coil = coils.find(c => c.address === s.address);
      if (coil && coil.name) return `${coil.group || 'Default'}::${coil.name}`;
    } else {
      const register = registers.find(r => r.address === s.address);
      if (register && register.name) return `${register.group || 'Default'}::${register.name}`;
    }
    if (s.address > 0) return `${s.source === 'coil' ? 'Coil' : 'Register'} ${s.address}`;
    return `Series ${s.id}`;
  }, [coils, registers]);

  const handleExportCSV = useCallback(() => {
    if (chartData.length === 0) {
      console.warn("No data to export.");
      return;
    }

    const enabledSeries = settings.series.filter(s => s.enabled && s.address > 0);
    const header = ["Time (s)", ...enabledSeries.map(s => getLineName(s))];
    
    const rows = chartData.map(dp => {
      const row = [new Date(dp.time).toISOString()];
      enabledSeries.forEach(s => {
        const value = dp[`series_${s.id}`];
        row.push(typeof value === 'number' ? value.toFixed(2) : '');
      });
      return row.join(',');
    });

    const csvContent = [header.join(','), ...rows].join('\n');
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const link = document.createElement('a');
    const url = URL.createObjectURL(blob);
    link.setAttribute('href', url);
    const n = new Date();
    const dS = `${n.getFullYear()}-${String(n.getMonth() + 1).padStart(2, '0')}-${String(n.getDate()).padStart(2, '0')}`;
    const tS = `${String(n.getHours()).padStart(2, '0')}${String(n.getMinutes()).padStart(2, '0')}${String(n.getSeconds()).padStart(2, '0')}`;
    link.setAttribute('download', `chart_data_${dS}_${tS}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  }, [chartData, settings.series, getLineName]);

  const setSeries = (arg: React.SetStateAction<SeriesConfig[]>) => {
    updateSettings(prev => {
      const newSeries = typeof arg === 'function' ? arg(prev.series) : arg;
      return { ...prev, series: newSeries };
    });
  }

  const handleSaveAs = () => {
    const name = newProfileName.trim();
    if (!name) {
      toast.error("Please enter a profile name.");
      return;
    }
    if (profiles[name]) {
      toast.warning(`Profile "${name}" already exists. Please choose another name.`);
      return;
    }
    setProfiles(p => ({...p, [name]: settings }));
    setActiveProfileName(name);
    setNewProfileName('');
    toast.success(`Profile "${name}" saved.`);
  };

  const handleDeleteProfile = () => {
    if (Object.keys(profiles).length <= 1) {
      toast.error("Cannot delete the last profile.");
      return;
    }
    if (window.confirm(`Are you sure you want to delete profile "${activeProfileName}"?`)) {
      const newProfiles = { ...profiles };
      delete newProfiles[activeProfileName];
      setProfiles(newProfiles);
      
      const defaultProfileName = localStorage.getItem(DEFAULT_PROFILE_KEY) || 'Default';
      // If we deleted the default profile, reset it
      if (activeProfileName === defaultProfileName) {
        localStorage.removeItem(DEFAULT_PROFILE_KEY);
      }
      
      toast.info(`Profile "${activeProfileName}" deleted.`);
    }
  };

  const handleSetDefault = () => {
    localStorage.setItem(DEFAULT_PROFILE_KEY, activeProfileName);
    toast.success(`"${activeProfileName}" is now the default profile.`);
  };

  const handleExportProfile = () => {
    const jsonString = JSON.stringify(settings, null, 2);
    const blob = new Blob([jsonString], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `${activeProfileName}.json`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  };

  const handleImportProfile = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const text = e.target?.result as string;
        const importedSettings = JSON.parse(text);
        // Basic validation could be added here
        
        let profileName = file.name.replace(/\.json$/, "");
        let finalName = profileName;
        let i = 1;
        while(profiles[finalName]) {
          finalName = `${profileName}-${i++}`;
        }

        setProfiles(p => ({ ...p, [finalName]: importedSettings }));
        setActiveProfileName(finalName);
        toast.success(`Profile "${finalName}" imported successfully.`);
      } catch (error) {
        toast.error("Failed to import profile. Invalid JSON file.");
      }
    };
    reader.readAsText(file);
    event.target.value = ''; // Reset file input
  };
  
  const seriesWithAddress = useMemo(() => settings.series.filter(s => s.address > 0), [settings.series]);

  const groupedSeriesForToggle = useMemo(() => {
    return seriesWithAddress.reduce((acc, series) => {
        let item;
        if (series.source === 'register') {
            item = registers.find(r => r.address === series.address);
        } else {
            item = coils.find(c => c.address === series.address);
        }

        const groupName = item?.group || 'Uncategorized';
        
        if (!acc[groupName]) {
            acc[groupName] = [];
        }
        
        acc[groupName].push(series);
        
        return acc;
    }, {} as Record<string, SeriesConfig[]>);
  }, [seriesWithAddress, registers, coils]);

  const handleToggleSeriesVisibility = (seriesId: number) => {
    updateSettings(prev => ({
        ...prev,
        series: prev.series.map(s => s.id === seriesId ? { ...s, enabled: !s.enabled } : s)
    }));
  };

  return (
    <Card>
      <CardHeader>
        <CardTitle><T>Real time Charting</T></CardTitle>
      </CardHeader>
      <CardContent className="space-y-4">
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          <div className="border p-2 rounded-md">
            <h3 className="font-semibold text-sm mb-2"><T>X-Axis</T></h3>
            <div className="flex items-center gap-2">
              <Input
                type="number"
                value={settings.duration}
                onChange={e => updateSettings(s => ({ ...s, duration: Number(e.target.value) || 0 }))}
                onBlur={() => {
                    if (settings.duration < 10) {
                        updateSettings(s => ({...s, duration: 10}));
                    }
                }}
                className="w-24"
              />
              <span>s</span>
            </div>
            <label htmlFor="refresh-rate" className="text-xs font-medium mt-2 block"><T>Refresh Rate</T></label>
            <div className="flex items-center gap-2">
              <Input 
                type="number" 
                id="refresh-rate" 
                value={settings.refreshRateMs} 
                onChange={e => updateSettings(s=>({...s, refreshRateMs: Number(e.target.value) || 1000}))} 
                onBlur={() => {
                  if (settings.refreshRateMs < 50) {
                    updateSettings(s => ({...s, refreshRateMs: 50}));
                  }
                }}
                step="50" 
                min="50"
                className="w-24" 
              />
              <span>ms</span>
            </div>
            <Select defaultValue="continue">
              <SelectTrigger className="mt-2">
                <SelectValue placeholder="Mode" />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="continue"><T>Continue</T></SelectItem>
                <SelectItem value="stop"><T>Stop at end</T></SelectItem>
                <SelectItem value="restart"><T>Restart at end</T></SelectItem>
              </SelectContent>
            </Select>
          </div>

          <div className="border p-2 rounded-md">
            <h3 className="font-semibold text-sm mb-2"><T>Y-Axis Left</T></h3>
            <div className="space-y-2">
              <div className="flex items-center gap-2">
                <span className="text-xs w-8"><T>Max</T></span>
                <Input type="number" value={settings.yLeftMax} onChange={e => updateSettings(s=>({...s, yLeftMax: Number(e.target.value)}))} />
              </div>
              <div className="flex items-center gap-2">
                <span className="text-xs w-8"><T>Min</T></span>
                <Input type="number" value={settings.yLeftMin} onChange={e => updateSettings(s=>({...s, yLeftMin: Number(e.target.value)}))} />
              </div>
            </div>
          </div>

          <div className="border p-2 rounded-md">
            <h3 className="font-semibold text-sm mb-2"><T>Series</T></h3>
            <div className="space-y-2">
              <SeriesSettingsModal
                series={settings.series}
                setSeries={setSeries}
                registers={registers}
                coils={coils}
                favoriteRegisters={favoriteRegisters}
                favoriteCoils={favoriteCoils}
              >
                <Button variant="outline" className="w-full"><T>Settings...</T></Button>
              </SeriesSettingsModal>
              <Button variant="outline" className="w-full"><T>Copy...</T></Button>
            </div>
          </div>

          <div className="border p-2 rounded-md flex flex-col justify-between">
            <div className="flex items-center space-x-2">
                <Checkbox id="show-legend" checked={settings.showLegend} onCheckedChange={(checked) => updateSettings(s=>({...s, showLegend: Boolean(checked)}))} />
                <label htmlFor="show-legend" className="text-sm font-medium"><T>Show Legend</T></label>
            </div>
            <div className="flex flex-col space-y-2 mt-4">
                <Button variant="outline"><T>Apply</T></Button>
                <Button onClick={handleStartStop} disabled={!isConnected}>
                  {isStarted ? <T>Stop</T> : <T>Start</T>}
                </Button>
                <div className="flex gap-2 pt-2">
                    <Button variant="outline" size="sm" onClick={handleExportCSV} className="w-full flex items-center gap-1" disabled={chartData.length === 0}>
                        <Download className="h-4 w-4" /> <T>CSV</T>
                    </Button>
                    {!isPopOut && (
                      <Button variant="outline" size="sm" onClick={handlePopOut} className="w-full flex items-center gap-1">
                          <ExternalLink className="h-4 w-4" /> <T>Pop-out</T>
                      </Button>
                    )}
                </div>
                <Button variant="outline" size="sm" onClick={handleClearChart} className="w-full flex items-center gap-1 mt-2">
                    <Trash2 className="h-4 w-4" /> <T>Clear</T>
                </Button>
            </div>
          </div>
        </div>

        <Card className="mt-4">
          <CardHeader>
            <CardTitle><T>Profiles</T></CardTitle>
          </CardHeader>
          <CardContent className="space-y-4">
            <div className="flex flex-wrap items-center gap-2">
              <Select value={activeProfileName} onValueChange={setActiveProfileName}>
                <SelectTrigger className="w-[180px]">
                  <SelectValue placeholder="Select a profile" />
                </SelectTrigger>
                <SelectContent>
                  {Object.keys(profiles).map(name => (
                    <SelectItem key={name} value={name}>{name}</SelectItem>
                  ))}
                </SelectContent>
              </Select>
              <div className="flex gap-2 flex-grow">
                <Input 
                  placeholder="New profile name..." 
                  value={newProfileName}
                  onChange={e => setNewProfileName(e.target.value)}
                  className="flex-grow"
                />
                <Button onClick={handleSaveAs}><Save className="h-4 w-4 mr-2" /><T>Save As</T></Button>
              </div>
            </div>
            <div className="flex flex-wrap items-center gap-2">
                <Button variant="outline" onClick={handleSetDefault}><Star className="h-4 w-4 mr-2" /><T>Set as Default</T></Button>
                <Button variant="destructive" onClick={handleDeleteProfile} disabled={Object.keys(profiles).length <= 1}><Trash2 className="h-4 w-4 mr-2" /><T>Delete</T></Button>
                <Button variant="outline" onClick={handleExportProfile}><Download className="h-4 w-4 mr-2" /><T>Export</T></Button>
                <Button variant="outline" onClick={() => importInputRef.current?.click()}><Upload className="h-4 w-4 mr-2" /><T>Import</T></Button>
                <input type="file" ref={importInputRef} onChange={handleImportProfile} accept=".json" className="hidden" />
            </div>
          </CardContent>
        </Card>

        <div className="h-96 rounded-md border bg-muted/20 p-2 dark:bg-black/20">
          <ResponsiveContainer width="100%" height="100%">
            <LineChart data={chartData} margin={{ top: 5, right: 30, left: 0, bottom: 5 }}>
              <CartesianGrid strokeDasharray="3 3" stroke="hsl(var(--border))" />
              <XAxis 
                dataKey="time" 
                type="number" 
                domain={[
                  (dataMin: number) => {
                    const now = Date.now();
                    const windowMs = (settings.duration || 600) * 1000;
                    return now - windowMs;
                  },
                  (dataMax: number) => Date.now()
                ]} 
                stroke="hsl(var(--muted-foreground))"
                tickFormatter={(unixTime) => new Date(unixTime).toLocaleTimeString()}
              />
              <YAxis 
                yAxisId="left" 
                stroke="hsl(var(--muted-foreground))" 
                domain={[settings.yLeftMin, settings.yLeftMax]} 
                allowDataOverflow={false} 
                tickCount={11}
              />
              <Tooltip
                contentStyle={{
                  backgroundColor: 'hsl(var(--popover))',
                  color: 'hsl(var(--popover-foreground))',
                  border: '1px solid hsl(var(--border))',
                }}
                labelStyle={{ fontWeight: 'bold' }}
                labelFormatter={(label) => new Date(label).toLocaleString()}
                formatter={(value: number) => value.toFixed(2)}
              />
              {settings.showLegend && <Legend wrapperStyle={{ paddingTop: '10px' }}/>}
              {settings.series.map(s => {
                  if (!s.enabled) return null;
                  return (
                    <Line
                      key={s.id}
                      yAxisId="left"
                      type={s.source === 'coil' ? 'stepAfter' : 'monotone'}
                      dataKey={`series_${s.id}`}
                      stroke={s.color}
                      name={getLineName(s)}
                      dot={false}
                      isAnimationActive={false}
                      connectNulls={true}
                    />
                  );
                })}
            </LineChart>
          </ResponsiveContainer>
        </div>

        {seriesWithAddress.length > 0 && (
            <div className="border-t pt-4">
                <h3 className="text-sm font-semibold mb-2"><T>Series Toggles</T></h3>
                <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-6 gap-x-4 gap-y-1">
                    {Object.entries(groupedSeriesForToggle).sort(([a], [b]) => a.localeCompare(b)).map(([groupName, seriesInGroup]) => (
                        <div key={groupName} className="mb-2">
                            <h4 className="text-xs font-bold text-muted-foreground mb-1">{groupName}</h4>
                            {seriesInGroup.map(s => (
                                <div key={s.id} className="flex items-center space-x-2 py-0.5">
                                    <Checkbox 
                                        id={`toggle-${s.id}`} 
                                        checked={s.enabled}
                                        onCheckedChange={() => handleToggleSeriesVisibility(s.id)}
                                        style={{ color: s.color }}
                                        className="border-current"
                                    />
                                    <label htmlFor={`toggle-${s.id}`} className="text-xs cursor-pointer truncate" title={getLineName(s)}>
                                        {getLineName(s)}
                                    </label>
                                </div>
                            ))}
                        </div>
                    ))}
                </div>
            </div>
        )}

      </CardContent>
    </Card>
  );
};

export default RealTimeChart; 