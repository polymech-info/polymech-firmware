
import { useState } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Check } from 'lucide-react';

const DisplaySettings = () => {
  const { 
    coilStartAddress,
    registerStartAddress,
    coilCount,
    registerCount,
    setCoilStartAddress,
    setRegisterStartAddress,
    setCoilCount,
    setRegisterCount
  } = useModbus();
  
  const [tempCoilStart, setTempCoilStart] = useState(coilStartAddress.toString());
  const [tempCoilCount, setTempCoilCount] = useState(coilCount.toString());
  const [tempRegStart, setTempRegStart] = useState(registerStartAddress.toString());
  const [tempRegCount, setTempRegCount] = useState(registerCount.toString());

  const handleApplyCoilSettings = () => {
    setCoilStartAddress(parseInt(tempCoilStart) || 0);
    setCoilCount(parseInt(tempCoilCount) || 10);
  };

  const handleApplyRegisterSettings = () => {
    setRegisterStartAddress(parseInt(tempRegStart) || 0);
    setRegisterCount(parseInt(tempRegCount) || 10);
  };

  return (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
      <div className="glass-morphism p-4 rounded-lg">
        <h3 className="text-sm font-bold mb-3 text-gradient">Coil Settings</h3>
        <div className="grid grid-cols-2 gap-3">
          <div className="space-y-2">
            <Label htmlFor="coilStartAddress" className="text-xs font-mono">START ADDRESS</Label>
            <Input
              id="coilStartAddress"
              value={tempCoilStart}
              onChange={(e) => setTempCoilStart(e.target.value.replace(/[^0-9]/g, ''))}
              className="bg-black/30 border-white/10 font-mono"
            />
          </div>
          <div className="space-y-2">
            <Label htmlFor="coilCount" className="text-xs font-mono">COUNT</Label>
            <Input
              id="coilCount"
              value={tempCoilCount}
              onChange={(e) => setTempCoilCount(e.target.value.replace(/[^0-9]/g, ''))}
              className="bg-black/30 border-white/10 font-mono"
            />
          </div>
        </div>
        <div className="flex justify-end mt-3">
          <Button 
            size="sm" 
            onClick={handleApplyCoilSettings}
            className="bg-primary/30 hover:bg-primary text-white flex items-center gap-2"
          >
            <Check className="h-3 w-3" />
            <span>Apply</span>
          </Button>
        </div>
      </div>

      <div className="glass-morphism p-4 rounded-lg">
        <h3 className="text-sm font-bold mb-3 text-gradient">Register Settings</h3>
        <div className="grid grid-cols-2 gap-3">
          <div className="space-y-2">
            <Label htmlFor="registerStartAddress" className="text-xs font-mono">START ADDRESS</Label>
            <Input
              id="registerStartAddress"
              value={tempRegStart}
              onChange={(e) => setTempRegStart(e.target.value.replace(/[^0-9]/g, ''))}
              className="bg-black/30 border-white/10 font-mono"
            />
          </div>
          <div className="space-y-2">
            <Label htmlFor="registerCount" className="text-xs font-mono">COUNT</Label>
            <Input
              id="registerCount"
              value={tempRegCount}
              onChange={(e) => setTempRegCount(e.target.value.replace(/[^0-9]/g, ''))}
              className="bg-black/30 border-white/10 font-mono"
            />
          </div>
        </div>
        <div className="flex justify-end mt-3">
          <Button 
            size="sm" 
            onClick={handleApplyRegisterSettings}
            className="bg-primary/30 hover:bg-primary text-white flex items-center gap-2"
          >
            <Check className="h-3 w-3" />
            <span>Apply</span>
          </Button>
        </div>
      </div>
    </div>
  );
};

export default DisplaySettings;
