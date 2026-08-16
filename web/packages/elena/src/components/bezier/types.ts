import { ControlPoint } from "@/lib/api";

export type ControlPointType = 'linear' | 'quadratic' | 'cubic';

export interface BezierEditorProps {
  controlPoints: ControlPoint[];
  onChange: (points: ControlPoint[]) => void;
  minTemp: number;
  maxTemp: number;
  className?: string;
  readonly?: boolean;
  duration?: number;
  onTempRangeChange?: (minTemp: number, maxTemp: number) => void;
}
