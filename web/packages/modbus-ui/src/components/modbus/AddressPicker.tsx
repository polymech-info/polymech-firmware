import React, { useState, useMemo } from 'react';
import { Button } from '@/components/ui/button';
import { Popover, PopoverContent, PopoverTrigger } from '@/components/ui/popover';
import { Command, CommandEmpty, CommandGroup, CommandInput, CommandItem, CommandList } from '@/components/ui/command';
import { ChevronsUpDown } from 'lucide-react';
import { translate } from '../../i18n';

export interface AddressOption {
  value: string;
  label: string;
  source: 'register' | 'coil';
  titleForSeries: string;
  group: string;
}

export interface AddressGroup {
  label: string;
  options: AddressOption[];
}

export interface AddressPickerProps {
  value: string;
  onSelect: (value: string, source: 'register' | 'coil', title: string) => void;
  groupedItems: AddressGroup[];
  itemLabelMap: Map<string, string>;
  placeholder?: string;
  className?: string;
  showCoils?: boolean;
  showRegisters?: boolean;
  showFavourites?: boolean;
}

export const AddressPicker: React.FC<AddressPickerProps> = ({ 
  value, 
  onSelect, 
  groupedItems, 
  itemLabelMap,
  placeholder = "Select source...",
  className = "w-full justify-between text-xs",
  showCoils = true,
  showRegisters = true,
  showFavourites = true
}) => {
  const [open, setOpen] = useState(false);

  const filteredGroupedItems = useMemo(() => {
    return groupedItems
      .map(group => {
        // Filter out favorites group if showFavourites is false
        if (!showFavourites && group.label.startsWith('⭐')) {
          return null;
        }
        
        // Filter options within each group based on source type
        const filteredOptions = group.options.filter(option => {
          if (option.source === 'coil' && !showCoils) return false;
          if (option.source === 'register' && !showRegisters) return false;
          return true;
        });

        // Only return group if it has options after filtering
        if (filteredOptions.length === 0) return null;

        return {
          ...group,
          options: filteredOptions
        };
      })
      .filter(Boolean) as AddressGroup[];
  }, [groupedItems, showCoils, showRegisters, showFavourites]);

  const commandFilter = (value: string, search: string): number => {
    const lowerCaseValue = value.toLowerCase();
    const trimmedSearch = search.trim().toLowerCase();

    if (!trimmedSearch) return 1;

    const orGroups = trimmedSearch.split('|').map(s => s.trim()).filter(Boolean);

    const match = orGroups.some(group => {
      const andTerms = group.split(' ').filter(Boolean);
      if (andTerms.length === 0) return false;

      return andTerms.every(term => lowerCaseValue.includes(term));
    });

    return match ? 1 : 0;
  };

  return (
    <Popover open={open} onOpenChange={setOpen}>
      <PopoverTrigger asChild>
        <Button
          role="combobox"
          aria-expanded={open}
          className={`${className} glass-input min-h-[40px] h-auto justify-between text-left`}
        >
          <span className="whitespace-normal break-words text-xs leading-relaxed flex-1 text-left">
            {value ? itemLabelMap.get(value) : translate(placeholder)}
          </span>
          <ChevronsUpDown className="ml-2 h-4 w-4 shrink-0 opacity-50" />
        </Button>
      </PopoverTrigger>
      <PopoverContent className="w-[95vw] max-w-[400px] p-0 glass-panel border-0">
        <Command filter={commandFilter} className="glass-panel">
          <CommandInput placeholder={translate("Search...")} className="glass-input border-0 p-1" />
          <CommandList className="max-h-[300px] overflow-y-auto">
            <CommandEmpty className="text-slate-500 dark:text-slate-400">{translate("No source found.")}</CommandEmpty>
            {filteredGroupedItems.map(group => (
              <CommandGroup key={group.label} heading={group.label} className="text-slate-600 dark:text-slate-300">
                {group.options.map(option => (
                  <CommandItem
                    key={`${option.group}-${option.source}-${option.value}`}
                    value={`${option.label} ${option.source} ${option.value}`}
                    onSelect={() => {
                      onSelect(option.value, option.source, option.titleForSeries);
                      setOpen(false);
                    }}
                    className="text-slate-700 dark:text-white hover:bg-slate-100 dark:hover:bg-white/10 data-[selected]:bg-slate-200 dark:data-[selected]:bg-white/20 min-h-[40px] py-2"
                  >
                    <span className="whitespace-normal break-words text-xs leading-relaxed w-full">{option.label}</span>
                  </CommandItem>
                ))}
              </CommandGroup>
            ))}
          </CommandList>
        </Command>
      </PopoverContent>
    </Popover>
  );
};