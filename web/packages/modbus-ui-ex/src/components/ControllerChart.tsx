import React, { useEffect, useMemo, useRef, useState } from 'react';
import { ResponsiveContainer, LineChart, CartesianGrid, XAxis, YAxis, Tooltip, Line, Brush } from 'recharts';
import { useModbus } from '@/contexts/ModbusContext';
import { PlotStatus } from '@/types';
import { PARTITION_CONFIG, getSlaveIdFromGroup, ControllerConfig } from '@/lib/controllerUtils';
import { T } from '../i18n';
import { Button } from '@/components/ui/button';
import { Download, RotateCcw, ZoomOut } from 'lucide-react';
import { PV_REGISTER_NAME_SUFFIX, SP_REGISTER_NAME_SUFFIX } from '@/constants';


const MAX_DATA_POINTS = 1200;
const DEFAULT_WINDOW_MINUTES = 10;
const MIN_WINDOW_MINUTES = 1;
const MAX_WINDOW_MINUTES = 120;
const LOCALSTORAGE_SETTINGS_KEY = 'controllerChartSettings';
const SESSION_CHART_DATA_KEY = 'controllerChartSessionData';
const SESSION_FAULTY_IDS_KEY = 'controllerChartSessionFaultyIds';
const ACTIVE_PROFILE_MEASURED_TEMP_DATAKEY = 'active_profile_measured_temp';

interface ChartSettings {
  visibleControllerSlaveIds: number[];
  showPV: boolean;
  showSP: boolean;
  dataWindowMinutes: number;
  showProfileLine: boolean;
}

interface ChartDataPoint { timestamp: number; [key: string]: number | string; }
interface ControllerLineInfo { id: string; name: string; slaveId: number; type: 'PV' | 'SP'; color: string; }
interface UniqueController { slaveId: number; name: string;}
interface PartitionDisplayInfo { name: string; controllers: UniqueController[];}
const generateColor = (index: number, type: 'PV' | 'SP' | 'PROFILE'): string => {
    if (type === 'PROFILE') return '#4ade80';
    const pvColors = ['#8884d8', '#ff7300', '#00C49F', '#FF8042', '#A4DE6C', '#FF6B6B', '#5470C6', '#FAC858', '#EF5350', '#26A69A', '#66BB6A', '#EC407A', '#29B6F6'];
    const spColors = ['#82ca9d', '#ffc658', '#FFBB28', '#0088FE', '#D0ED57', '#4ECDC4', '#91CC75', '#AB47BC', '#FFA726', '#5C6BC0', '#78909C', '#FFEE58'];
    return type === 'PV' ? pvColors[index % pvColors.length] : spColors[index % spColors.length];
};

const defaultChartSettings: ChartSettings = {
  visibleControllerSlaveIds: [],
  showPV: true,
  showSP: true,
  dataWindowMinutes: DEFAULT_WINDOW_MINUTES,
  showProfileLine: true,
};

