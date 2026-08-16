import React from 'react';
import { Card, CardContent } from '@/components/ui/card';
import { Label } from '@/components/ui/label';
import { Input } from '@/components/ui/input';
import { ControlPoint } from '@/types';
import { T } from '@/i18n';

interface BezierControlPointPropertiesProps {
  selectedPoint: ControlPoint | null;
  onPointChange: (newPoint: ControlPoint) => void;
  max: number;
  duration: number;
}

const BezierControlPointProperties: React.FC<BezierControlPointPropertiesProps> = ({
  selectedPoint,
  onPointChange,
  max,
  duration,
}) => {
  if (!selectedPoint) {
    return (
      <Card>
        <CardContent className="pt-6">
          <p className="text-sm text-muted-foreground">
            <T>Select a point on the curve to see its properties.</T>
          </p>
        </CardContent>
      </Card>
    );
  }

  const handleTimeChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const newTimeMs = Number(e.target.value);
    if (duration > 0) {
      onPointChange({ ...selectedPoint, x: newTimeMs / duration });
    }
  };

  const handleTempChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const newTemp = Number(e.target.value);
    if (max > 0) {
      onPointChange({ ...selectedPoint, y: newTemp / max });
    }
  };

  const currentTimeMs = selectedPoint.x * duration;
  const currentTemp = selectedPoint.y * max;

  return (
    <Card>
      <CardContent className="pt-6 space-y-4">
        <div className="space-y-2">
          <Label htmlFor="cp-time"><T>Time (ms)</T></Label>
          <Input
            id="cp-time"
            type="number"
            value={Math.round(currentTimeMs)}
            onChange={handleTimeChange}
            max={duration}
            min={0}
          />
        </div>
        <div className="space-y-2">
          <Label htmlFor="cp-temp"><T>Temperature (°C)</T></Label>
          <Input
            id="cp-temp"
            type="number"
            value={Math.round(currentTemp)}
            onChange={handleTempChange}
            max={max}
            min={0}
          />
        </div>
      </CardContent>
    </Card>
  );
};

export default BezierControlPointProperties; 