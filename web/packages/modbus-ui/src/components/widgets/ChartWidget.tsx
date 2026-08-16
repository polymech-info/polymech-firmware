import React, { useState, useEffect, useMemo, useCallback } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import { useModbus } from '@/contexts/ModbusContext';
import { Button } from '@/components/ui/button';
import { AddressPicker, AddressGroup, AddressOption } from '@/components/modbus/AddressPicker';
import { useFavorites } from '@/hooks/useFavorites';
import { X, Plus } from 'lucide-react';

interface SeriesConfig {
  id: string;
  address: number;
  source: 'register' | 'coil';
  color: string;
  title?: string;
}

interface ChartWidgetProps {
  widgetInstanceId?: string;
  onPropsChange?: (newProps: Record<string, any>) => void;

  // Minimal settings
  height?: number;
  duration?: number;
  yMax?: number;
  yMin?: number;
  refreshRateMs?: number;

  // Series data (stored as JSON string)
  seriesData?: string;
}

const defaultColors = [
  '#00FF00', '#FFFF00', '#0000FF', '#FF0000', '#00FFFF', '#FF00FF',
  '#FFA500', '#800080', '#FFC0CB', '#A52A2A', '#808080', '#000080'
];

const ChartWidget: React.FC<ChartWidgetProps> = ({
  widgetInstanceId,
  onPropsChange,
  height = 300,
  duration = 300,
  yMax = 100,
  yMin = 0,
  refreshRateMs = 1000,
  seriesData = '[]'
}) => {
  const { registers, coils, isConnected } = useModbus();
  const { favoriteRegisters, favoriteCoils } = useFavorites();

  const storageKey = useMemo(() => {
    if (!widgetInstanceId) return null;
    return `chart_widget_data_${widgetInstanceId}`;
  }, [widgetInstanceId]);

  const [chartData, setChartData] = useState<any[]>([]);
  const chartDataRef = React.useRef(chartData);

  // Keep ref in sync
  useEffect(() => {
    chartDataRef.current = chartData;
  }, [chartData]);

  // Load initial data on mount or id change
  useEffect(() => {
    if (storageKey) {
      try {
        const savedData = sessionStorage.getItem(storageKey);
        if (savedData) {
          setChartData(JSON.parse(savedData));
          return;
        }
      } catch (e) {
        console.error("Failed to load chart data", e);
      }
    }
    setChartData([]);
  }, [storageKey]);

  // Save on unmount or visibility change (optimization)
  useEffect(() => {
    const save = () => {
      if (storageKey && chartDataRef.current.length > 0) {
        sessionStorage.setItem(storageKey, JSON.stringify(chartDataRef.current));
      }
    };

    const onVisibilityChange = () => {
      if (document.visibilityState === 'hidden') save();
    };

    window.addEventListener('visibilitychange', onVisibilityChange);
    window.addEventListener('beforeunload', save);

    return () => {
      save(); // Save on unmount
      window.removeEventListener('visibilitychange', onVisibilityChange);
      window.removeEventListener('beforeunload', save);
    };
  }, [storageKey]);

  const [showAddSeries, setShowAddSeries] = useState(false);

  // Parse series from JSON string
  const series: SeriesConfig[] = useMemo(() => {
    try {
      return JSON.parse(seriesData);
    } catch {
      return [];
    }
  }, [seriesData]);

  // Save series to props
  const updateSeries = useCallback((newSeries: SeriesConfig[]) => {
    onPropsChange?.({
      seriesData: JSON.stringify(newSeries)
    });
  }, [onPropsChange]);

  // Add a new series
  const addSeries = useCallback((address: number, source: 'register' | 'coil') => {
    const newSeries: SeriesConfig = {
      id: `${source}-${address}-${Date.now()}`,
      address,
      source,
      color: defaultColors[series.length % defaultColors.length],
      title: ''
    };
    updateSeries([...series, newSeries]);
    setShowAddSeries(false);
  }, [series, updateSeries]);

  // Remove a series
  const removeSeries = useCallback((seriesId: string) => {
    updateSeries(series.filter(s => s.id !== seriesId));
  }, [series, updateSeries]);

  // Address picker groups
  const groupedItems = useMemo((): AddressGroup[] => {
    const favoriteRegisterSet = new Set(favoriteRegisters);
    const favoriteCoilSet = new Set(favoriteCoils);

    const createOption = (item: any, source: 'register' | 'coil'): AddressOption => {
      const prefix = source === 'register' ? '[R]' : '[C]';
      const name = item.name || `${source === 'register' ? 'Register' : 'Coil'} ${item.address}`;
      const group = item.group || 'Default';
      return {
        value: `${source}-${item.address}`,
        label: `${prefix} ${group}::${name} (${item.address})`,
        titleForSeries: `${group}::${name}`,
        source: source,
        group,
      };
    };

    const favoriteOptions = [
      ...registers.filter(r => favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...coils.filter(c => favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ].sort((a, b) => a.label.localeCompare(b.label));

    const allNonFavoriteItems = [
      ...registers.filter(r => !favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...coils.filter(c => !favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ];

    const groupedNonFavorites = allNonFavoriteItems.reduce((acc, item) => {
      const group = item.group || 'Uncategorized';
      const groupName = `${group} (${item.source === 'register' ? 'Registers' : 'Coils'})`;

      if (!acc[groupName]) acc[groupName] = [];
      acc[groupName].push(item);
      return acc;
    }, {} as Record<string, AddressOption[]>);

    const result: AddressGroup[] = [];
    if (favoriteOptions.length > 0) {
      result.push({ label: `⭐ Favorites`, options: favoriteOptions });
    }

    Object.entries(groupedNonFavorites)
      .map(([label, options]) => ({
        label,
        options: options.sort((a, b) => a.label.localeCompare(b.label)),
      }))
      .sort((a, b) => a.label.localeCompare(b.label))
      .forEach(group => result.push(group));

    return result;
  }, [registers, coils, favoriteRegisters, favoriteCoils]);

  const itemLabelMap = useMemo(() => {
    const newMap = new Map<string, string>();
    registers.forEach(r => {
      newMap.set(`register-${r.address}`, `[R] ${r.group || 'Default'}::${r.name || `Register ${r.address}`} (${r.address})`);
    });
    coils.forEach(c => {
      newMap.set(`coil-${c.address}`, `[C] ${c.group || 'Default'}::${c.name || `Coil ${c.address}`} (${c.address})`);
    });
    return newMap;
  }, [registers, coils]);

  // Data polling effect
  useEffect(() => {
    if (!isConnected || series.length === 0) return;

    const intervalId = setInterval(() => {
      const now = Date.now();
      const newDataPoint: any = { time: now };
      let hasNewData = false;

      series.forEach(s => {
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
            processedValue = currentValue ? yMax : yMin;
          } else {
            processedValue = currentValue === 65535 ? 0 : currentValue as number;
          }
          newDataPoint[s.id] = processedValue;
        }
      });

      if (hasNewData) {
        setChartData(prevData => {
          const windowMs = duration * 1000;
          const cutoff = Date.now() - windowMs;
          const updatedData = [...prevData, newDataPoint];
          return updatedData.filter(d => d.time >= cutoff);
        });
      }
    }, refreshRateMs);

    return () => clearInterval(intervalId);
  }, [isConnected, registers, coils, series, duration, refreshRateMs, yMax, yMin]);

  // Generate line names
  const getLineName = useCallback((s: SeriesConfig) => {
    if (s.title) return s.title;
    if (s.source === 'coil') {
      const coil = coils.find(c => c.address === s.address);
      if (coil && coil.name) return `${coil.group || 'Default'}::${coil.name}`;
    } else {
      const register = registers.find(r => r.address === s.address);
      if (register && register.name) return `${register.group || 'Default'}::${register.name}`;
    }
    return `${s.source === 'coil' ? 'Coil' : 'Register'} ${s.address}`;
  }, [coils, registers]);

  const handleAddressSelect = (value: string, source: 'register' | 'coil') => {
    const address = parseInt(value.split('-')[1], 10);
    if (!isNaN(address)) {
      addSeries(address, source);
    }
  };

  return (
    <div className="w-full p-0.5 sm:p-1 flex flex-col" style={{ height: `${height}px` }}>
      <div className="rounded border bg-muted/20 p-0.5 sm:p-1 dark:bg-black/20 flex-grow min-h-0">
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={chartData} >
            <CartesianGrid strokeDasharray="3 3" stroke="hsl(var(--border))" />
            <XAxis
              dataKey="time"
              type="number"
              domain={[
                (_dataMin: number) => Date.now() - (duration * 1000),
                (_dataMax: number) => Date.now()
              ]}
              stroke="hsl(var(--muted-foreground))"
              tickFormatter={(unixTime) => new Date(unixTime).toLocaleTimeString()}
              fontSize={8}
              tickCount={3}
              allowDataOverflow={false}
            />
            <YAxis
              stroke="hsl(var(--muted-foreground))"
              domain={[yMin, yMax]}
              allowDataOverflow={false}
              tickCount={5}
              fontSize={9}
              tick={{ fontSize: 10 }}
              width={15}
            />
            <Tooltip
              contentStyle={{
                backgroundColor: 'hsl(var(--popover))',
                color: 'hsl(var(--popover-foreground))',
                border: '1px solid hsl(var(--border))',
                fontSize: '10px'
              }}
              labelFormatter={(label) => new Date(label).toLocaleTimeString()}
              formatter={(value: number) => value?.toFixed(2) || 'N/A'}
            />
            <Legend
              wrapperStyle={{ fontSize: '10px', paddingTop: '5px' }}
              iconSize={8}
            />
            {series.map(s => (
              <Line
                key={s.id}
                type={s.source === 'coil' ? 'stepAfter' : 'monotone'}
                dataKey={s.id}
                stroke={s.color}
                name={getLineName(s)}
                dot={false}
                isAnimationActive={false}
                animateNewValues={false}
                connectNulls={false}
                strokeWidth={1.5}
              />
            ))}
          </LineChart>
        </ResponsiveContainer>
      </div>

      {/* Series Management - Below Chart */}
      <div className="flex flex-wrap items-center gap-1 mt-0.5 sm:mt-1 min-h-[16px] sm:min-h-[28px] flex-shrink-0">
        {series.map(s => (
          <div key={s.id} className="flex items-center gap-1 bg-slate-100 dark:bg-slate-800 rounded px-1.5 py-0.5 text-xs">
            <div className="w-2.5 h-2.5 sm:w-3 sm:h-3 rounded" style={{ backgroundColor: s.color }}></div>
            <span className="truncate max-w-[120px] sm:max-w-none">{getLineName(s)}</span>
            <Button
              size="icon"
              variant="ghost"
              className="h-3.5 w-3.5 sm:h-4 sm:w-4 p-0 hover:bg-red-500/20"
              onClick={() => removeSeries(s.id)}
            >
              <X className="h-2.5 w-2.5 sm:h-3 sm:w-3" />
            </Button>
          </div>
        ))}

        {showAddSeries ? (
          <div className="flex items-center gap-1 sm:gap-2">
            <AddressPicker
              value=""
              onSelect={handleAddressSelect}
              groupedItems={groupedItems}
              itemLabelMap={itemLabelMap}
              placeholder="Select source..."
              className="w-40 sm:w-48 text-xs"
            />
            <Button
              size="icon"
              variant="ghost"
              className="h-5 w-5 sm:h-6 sm:w-6 p-0"
              onClick={() => setShowAddSeries(false)}
            >
              <X className="h-3 w-3" />
            </Button>
          </div>
        ) : (
          <Button
            size="icon"
            variant="outline"
            className="h-5 w-5 sm:h-6 sm:w-6 p-0"
            onClick={() => setShowAddSeries(true)}
          >
            <Plus className="h-2.5 w-2.5 sm:h-3 sm:w-3" />
          </Button>
        )}
      </div>
    </div>
  );
};

export default ChartWidget;
