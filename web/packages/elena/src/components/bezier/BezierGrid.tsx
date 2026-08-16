import React from "react";
import { toSvgCoords } from "./bezierUtils";

// Import constants from bezierUtils (define them again here since we can't import constants directly)
const PADDING_X = 10;
const PADDING_Y = 20;
const PADDING_BOTTOM = 80;
const PADDING_RIGHT = 50;

interface BezierGridProps {
  svgDimensions: { width: number; height: number };
  minTemp: number;
  maxTemp: number;
  duration?: number;
}

const BezierGrid: React.FC<BezierGridProps> = ({ 
  svgDimensions, 
  minTemp, 
  maxTemp,
  duration = 120
}) => {
  const renderTempLabels = () => {
    const labels = [];
    const steps = 5;
    
    for (let i = 0; i <= steps; i++) {
      const y = i / steps;
      const temp = maxTemp - (maxTemp - minTemp) * y;
      const svgCoords = toSvgCoords({ x: 0, y }, svgDimensions);
      
      labels.push(
        <text
          key={`label-${i}`}
          x={5}
          y={svgCoords.y}
          className="text-xs fill-muted-foreground"
          alignmentBaseline="middle"
          data-testid="temp-label"
          style={{ pointerEvents: 'none' }}
        >
          {Math.round(temp)}°C
        </text>
      );
    }
    
    return labels;
  };
  
  const renderTimeLabels = () => {
    const labels = [];
    const steps = 5;
    
    for (let i = 0; i <= steps; i++) {
      const x = i / steps;
      const timeInMinutes = Math.round(x * duration);
      const svgCoords = toSvgCoords({ x, y: 1 }, svgDimensions);
      
      // For the last label, adjust position to prevent clipping
      const xPos = i === steps ? svgCoords.x - 20 : svgCoords.x;
      
      labels.push(
        <text
          key={`time-${i}`}
          x={xPos}
          y={svgDimensions.height - PADDING_BOTTOM / 2}
          textAnchor={i === steps ? "end" : "middle"}
          className="text-xs fill-muted-foreground"
          data-testid="time-label"
          style={{ pointerEvents: 'none' }}
        >
          {timeInMinutes} min
        </text>
      );
    }
    
    return labels;
  };
  
  const renderGridLines = () => {
    const horizontalLines = [0.2, 0.4, 0.6, 0.8].map(y => {
      const { y: svgY } = toSvgCoords({ x: 0, y }, svgDimensions);
      return (
        <line
          key={`grid-y-${y}`}
          x1={PADDING_X}
          y1={svgY}
          x2={svgDimensions.width - PADDING_X - PADDING_RIGHT}
          y2={svgY}
          className="stroke-muted/80 stroke-dasharray-2"
          style={{ pointerEvents: 'none' }}
        />
      );
    });
    
    const verticalLines = [0.2, 0.4, 0.6, 0.8].map(x => {
      const { x: svgX } = toSvgCoords({ x, y: 0 }, svgDimensions);
      return (
        <line
          key={`grid-x-${x}`}
          x1={svgX}
          y1={PADDING_Y}
          x2={svgX}
          y2={svgDimensions.height - PADDING_BOTTOM}
          className="stroke-muted/80 stroke-dasharray-2"
          style={{ pointerEvents: 'none' }}
        />
      );
    });
    
    return [...horizontalLines, ...verticalLines];
  };
  
  const renderXAxis = () => {
    const startPoint = toSvgCoords({ x: 0, y: 1 }, svgDimensions);
    const endPoint = toSvgCoords({ x: 1, y: 1 }, svgDimensions);
    
    // Draw the markers for each 20% on the x-axis
    const markers = [0, 0.2, 0.4, 0.6, 0.8, 1].map(x => {
      const point = toSvgCoords({ x, y: 1 }, svgDimensions);
      return (
        <line
          key={`marker-${x}`}
          x1={point.x}
          y1={point.y}
          x2={point.x}
          y2={point.y + 5}
          stroke="#64748b"
          strokeWidth="1"
          style={{ pointerEvents: 'none' }}
        />
      );
    });
    
    return (
      <>
        <line
          x1={startPoint.x}
          y1={startPoint.y}
          x2={endPoint.x}
          y2={endPoint.y}
          stroke="#64748b"
          strokeWidth="1"
          style={{ pointerEvents: 'none' }}
        />
        {markers}
      </>
    );
  };
  
  const renderYAxis = () => {
    const startPoint = toSvgCoords({ x: 0, y: 0 }, svgDimensions);
    const endPoint = toSvgCoords({ x: 0, y: 1 }, svgDimensions);
    
    return (
      <line
        x1={startPoint.x}
        y1={startPoint.y}
        x2={endPoint.x}
        y2={endPoint.y}
        stroke="#64748b"
        strokeWidth="1"
        style={{ pointerEvents: 'none' }}
      />
    );
  };
  
  return (
    <>
      {renderGridLines()}
      {renderTempLabels()}
      {renderXAxis()}
      {renderYAxis()}
      {renderTimeLabels()}
    </>
  );
};

export default BezierGrid;
