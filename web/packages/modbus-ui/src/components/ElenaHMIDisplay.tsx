import React, { useMemo } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { T } from '../i18n';
import { useToast } from "@/components/ui/use-toast";
import CollapsibleSection from './CollapsibleSection';
import Commons from './Commons';
import VFDControls from './VFDControls';
import PlungerHMIDisplay from './PlungerHMIDisplay';


const ElenaHMIDisplay = () => {
  const {
    registers: allModbusRegisters,
    profiles: contextProfiles,
    updateRegister,
    isConnected,
    settings,
    featureFlags
  } = useModbus();  
  if (!isConnected && (!allModbusRegisters || allModbusRegisters.length === 0)) {
    return (
      <div className="p-4 text-center">
        <p className="text-muted-foreground"><T>Connect to a Modbus server to see controller data.</T></p>
      </div>
    );
  }

  if (allModbusRegisters.length === 0 && isConnected) {
    return (
      <div className="p-4 text-center">
        <p className="text-muted-foreground"><T>Connected, but no register data received yet. Waiting for data...</T></p>
      </div>
    );
  }

  return (
    <div className="space-y-6" id="elena-hmi-display">
      {/* Dashboard - Always visible, no borders */}
      <div>
        <PlungerHMIDisplay isDashboardView={true} />
      </div>    

      {/* Advanced Controls - Only show if features are enabled */}
      {(featureFlags.ENABLE_SAKO_VFD) && (
        <CollapsibleSection
          title={<T>Advanced Controls</T>}
          storageKey="hmi-advanced-collapsible"
          initiallyOpen={false}
          minimal={true}
        >
          <div className="space-y-4">
            {featureFlags.ENABLE_SAKO_VFD && (
              <div>
                <h3 className="text-md font-medium mb-2"><T>VFD Control</T></h3>
                <VFDControls />
              </div>
            )}
      
            <div>
              <h3 className="text-md font-medium mb-2"><T>Commons</T></h3>
              <Commons />
            </div>
          </div>
        </CollapsibleSection>
      )}
    </div>
  );
};

export default ElenaHMIDisplay; 