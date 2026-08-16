import { ControlPoint } from "@/types";

export type ControlPointType = 'linear';

export interface BezierEditorProps {
  controlPoints: ControlPoint[];
  onChange: (points: ControlPoint[]) => void;
  max: number;
  className?: string;
  readonly?: boolean;
  duration?: number;
  onTempRangeChange?: (max: number) => void;
  showGridLabels?: boolean;
  elapsedTime?: number;
  isRunning?: boolean;
  currentTemp?: number;
}
