import React, { useMemo, useState, useEffect } from 'react';
import { Button } from '@/components/ui/button';
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
  DialogClose,
} from '@/components/ui/dialog';
import { Input } from '@/components/ui/input';
import { Checkbox } from '@/components/ui/checkbox';
import { T, translate } from '../i18n';
import { RegisterData, CoilData } from '@/contexts/ModbusContext';
import { AddressPicker, AddressGroup, AddressOption } from './modbus/AddressPicker';

export interface SeriesConfig {
  id: number;
  enabled: boolean;
  color: string;
  offset: number;
  scale: number;
  title: string;
  address: number;
  fileName: string;
  source: 'register' | 'coil';
}

interface SeriesData {
  registers: RegisterData[];
  coils: CoilData[];
}

interface SeriesSettingsModalProps {
  series: SeriesConfig[];
  setSeries: React.Dispatch<React.SetStateAction<SeriesConfig[]>>;
  children: React.ReactNode;
  registers: RegisterData[];
  coils: CoilData[];
  favoriteRegisters: number[];
  favoriteCoils: number[];  
}

export const SeriesSettingsModal: React.FC<SeriesSettingsModalProps> = ({ series, setSeries, children, registers, coils, favoriteRegisters, favoriteCoils }) => {
  const [open, setOpen] = useState(false);
  const [snapshot, setSnapshot] = useState<SeriesData | null>(null);

  useEffect(() => {
    if (open && !snapshot) {
      setSnapshot({
        registers: JSON.parse(JSON.stringify(registers)),
        coils: JSON.parse(JSON.stringify(coils)),
      });
    } else if (!open) {
      setSnapshot(null);
    }
  }, [open, registers, coils, snapshot]);
  
  const handleSeriesChange = (id: number, field: keyof SeriesConfig, value: any) => {
    setSeries(prevSeries =>
      prevSeries.map(s => (s.id === id ? { ...s, [field]: value } : s))
    );
  };

  const groupedItems = useMemo((): AddressGroup[] => {
    if (!snapshot) return [];
    const favoriteRegisterSet = new Set(favoriteRegisters);
    const favoriteCoilSet = new Set(favoriteCoils);
    
    const createOption = (item: RegisterData | CoilData, source: 'register' | 'coil'): AddressOption => {
      const prefix = source === 'register' ? '[R]' : '[C]';
      const name = item.name || `${source === 'register' ? 'Register' : 'Coil'} ${item.address}`;
      const group = item.group || 'Default';
      return {
        value: String(item.address),
        label: `${prefix} ${group}::${name} (${item.address})`,
        titleForSeries: `${group}::${name}`,
        source: source,
        group,
      };
    };

    const favoriteOptions = [
      ...snapshot.registers.filter(r => favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...snapshot.coils.filter(c => favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ].sort((a,b) => a.label.localeCompare(b.label));

    const allNonFavoriteItems = [
      ...snapshot.registers.filter(r => !favoriteRegisterSet.has(r.address)).map(r => createOption(r, 'register')),
      ...snapshot.coils.filter(c => !favoriteCoilSet.has(c.address)).map(c => createOption(c, 'coil'))
    ];

    const groupedNonFavorites = allNonFavoriteItems.reduce((acc, item) => {
      const itemName = item.label.split('::')[1]?.split(' (')[0] || '';
      const group = (itemName.includes('Register') || itemName.includes('Coil')) ? 'Uncategorized' : item.label.split('::')[0].replace(/\[[RC]\] /,'');
      const groupName = `${group} (${item.source === 'register' ? 'Registers' : 'Coils'})`;

      if (!acc[groupName]) acc[groupName] = [];
      acc[groupName].push(item);
      return acc;
    }, {} as Record<string, AddressOption[]>);

    const result: AddressGroup[] = [];
    if(favoriteOptions.length > 0) {
      result.push({ label: `⭐ ${translate('Favorites')}`, options: favoriteOptions });
    }

    Object.entries(groupedNonFavorites)
      .map(([label, options]) => ({
        label,
        options: options.sort((a, b) => a.label.localeCompare(b.label)),
      }))
      .sort((a, b) => a.label.localeCompare(b.label))
      .forEach(group => result.push(group));

    return result;
  }, [snapshot, favoriteRegisters, favoriteCoils]);
  
  const itemLabelMap = useMemo(() => {
    if (!snapshot) return new Map();
    const newMap = new Map<string, string>();
    snapshot.registers.forEach(r => {
        newMap.set(`register-${r.address}`, `[R] ${r.group || 'Default'}::${r.name || `Register ${r.address}`} (${r.address})`);
    });
    snapshot.coils.forEach(c => {
        newMap.set(`coil-${c.address}`, `[C] ${c.group || 'Default'}::${c.name || `Coil ${c.address}`} (${c.address})`);
    });
    return newMap;
  }, [snapshot]);


  return (
    <Dialog open={open} onOpenChange={setOpen}>
      <DialogTrigger asChild>{children}</DialogTrigger>
      <DialogContent className="max-w-7xl">
        <DialogHeader>
          <DialogTitle><T>Series settings</T></DialogTitle>
          <DialogDescription>
            <T>Configure the series to be displayed on the chart.</T>
          </DialogDescription>
        </DialogHeader>

        <div className="grid grid-cols-[auto_auto_auto_1fr_1fr_1.5fr_1.5fr_1fr] gap-x-4 gap-y-2 items-center p-4 text-sm max-h-[60vh] overflow-y-auto">
          {/* Header */}
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Series</T></div>
          <div className="font-bold text-center sticky top-0 bg-background pb-2 z-10"><T>Enabled</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Color</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Offset</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Scale</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Title (Optional)</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>Source</T></div>
          <div className="font-bold sticky top-0 bg-background pb-2 z-10"><T>File name</T></div>

          {series.map(s => (
            <React.Fragment key={s.id}>
              <div className="font-semibold">{s.id}</div>
              <div className="flex justify-center">
                <Checkbox
                  checked={s.enabled}
                  onCheckedChange={checked => handleSeriesChange(s.id, 'enabled', !!checked)}
                />
              </div>
              <div>
                <Input
                  type="color"
                  value={s.color}
                  onChange={e => handleSeriesChange(s.id, 'color', e.target.value)}
                  className="p-1 h-8 w-14"
                />
              </div>
              <div>
                <Input
                  type="number"
                  value={s.offset}
                  onChange={e => handleSeriesChange(s.id, 'offset', Number(e.target.value))}
                />
              </div>
              
              <div>
                <Input
                  type="number"
                  value={s.scale ?? 1}
                  onChange={(e) => handleSeriesChange(s.id, 'scale', Number(e.target.value))}
                  placeholder="1"
                />
              </div>

              <div>
                <Input
                  value={s.title}
                  onChange={(e) => handleSeriesChange(s.id, 'title', e.target.value)}
                  placeholder="e.g. Main Temperature"
                />
              </div>
              
              <div>
                <AddressPicker
                  value={s.address > 0 ? `${s.source}-${s.address}` : ''}
                  onSelect={(value, source, title) => {
                    setSeries(prevSeries => prevSeries.map(p => {
                      if (p.id === s.id) {
                        const newSeriesData: Partial<SeriesConfig> = {
                          address: parseInt(value, 10) || 0,
                          source: source,
                        };
                        if (!p.title) {
                          newSeriesData.title = title;
                        }
                        return { ...p, ...newSeriesData };
                      }
                      return p;
                    }))
                  }}
                  groupedItems={groupedItems}
                  itemLabelMap={itemLabelMap}
                />
              </div>

              <div>
                <Input
                  value={s.fileName}
                  onChange={e => handleSeriesChange(s.id, 'fileName', e.target.value)}
                  placeholder="e.g. Mbpoll1"
                />
              </div>
            </React.Fragment>
          ))}
        </div>
        <DialogFooter>
            <DialogClose asChild>
                <Button type="button"><T>OK</T></Button>
            </DialogClose>
            <DialogClose asChild>
                <Button type="button" variant="secondary"><T>Cancel</T></Button>
            </DialogClose>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
}; 