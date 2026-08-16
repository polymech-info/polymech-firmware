import { Switch } from '@/components/ui/switch';
import { cn } from '@/lib/utils';
import { useCoilToggle } from '@/hooks/useCoilToggle';

interface CoilSwitchProps {
  address: number;
  value: boolean;
  disabled?: boolean;
  className?: string;
  'aria-label'?: string;
}

export const CoilSwitch: React.FC<CoilSwitchProps> = ({ 
  address, 
  value, 
  disabled = false,
  className,
  'aria-label': ariaLabel 
}) => {
  const { pendingUpdates, handleCoilToggle } = useCoilToggle();
  const isPending = pendingUpdates.has(address);

  return (
    <div className="flex items-center space-x-2">
      <span className={cn(
        "text-xs font-mono",
        value ? "text-green-400" : "text-muted-foreground"
      )}>
        {value ? "ON" : "OFF"}
      </span>
      <Switch
        checked={value}
        onCheckedChange={(checked) => handleCoilToggle(address, checked)}
        disabled={disabled || isPending}
        className={cn(
          value && "bg-primary",
          isPending && "animate-pulse",
          className
        )}
        aria-label={ariaLabel}
      />
    </div>
  );
}; 