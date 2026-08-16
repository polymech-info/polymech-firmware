import React from 'react';
import { ControlPoint } from '@/types';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Trash2Icon } from 'lucide-react';
import { Label } from '@/components/ui/label';
import { TimeCodeEditor } from '@/components/TimeCodeEditor';
import { T, translate } from '../../../i18n';

interface BezierControlPointListProps {
  controlPoints: ControlPoint[];
  onChange: (points: ControlPoint[]) => void;
  duration: number; // in milliseconds
  max: number; // max temperature
  readonly?: boolean;
}

const BezierControlPointList: React.FC<BezierControlPointListProps> = ({
  controlPoints,
  onChange,
  duration,
  max,
  readonly = false,
}) => {
  const handlePointChange = (index: number, field: 'x' | 'y', value: number) => {
    const newPoints = [...controlPoints];
    const point = { ...newPoints[index] };

    if (field === 'x') { // value is time in ms
      point.x = duration > 0 ? Math.max(0, Math.min(1, value / duration)) : 0;
    } else { // value is temperature
      point.y = max > 0 ? Math.max(0, Math.min(1, value / max)) : 0;
    }

    // prevent crossing over
    if (field === 'x') {
      const prevPointX = index > 0 ? newPoints[index - 1].x : -1;
      const nextPointX = index < newPoints.length - 1 ? newPoints[index + 1].x : 2;
      if (point.x < prevPointX) point.x = prevPointX;
      if (point.x > nextPointX) point.x = nextPointX;
    }
    
    newPoints[index] = point;
    onChange(newPoints);
  };

  const handleDeletePoint = (index: number) => {
    if (readonly || index === 0 || index === controlPoints.length - 1) return;
    const newPoints = controlPoints.filter((_, i) => i !== index);
    onChange(newPoints);
  };
  
  if (!controlPoints) {
    return null;
  }

  return (
    <div className="space-y-2">
      <h3 className="text-sm font-medium text-muted-foreground"><T>Control Points</T> ({controlPoints.length})</h3>
      <div className="space-y-2 rounded-md border p-2 max-h-80 overflow-y-auto">
        {controlPoints.map((point, index) => (
          <div key={index} className="flex items-end gap-2 p-2 rounded-md bg-muted/20">
            <div className="w-16 font-semibold self-center">Point {index + 1}</div>
            
            <div className="flex-1">
                <Label htmlFor={`timecode-h-${index}`} className="text-xs"><T>Time</T></Label>
                <TimeCodeEditor
                  idPrefix={`timecode-${index}`}
                  totalMilliseconds={point.x * duration}
                  onDurationChange={(newMs) => handlePointChange(index, 'x', newMs)}
                  disabled={readonly || index === 0 || index === controlPoints.length - 1}
                />
            </div>

            <div className="w-28">
                <Label htmlFor={`temp-input-${index}`} className="text-xs"><T>Temp (°C)</T></Label>
                <Input
                  id={`temp-input-${index}`}
                  type="number"
                  value={Math.round(point.y * max)}
                  onChange={(e) => handlePointChange(index, 'y', Number(e.target.value))}
                  disabled={readonly}
                  className="h-8"
                  min="0"
                  max={max}
                />
            </div>
            
            <Button
              type="button"
              variant="ghost"
              size="icon"
              onClick={(e) => { e.stopPropagation(); handleDeletePoint(index); }}
              disabled={readonly || index === 0 || index === controlPoints.length - 1}
              className="text-destructive"
              title={translate("Delete control point")}
            >
              <Trash2Icon className="h-4 w-4" />
            </Button>
          </div>
        ))}
      </div>
    </div>
  );
};

export default BezierControlPointList; 