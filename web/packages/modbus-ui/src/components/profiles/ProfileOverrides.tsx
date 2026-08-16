import React, { useMemo } from 'react';
import { Button } from '@/components/ui/button';
import { Label } from '@/components/ui/label';
import { Controller } from '@/types';
import { Badge } from '@/components/ui/badge';
import { Plus, Minus } from 'lucide-react';
import { T } from '@/i18n';
import { SP_CMD_COMMAND_REGISTER_PREFIX } from '@/constants';
import { useModbus } from '@/contexts/ModbusContext';
import { getSlaveIdFromGroup } from '@/lib/controllerUtils';


export interface OffsetData {
    targetRegister: number;
    offset: number;
}

interface ProfileOverridesProps {
    selectedTargetRegisters: number[];
    availableControllers: Controller[];
    overrides: OffsetData[];
    onChange: (newOverrides: OffsetData[]) => void;
    readonly?: boolean;
}

const ProfileOverrides: React.FC<ProfileOverridesProps> = ({
    selectedTargetRegisters,
    availableControllers,
    overrides,
    onChange,
    readonly = false
}) => {
    const { registers } = useModbus();

    // Map selected registers to controllers
    const activeControllers = useMemo(() => {
        if (!registers || registers.length === 0) return [];

        return selectedTargetRegisters
            .map(regAddr => {
                if (!regAddr) return null;

                // Find controller that matches this register address
                // We assume the selected register is the SP CMD register
                const matchingReg = registers.find(r => r.address === regAddr);
                if (!matchingReg) return null;

                const slaveId = getSlaveIdFromGroup(matchingReg.group);
                const controller = availableControllers.find(c => c.slaveId === slaveId);

                return {
                    regAddr,
                    controllerName: controller ? controller.name : `Addr: ${regAddr}`,
                    slaveId: slaveId || '?'
                };
            })
            .filter((item): item is { regAddr: number; controllerName: string; slaveId: number | string } => item !== null);
    }, [selectedTargetRegisters, availableControllers, registers]);

    const getOffset = (regAddr: number) => {
        const found = overrides.find(o => o.targetRegister === regAddr);
        return found ? found.offset : 0;
    };

    const handleOffsetChange = (regAddr: number, delta: number) => {
        if (readonly) return;

        const currentOffset = getOffset(regAddr);
        const newOffset = currentOffset + delta;

        const otherOverrides = overrides.filter(o => o.targetRegister !== regAddr);

        if (newOffset === 0) {
            onChange(otherOverrides);
        } else {
            onChange([...otherOverrides, { targetRegister: regAddr, offset: newOffset }]);
        }
    };

    if (activeControllers.length === 0) {
        return null; // Don't show if no controllers selected
    }

    return (
        <div className="space-y-2">
            <Label className="text-slate-700 dark:text-white"><T>Temperature Offsets</T></Label>
            <div className="glass-panel p-3 rounded-xl grid grid-cols-1 sm:grid-cols-2 gap-3">
                {activeControllers.map(({ regAddr, controllerName, slaveId }) => (
                    <div key={regAddr} className="flex items-center justify-between bg-white/50 dark:bg-black/20 p-2 rounded-lg border border-slate-200 dark:border-slate-700">
                        <div className="flex flex-col">
                            <span className="text-xs font-semibold text-slate-700 dark:text-slate-300">{controllerName}</span>
                            <span className="text-[10px] text-slate-500">ID: {slaveId}</span>
                        </div>

                        <div className="flex items-center gap-2">
                            <Button
                                variant="outline"
                                size="icon"
                                className="h-6 w-6 rounded-full"
                                onClick={() => handleOffsetChange(regAddr, -1)}
                                disabled={readonly}
                            >
                                <Minus className="h-3 w-3" />
                            </Button>

                            <Badge variant={getOffset(regAddr) !== 0 ? "default" : "secondary"} className="w-12 justify-center">
                                {getOffset(regAddr) > 0 ? '+' : ''}{getOffset(regAddr)}°C
                            </Badge>

                            <Button
                                variant="outline"
                                size="icon"
                                className="h-6 w-6 rounded-full"
                                onClick={() => handleOffsetChange(regAddr, 1)}
                                disabled={readonly}
                            >
                                <Plus className="h-3 w-3" />
                            </Button>
                        </div>
                    </div>
                ))}
            </div>
            <p className="text-[10px] text-muted-foreground"><T>Offsets are applied to the profile setpoint for each specific controller.</T></p>
        </div>
    );
};

export default ProfileOverrides;