const ControllerChart: React.FC = () => {
  const { registers: allModbusRegisters, isConnected, profiles } = useModbus();
  const lastUpdateTimeRef = useRef<number>(Date.now());

  const loadInitialSettings = (): ChartSettings => {
    const saved = localStorage.getItem(LOCALSTORAGE_SETTINGS_KEY);
    if (saved) {
        try {
            const parsed = JSON.parse(saved) as Partial<ChartSettings>;
            return { ...defaultChartSettings, ...parsed };
        } catch (e) { console.error("Error parsing chart settings:", e); }
    }
    return { ...defaultChartSettings };
  };
  const initialSettings = useMemo(loadInitialSettings, []);

  const [showPV, setShowPV] = useState<boolean>(initialSettings.showPV);
  const [showSP, setShowSP] = useState<boolean>(initialSettings.showSP);
  const [dataWindowMinutes, setDataWindowMinutes] = useState<number>(initialSettings.dataWindowMinutes);
  const [showProfileLine, setShowProfileLine] = useState<boolean>(initialSettings.showProfileLine);

  const [chartData, setChartData] = useState<ChartDataPoint[]>(() => JSON.parse(sessionStorage.getItem(SESSION_CHART_DATA_KEY) || '[]'));
  const [faultySlaveIds, setFaultySlaveIds] = useState<Set<number>>(new Set());
  const [processedChartData, setProcessedChartData] = useState<ChartDataPoint[]>([]);
  const [xZoomDomain, setXZoomDomain] = useState<[number, number] | null>(null);
  
  const partitionedControllers = useMemo((): PartitionDisplayInfo[] => {
    const result: PartitionDisplayInfo[] = [];
    PARTITION_CONFIG.forEach(partition => {
      const pUniqueCtrlrs: UniqueController[] = [];
      let currentPCs: ControllerConfig[] = partition.controllers || [];
      if (!partition.controllers && partition.startSlaveId !== undefined && partition.numControllers !== undefined) {
        currentPCs = [];
        for (let i = 0; i < partition.numControllers; i++) currentPCs.push({ slaveId: partition.startSlaveId + i, name: `Ctrl ${partition.startSlaveId + i}` });
      }
      const seenInP = new Set<number>();
      currentPCs.forEach(c => { if (!seenInP.has(c.slaveId)) { pUniqueCtrlrs.push({ slaveId: c.slaveId, name: c.name || `Ctrl ${c.slaveId}` }); seenInP.add(c.slaveId); }});
      if (pUniqueCtrlrs.length > 0) result.push({ name: partition.name, controllers: pUniqueCtrlrs });
    });
    return result;
  }, []);

  //console.log("partitionedControllers", partitionedControllers);
  const allUniqueControllersFlat = useMemo((): UniqueController[] => partitionedControllers.flatMap(p => p.controllers), [partitionedControllers]);
  const activeProfile = useMemo(() => profiles?.find(p => p.status === PlotStatus.RUNNING || p.status === PlotStatus.PAUSED), [profiles]);

  const [visibleControllerSlaveIds, setVisibleControllerSlaveIds] = useState<Set<number>>(() => {
    const defaultVis = new Set(allUniqueControllersFlat.map(c => c.slaveId));
    const savedVisIds = initialSettings.visibleControllerSlaveIds;
    if (savedVisIds && Array.isArray(savedVisIds)) {
        const existingIds = new Set(allUniqueControllersFlat.map(c => c.slaveId));
        const validIds = savedVisIds.filter(id => existingIds.has(id));
        return validIds.length > 0 || savedVisIds.length === 0 ? new Set(validIds) : defaultVis; 
    }
    return defaultVis;
  });
  
  useEffect(() => {
    localStorage.setItem(LOCALSTORAGE_SETTINGS_KEY, JSON.stringify({ visibleControllerSlaveIds: Array.from(visibleControllerSlaveIds), showPV, showSP, dataWindowMinutes, showProfileLine } as ChartSettings));
  }, [visibleControllerSlaveIds, showPV, showSP, dataWindowMinutes, showProfileLine]);
  useEffect(() => { sessionStorage.setItem(SESSION_CHART_DATA_KEY, JSON.stringify(chartData)); }, [chartData]);
  useEffect(() => { sessionStorage.setItem(SESSION_FAULTY_IDS_KEY, JSON.stringify(Array.from(faultySlaveIds))); }, [faultySlaveIds]);

  const allControllerLines = useMemo((): ControllerLineInfo[] => {
    const lines: ControllerLineInfo[] = [];
    const cMap = new Map<number, number>();
    allUniqueControllersFlat.forEach((c, i) => cMap.set(c.slaveId, i));
    partitionedControllers.forEach(p => p.controllers.forEach(c => {
      const cIdx = cMap.get(c.slaveId) ?? 0;
      lines.push({ id: `s${c.slaveId}_PV`, name: `${c.name} PV`, slaveId: c.slaveId, type: 'PV', color: generateColor(cIdx, 'PV') });
      lines.push({ id: `s${c.slaveId}_SP`, name: `${c.name} SP`, slaveId: c.slaveId, type: 'SP', color: generateColor(cIdx, 'SP') });
    }));
    return lines;
  }, [partitionedControllers, allUniqueControllersFlat]);

  useEffect(() => {
    if (!isConnected || !allModbusRegisters || allModbusRegisters.length === 0) return;
    const newFaulty = new Set(faultySlaveIds);
    const freshFaultyIds = new Set<number>();
    allUniqueControllersFlat.forEach(c => {
      const pv = allModbusRegisters.find(r => getSlaveIdFromGroup(r.group) === c.slaveId && r.name.endsWith(PV_REGISTER_NAME_SUFFIX));
      const sp = allModbusRegisters.find(r => getSlaveIdFromGroup(r.group) === c.slaveId && r.name.endsWith(SP_REGISTER_NAME_SUFFIX));
      if ((pv?.value !== undefined && typeof pv.value === 'number' && pv.value > 400) || (sp?.value !== undefined && typeof sp.value === 'number' && sp.value > 400)) {
        freshFaultyIds.add(c.slaveId);
      }
    });
    if (freshFaultyIds.size !== faultySlaveIds.size || ![...freshFaultyIds].every(id => faultySlaveIds.has(id))) {
        setFaultySlaveIds(freshFaultyIds);
    }

    const now = Date.now();
    if (now - lastUpdateTimeRef.current < 1000 && chartData.length > 0 && !(activeProfile && showProfileLine && processedChartData.length === 0)) return;
    lastUpdateTimeRef.current = now;

    const newData: ChartDataPoint = { timestamp: now };
    let hasData = false;
    
    allUniqueControllersFlat.forEach(c => {
      if (faultySlaveIds.has(c.slaveId) || !visibleControllerSlaveIds.has(c.slaveId)) return;
      const pv = allModbusRegisters.find(r => getSlaveIdFromGroup(r.group) === c.slaveId && r.name.endsWith(PV_REGISTER_NAME_SUFFIX));
      const sp = allModbusRegisters.find(r => getSlaveIdFromGroup(r.group) === c.slaveId && r.name.endsWith(SP_REGISTER_NAME_SUFFIX));
      if (showPV && pv?.value !== undefined && typeof pv.value === 'number' && pv.value >= 0 && pv.value <= 400) { newData[`s${c.slaveId}_PV`] = pv.value; hasData = true; }
      if (showSP && sp?.value !== undefined && typeof sp.value === 'number' && sp.value >= 0 && sp.value <= 400) { newData[`s${c.slaveId}_SP`] = sp.value; hasData = true; }
    });

        if (hasData || (activeProfile && showProfileLine)) {
      setChartData(prev => {
        const updated = [...prev, newData];
        const windowMs = dataWindowMinutes * 60 * 1000;
        const cutoff = lastUpdateTimeRef.current - windowMs;
        const filtered = updated.filter(d => d.timestamp >= cutoff);
        return filtered.length > MAX_DATA_POINTS ? filtered.slice(filtered.length - MAX_DATA_POINTS) : filtered;
      });
    }
  }, [allModbusRegisters, isConnected, visibleControllerSlaveIds, dataWindowMinutes, allUniqueControllersFlat, showPV, showSP, activeProfile, showProfileLine, chartData.length, processedChartData.length, faultySlaveIds]);

  useEffect(() => {
    let profileMeasuredTemp: number | undefined = undefined;
    if (activeProfile && showProfileLine && activeProfile.currentTemp !== undefined && typeof activeProfile.currentTemp === 'number') {
      if (activeProfile.currentTemp >= 0 && activeProfile.currentTemp <= 400) {
        profileMeasuredTemp = activeProfile.currentTemp;
      }
    }

    if (chartData.length === 0 && processedChartData.length > 0) { 
        setProcessedChartData([]);
        return;
    }
    if (chartData.length === 0 && processedChartData.length === 0) {
        if(processedChartData.length > 0) setProcessedChartData([]);
        return;
    }

    setProcessedChartData(
        chartData.map(dp => ({
            ...dp,
            [ACTIVE_PROFILE_MEASURED_TEMP_DATAKEY]: profileMeasuredTemp
        }))
    );
  }, [chartData, activeProfile, showProfileLine]);

  const handleClearChart = () => { 
    setChartData([]); 
    setFaultySlaveIds(new Set()); 
    setProcessedChartData([]); 
    sessionStorage.removeItem(SESSION_CHART_DATA_KEY); 
    sessionStorage.removeItem(SESSION_FAULTY_IDS_KEY); 
    setXZoomDomain(null);
  };
  
  const handleResetZoom = () => {
    setXZoomDomain(null);
  };

  const handleExportCSV = () => {
    const dataExp = processedChartData;
    if (dataExp.length === 0) { console.warn("No data to export."); return; }
    const actCtrlLines = allControllerLines.filter(l => visibleControllerSlaveIds.has(l.slaveId) && !faultySlaveIds.has(l.slaveId) && ((l.type === 'PV' && showPV) || (l.type === 'SP' && showSP)));
    const hdr = ["Timestamp", ...actCtrlLines.map(l => l.name)];
    if (activeProfile && showProfileLine) hdr.push(`Profile: ${activeProfile.name} (Actual Temp)`);
    const rws = dataExp.map(dp => {
      const r = [new Date(dp.timestamp).toLocaleString(), ...actCtrlLines.map(l => { const v = dp[l.id]; return typeof v === 'number' ? v.toFixed(2) : (v ?? ''); })];
      if (activeProfile && showProfileLine) { 
        const pV = dp[ACTIVE_PROFILE_MEASURED_TEMP_DATAKEY]; 
        r.push(typeof pV === 'number' ? pV.toFixed(2) : (pV ?? '')); 
      }
      return r;
    });
    let csvStr = hdr.join(",") + "\n" + rws.map(r => r.join(",")).join("\n");
    const blb = new Blob([csvStr],{type:'text/csv;charset=utf-8;'}); const lnk=document.createElement("a");const u=URL.createObjectURL(blb); lnk.setAttribute("href",u); const n=new Date();const dS=`${n.getFullYear()}-${String(n.getMonth()+1).padStart(2,'0')}-${String(n.getDate()).padStart(2,'0')}`; const tS=`${String(n.getHours()).padStart(2,'0')}${String(n.getMinutes()).padStart(2,'0')}${String(n.getSeconds()).padStart(2,'0')}`; lnk.setAttribute("download",`chart_data_${dS}_${tS}.csv`); document.body.appendChild(lnk);lnk.click();document.body.removeChild(lnk);URL.revokeObjectURL(u);
  };

  const controllersForCheckboxes = partitionedControllers.map(p => ({ ...p, controllers: p.controllers.map(c => ({ ...c, isFaulty: faultySlaveIds.has(c.slaveId) })) }));

  if (!isConnected && processedChartData.length === 0 && allModbusRegisters.length === 0) return <div className="p-4 text-center text-muted-foreground"><T>Connect to a Modbus server to see controller chart data.</T></div>;
  if (isConnected && processedChartData.length === 0 && allModbusRegisters.length > 0 && allUniqueControllersFlat.length === 0) return <div className="p-4 text-center text-muted-foreground"><T>No controllers configured for charting.</T></div>;
  if (isConnected && processedChartData.length === 0 && chartData.length === 0 && allModbusRegisters.length > 0 && allUniqueControllersFlat.length > 0) return <div className="p-4 text-center text-muted-foreground"><T>Waiting for data...</T></div>;
  
  return (
    <div className="md:bg-card p-2 md:p-4 mt-2 rounded-lg shadow-lg space-y-4">
        <div className="pb-2 border-b border-border space-y-3">
            <div className="flex items-center space-x-6 gap-y-2 gap-x-2 mb-3 flex-wrap">
                <span className="text-sm font-medium whitespace-nowrap"><T>Global Settings</T>:</span>
                <label className="flex items-center space-x-1.5 cursor-pointer text-sm">
                    <input type="checkbox" checked={showPV} onChange={() => setShowPV(prev => !prev)} className="form-checkbox h-4 w-4 text-blue-600 rounded focus:ring-blue-500 dark:border-gray-600 dark:bg-gray-700 dark:focus:ring-offset-gray-800"/>
                    <span><T>Show PV</T></span>
                </label>
                <label className="flex items-center space-x-1.5 cursor-pointer text-sm">
                    <input type="checkbox" checked={showSP} onChange={() => setShowSP(prev => !prev)} className="form-checkbox h-4 w-4 text-green-600 rounded focus:ring-green-500 dark:border-gray-600 dark:bg-gray-700 dark:focus:ring-offset-gray-800"/>
                    <span><T>Show SP</T></span>
                </label>
                <label className="flex items-center space-x-1.5 cursor-pointer text-sm">
                    <input type="checkbox" checked={showProfileLine} onChange={() => setShowProfileLine(prev => !prev)} className="form-checkbox h-4 w-4 text-purple-600 rounded focus:ring-purple-500 dark:border-gray-600 dark:bg-gray-700 dark:focus:ring-offset-gray-800"/>
                    <span><T>Profile SP</T></span>
                </label>
                <div className="flex items-center space-x-1.5 text-sm">
                    <label htmlFor="chart-window-minutes" className="whitespace-nowrap"><T>Window (min)</T>:</label>
                    <input type="number" id="chart-window-minutes" value={dataWindowMinutes} onChange={(e) => { let v = parseInt(e.target.value, 10); if(isNaN(v))v=DEFAULT_WINDOW_MINUTES; else if(v<MIN_WINDOW_MINUTES)v=MIN_WINDOW_MINUTES; else if(v>MAX_WINDOW_MINUTES)v=MAX_WINDOW_MINUTES; setDataWindowMinutes(v);}} onBlur={(e) => {if(e.target.value==='')setDataWindowMinutes(DEFAULT_WINDOW_MINUTES);}} min={MIN_WINDOW_MINUTES} max={MAX_WINDOW_MINUTES} step={1} className="form-input h-7 w-20 rounded-md border-gray-300 shadow-sm focus:border-primary focus:ring focus:ring-primary-focus focus:ring-opacity-50 dark:bg-gray-700 dark:border-gray-600 dark:text-white text-sm p-1"/>
                </div>
            </div>
            <span className="text-sm font-medium mr-2 block"><T>Visible Controllers</T>:</span>
            {controllersForCheckboxes.map(partition => (
                <div key={partition.name} className="pl-2">
                    <h4 className="text-xs font-semibold text-muted-foreground mb-1"><T>{partition.name}</T></h4>
                    <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 xl:grid-cols-6 gap-x-4 gap-y-1 pl-2">
                        {partition.controllers.map(controller => (
                            <label key={controller.slaveId} className={`flex items-center space-x-1.5 cursor-pointer text-sm py-1 ${controller.isFaulty ? 'opacity-50 cursor-not-allowed' : ''}`} title={controller.isFaulty ? `${controller.name} (Faulty)` : controller.name}>
                                <input type="checkbox" checked={!controller.isFaulty && visibleControllerSlaveIds.has(controller.slaveId)} onChange={() => !controller.isFaulty && setVisibleControllerSlaveIds(prev => {const n=new Set(prev); if(n.has(controller.slaveId))n.delete(controller.slaveId); else n.add(controller.slaveId); return n;})} disabled={controller.isFaulty} className="form-checkbox h-4 w-4 text-primary rounded focus:ring-primary-focus dark:border-gray-600 dark:bg-gray-700 dark:focus:ring-offset-gray-800"/>
                                <span className={`truncate ${controller.isFaulty ? 'line-through' : ''}`} title={controller.name}>{controller.name}{controller.isFaulty && <span className="text-red-500 text-xs ml-1">(Fault)</span>}</span>
                            </label>
                        ))}
                    </div>
                </div>
            ))}
        </div>
        <div className="h-[28rem]">
            <ResponsiveContainer width="100%" height="100%">
                <LineChart 
                    data={processedChartData} 
                    margin={{ top: 5, right: 30, left: 0, bottom: 5 }}
                    onMouseDown={(e: any) => { /* Potential start of drag-to-zoom, more complex */ }}
                    onMouseMove={(e: any) => { /* Potential drag-to-zoom move */ }}
                    onMouseUp={() => { /* Potential end of drag-to-zoom */ }}
                >
                    <CartesianGrid strokeDasharray="3 3" stroke="#737373" strokeOpacity={0.5} />
                    <XAxis 
                        dataKey="timestamp" 
                        type="number" 
                        domain={xZoomDomain || ['dataMin', 'dataMax']}
                        allowDataOverflow 
                        tickFormatter={(unixTime) => new Date(unixTime).toLocaleTimeString()} 
                        stroke="#9ca3af" 
                        tick={{fontSize: 12}}
                    />
                    <YAxis 
                        stroke="#9ca3af" 
                        domain={[0, 400]} 
                        allowDataOverflow={false} 
                        tickCount={21} 
                        tick={{fontSize: 12}} 
                        label={{ value: '°C', angle: -90, position: 'insideLeft', fill: '#9ca3af', fontSize: 12, dx: -10 }}
                    />
                    <Tooltip contentStyle={{ backgroundColor: 'rgba(31, 41, 55, 0.9)', borderColor: 'rgba(255,255,255,0.2)', borderRadius: '0.375rem' }} labelStyle={{ color: '#e5e7eb', fontWeight: 'bold' }} itemStyle={{ color: '#d1d5db' }} formatter={(val: number, id: string, item: any) => [typeof val ==='number'?val.toFixed(1):val, item.name || id]} labelFormatter={(label) => new Date(label).toLocaleTimeString()}/>
                    {allControllerLines.map((line) => {
                        const isPV = line.type === 'PV'; const isSP = line.type === 'SP';
                        const render = visibleControllerSlaveIds.has(line.slaveId) && !faultySlaveIds.has(line.slaveId) && ((isPV && showPV) || (isSP && showSP));
                        return render && <Line key={line.id} type="monotone" dataKey={line.id} name={line.name} stroke={line.color} strokeWidth={1.5} dot={false} isAnimationActive={false} connectNulls={true} strokeDasharray={isPV ? "5 5" : undefined} />;
                    })}
                    {activeProfile && showProfileLine && (
                        <Line type="monotone" dataKey={ACTIVE_PROFILE_MEASURED_TEMP_DATAKEY} name={`Profile: ${activeProfile.name} (Actual Temp)`} stroke={generateColor(0, 'PROFILE')} strokeWidth={2} dot={false} strokeDasharray="4 8" isAnimationActive={false} connectNulls={true}/>
                    )}
                    <Brush 
                        dataKey="timestamp" 
                        height={30} 
                        stroke="#8884d8" 
                        startIndex={undefined}
                        endIndex={undefined}
                        onChange={(e: any) => {
                            if (e && typeof e.startIndex === 'number' && typeof e.endIndex === 'number' && processedChartData.length > 0) {
                                const startDataPoint = processedChartData[e.startIndex];
                                const endDataPoint = processedChartData[e.endIndex];
                                if (startDataPoint && endDataPoint) {
                                    setXZoomDomain([startDataPoint.timestamp, endDataPoint.timestamp]);
                                }
                            } else if (!e) {
                               // setXZoomDomain(null); // Potentially reset if Brush sends a null/undefined event
                            }
                        }}
                    />
                </LineChart>
            </ResponsiveContainer>
        </div>
        <div className="flex flex-wrap justify-between items-center gap-2 pt-2">
            <div className="flex flex-wrap space-x-2 gap-2">
                <Button variant="outline" onClick={handleClearChart} size="sm" className="text-orange-600 hover:text-orange-700 border-orange-500 hover:border-orange-600"><RotateCcw className="h-4 w-4 mr-2" /><T>Clear Chart</T></Button>
                <Button variant="outline" onClick={handleResetZoom} size="sm" disabled={!xZoomDomain}>
                    <ZoomOut className="h-4 w-4 mr-2" />
                    <T>Reset Zoom</T>
                </Button>
            </div>
            <Button variant="outline" onClick={handleExportCSV} size="sm"><Download className="h-4 w-4 mr-2" /><T>Export to CSV</T></Button>
        </div>
    </div>
  );
};

export default ControllerChart; 