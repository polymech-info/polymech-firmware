import { useState } from 'react';
import { useModbus } from '@/contexts/ModbusContext';

export const useCoilToggle = () => {
  const { updateCoil } = useModbus();
  const [pendingUpdates, setPendingUpdates] = useState<Set<number>>(new Set());

  const handleCoilToggle = async (address: number, newValue: boolean) => {
    setPendingUpdates(prev => new Set([...prev, address]));
    try {
      await updateCoil(address, newValue);
    } finally {
      setPendingUpdates(prev => {
        const newSet = new Set([...prev]);
        newSet.delete(address);
        return newSet;
      });
    }
  };

  return {
    pendingUpdates,
    handleCoilToggle
  };
}; 