import { Switch } from '@/components/ui/switch';
import { useModbus } from '@/contexts/ModbusContext';
import logger from '@/Logger';

interface CoilSwitchProps {
  address: number;
  value: boolean;
  disabled?: boolean;
  'aria-label'?: string;
}

export const CoilSwitch: React.FC<CoilSwitchProps> = ({
  address,
  value,
  disabled = false,
}) => {
  const { updateCoil } = useModbus();

  const handleToggle = async (checked: boolean) => {
    if (disabled) {
      logger.warn(`Cannot toggle coil ${address}: switch is disabled.`);
      return;
    }
    try {
      await updateCoil(address, checked);
    } catch (error) {
      logger.error(`Failed to toggle coil ${address}:`, error);
    }
  };

  return (
    <div className="flex items-center space-x-2">
      <Switch
        checked={value}
        onCheckedChange={handleToggle}
        disabled={disabled}
        className="data-[state=checked]:bg-emerald-500 data-[state=unchecked]:bg-slate-300 dark:data-[state=unchecked]:bg-slate-600"
      />
    </div>
  );
}; 