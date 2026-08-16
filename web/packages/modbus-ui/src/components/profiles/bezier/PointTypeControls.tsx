
import React from "react";
import { ControlPointType } from "./types";
import { cn } from "@/lib/utils";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";

interface PointTypeControlsProps {
  selectedType: ControlPointType;
  onChange: (type: ControlPointType) => void;
  disabled: boolean;
}

const PointTypeControls: React.FC<PointTypeControlsProps> = ({
  selectedType,
  onChange,
  disabled
}) => {
  return (
    <div className="flex justify-center space-x-2 mb-2">
      <ToggleGroup 
        type="single" 
        value={selectedType} 
        onValueChange={(value) => value && onChange(value as ControlPointType)}
        className={cn(disabled ? "opacity-50" : "")}
        disabled={disabled}
      >
        <ToggleGroupItem value="linear">Linear</ToggleGroupItem>
        <ToggleGroupItem value="quadratic">Quadratic</ToggleGroupItem>
        <ToggleGroupItem value="cubic">Cubic</ToggleGroupItem>
      </ToggleGroup>
    </div>
  );
};

export default PointTypeControls;
