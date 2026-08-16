import { ControlPoint, SSignalControlPoint } from '@/types';

export interface BezierEditorProps {
  controlPoints: ControlPoint[];
  onChange: (points: ControlPoint[]) => void;
  max: number;
  className?: string;
  readonly?: boolean;
  duration?: number;
  onTempRangeChange?: (newMax: number) => void;
  showGridLabels?: boolean;
  elapsedTime?: number;
  isRunning?: boolean;
  currentTemp?: number;
  signalControlPoints?: SSignalControlPoint[];
  pressureProfile?: any; // Pressure profile for overlay
  activeView?: 'temperature' | 'pressure'; // Active view toggle
  temperatureControlPoints?: ControlPoint[]; // Original temperature control points for background display
}

export type ControlPointType = 'corner' | 'bezier';
