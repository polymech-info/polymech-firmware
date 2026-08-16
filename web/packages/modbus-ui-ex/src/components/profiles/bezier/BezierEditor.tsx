import React, { useState, useRef, useEffect } from 'react';

import { ControlPoint } from '@/types';
import { getBezierPath } from '@/lib/bezier';

import { cn } from '@/lib/utils';
import { BezierEditorProps, ControlPointType } from './types';
import { toNormalizedCoords, isSelectable, toPathCoords } from './bezierUtils';
import BezierGrid from './BezierGrid';
import ControlPoints from './ControlPoints';
import PointTypeControls from './PointTypeControls';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';

const BezierEditor: React.FC<BezierEditorProps> = ({
  controlPoints,
  onChange,
  max,
  className,
  readonly = false,
  duration = 120,
  onTempRangeChange,
  showGridLabels = true,
  elapsedTime,
  isRunning,
  currentTemp,
}) => {
  const [activePointIndex, setActivePointIndex] = useState<number | null>(null);
  const [activeHandle, setActiveHandle] = useState<'point' | 'handle' | null>(null);
  const [selectedPointIndex, setSelectedPointIndex] = useState<number | null>(null);
  const svgRef = useRef<SVGSVGElement>(null);
  const [svgDimensions, setSvgDimensions] = useState({ width: 0, height: 0 });
  const [isDragging, setIsDragging] = useState(false);
  const [tempRange, setTempRange] = useState({ max: max });
  
  useEffect(() => {
    if (isDragging) {
      document.body.style.userSelect = 'none';
      document.body.style.webkitUserSelect = 'none';
    } else {
      document.body.style.userSelect = '';
      document.body.style.webkitUserSelect = '';
    }
    
    return () => {
      document.body.style.userSelect = '';
      document.body.style.webkitUserSelect = '';
    };
  }, [isDragging]);
  
  useEffect(() => {
    setTempRange({ max: max });
  }, [max]);
  
  useEffect(() => {
    const updateDimensions = () => {
      if (svgRef.current) {
        const rect = svgRef.current.getBoundingClientRect();
        setSvgDimensions({ width: rect.width, height: rect.height });
      }
    };
    
    updateDimensions();
    window.addEventListener('resize', updateDimensions);
    
    return () => window.removeEventListener('resize', updateDimensions);
  }, []);
  
  const handleTempRangeChange = (key: 'min' | 'max', value: string) => {
    const numValue = Number(value);
    if (isNaN(numValue)) return;
    
    const newRange = { ...tempRange, [key]: numValue };
        
    setTempRange(newRange);
    
    if (onTempRangeChange) {
      onTempRangeChange(newRange.max);
    }
  };
  
  const handlePointClick = (index: number, e: React.MouseEvent) => {
    if (readonly) return;
    e.stopPropagation();
    e.preventDefault();
    
    setSelectedPointIndex(index);
  };
  
  const handlePointMouseDown = (index: number, e: React.MouseEvent) => {
    if (readonly) return;
    e.stopPropagation();
    e.preventDefault();
    
    setActivePointIndex(index);
    setActiveHandle('point');
    setIsDragging(true);
    
    setSelectedPointIndex(index);
    
    window.addEventListener('mousemove', handleGlobalMouseMove);
    window.addEventListener('mouseup', handleGlobalMouseUp);
  };
  
  const handleHandleMouseDown = (index: number, e: React.MouseEvent) => {
    if (readonly) return;
    e.stopPropagation();
    e.preventDefault();
    
    setActivePointIndex(index);
    setActiveHandle('handle');
    setIsDragging(true);
    
    setSelectedPointIndex(index);
    
    window.addEventListener('mousemove', handleGlobalMouseMove);
    window.addEventListener('mouseup', handleGlobalMouseUp);
  };
  
  const handleGlobalMouseMove = (e: MouseEvent) => {
    if (readonly || activePointIndex === null || !activeHandle) return;
    
    const svgRect = svgRef.current?.getBoundingClientRect();
    if (!svgRect) return;
    
    const x = e.clientX - svgRect.left;
    const y = e.clientY - svgRect.top;
    const normalizedCoords = toNormalizedCoords(x, y, svgDimensions);
    
    const newPoints = [...controlPoints];
    
    if (activeHandle === 'point') {
      if (activePointIndex === 0) {
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          y: normalizedCoords.y
        };
      } else if (activePointIndex === newPoints.length - 1) {
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          y: normalizedCoords.y
        };
      } else {
        const prevPoint = newPoints[activePointIndex - 1];
        const nextPoint = newPoints[activePointIndex + 1];
        
        const boundedX = Math.max(prevPoint.x, Math.min(nextPoint.x, normalizedCoords.x));
        
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          x: boundedX,
          y: normalizedCoords.y
        };
      }
    } else if (activeHandle === 'handle') {
      newPoints[activePointIndex] = {
        ...newPoints[activePointIndex],
        x: normalizedCoords.x,
        y: normalizedCoords.y
      };
    }
    onChange(newPoints);
  };
  
  const handleGlobalMouseUp = (e: MouseEvent) => {
    if (!isDragging) return;
    
    if (activePointIndex !== null) {
      setSelectedPointIndex(activePointIndex);
    }
    
    setActivePointIndex(null);
    setActiveHandle(null);
    setIsDragging(false);
    
    window.removeEventListener('mousemove', handleGlobalMouseMove);
    window.removeEventListener('mouseup', handleGlobalMouseUp);
  };
  
  const handleMouseMove = (e: React.MouseEvent) => {
    if (isDragging) {
      return;
    }
  };
  
  const handleMouseUp = (e: React.MouseEvent | React.TouchEvent) => {
    if (isDragging) {
      return;
    }
  };
  
  const handleSvgMouseLeave = () => {
    // This handler is now redundant as we're using global mouse up
  };
  
  const handleDoubleClick = (e: React.MouseEvent) => {
    if (readonly) return;
    
    const svgRect = svgRef.current?.getBoundingClientRect();
    if (!svgRect) return;
    
    if (selectedPointIndex !== null && selectedPointIndex !== 0 && selectedPointIndex !== controlPoints.length - 1) {
      const newPoints = [...controlPoints];
      newPoints.splice(selectedPointIndex, 1);
      onChange(newPoints);
      setSelectedPointIndex(null);
      return;
    }
    
    const x = e.clientX - svgRect.left;
    const y = e.clientY - svgRect.top;
    const normalizedCoords = toNormalizedCoords(x, y, svgDimensions);
    
    const newPoints = [...controlPoints];
    let insertIndex = 1;
    
    for (let i = 0; i < newPoints.length - 1; i++) {
      if (normalizedCoords.x > newPoints[i].x && normalizedCoords.x < newPoints[i + 1].x) {
        insertIndex = i + 1;
        break;
      }
    }
    
    const newPoint: ControlPoint = {
      x: normalizedCoords.x,
      y: normalizedCoords.y
    };
    
    newPoints.splice(insertIndex, 0, newPoint);
    onChange(newPoints);
    setSelectedPointIndex(insertIndex);
    
    e.stopPropagation();
  };
  
  const handleSvgClick = (e: React.MouseEvent) => {
    if (e.nativeEvent.offsetY > svgDimensions.height - 40) {
      return;
    }
    
    if (!isDragging) {
      setSelectedPointIndex(null);
    }
  };
  
  const handleTouchStart = (index: number, handle: 'point' | 'handle', e: React.TouchEvent) => {
    if (readonly) return;
    e.stopPropagation();
    
    setActivePointIndex(index);
    setActiveHandle(handle);
    setIsDragging(true);
    setSelectedPointIndex(index);
  };
  
  const handleTouchMove = (e: React.TouchEvent) => {
    if (readonly || activePointIndex === null || !activeHandle) return;
    
    e.preventDefault();
    
    const touch = e.touches[0];
    const svgRect = svgRef.current?.getBoundingClientRect();
    if (!svgRect || !touch) return;
    
    const x = touch.clientX - svgRect.left;
    const y = touch.clientY - svgRect.top;
    const normalizedCoords = toNormalizedCoords(x, y, svgDimensions);
    
    const newPoints = [...controlPoints];
    
    if (activeHandle === 'point') {
      if (activePointIndex === 0) {
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          y: normalizedCoords.y
        };
      } else if (activePointIndex === newPoints.length - 1) {
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          y: normalizedCoords.y
        };
      } else {
        const prevPoint = newPoints[activePointIndex - 1];
        const nextPoint = newPoints[activePointIndex + 1];
        
        const boundedX = Math.max(prevPoint.x, Math.min(nextPoint.x, normalizedCoords.x));
        
        newPoints[activePointIndex] = {
          ...newPoints[activePointIndex],
          x: boundedX,
          y: normalizedCoords.y
        };
      }
    } else if (activeHandle === 'handle') {
      newPoints[activePointIndex] = {
        ...newPoints[activePointIndex],
        x: normalizedCoords.x,
        y: normalizedCoords.y
      };
    }
    
    // Log the points being sent to onChange in handleTouchMove
    console.log('BezierEditor (Touch): Calling onChange with newPoints:', JSON.parse(JSON.stringify(newPoints)));
    onChange(newPoints);
  };
  
  const handleTouchEnd = () => {
    if (activePointIndex !== null) {
      setSelectedPointIndex(activePointIndex);
    }
    
    setActivePointIndex(null);
    setActiveHandle(null);
    setIsDragging(false);
  };
  
  const pathPoints = getBezierPath(controlPoints);
  
  // Generate the SVG path data
  let pathData = '';
  if (pathPoints.length > 0) {
    const svgPoints = pathPoints.map(point => toPathCoords(point, svgDimensions));
    pathData = `M ${svgPoints[0].x},${svgPoints[0].y}`;
    
    for (let i = 1; i < svgPoints.length; i++) {
      pathData += ` L ${svgPoints[i].x},${svgPoints[i].y}`;
    }
  }
  
  // Calculate elapsed time line position
  let elapsedLineX: number | null = null;
  if (isRunning && elapsedTime !== undefined && duration > 0 && svgDimensions.width > 0) {
    const elapsedRatio = elapsedTime / duration;
    // Ensure the line is within the drawable area of the SVG, considering padding
    // The toPathCoords function implicitly handles some of this via svgDimensions,
    // but we need to be careful about how x is normalized for elapsed time.
    // Assuming the grid and path start effectively at x=0 of the main plotting area.
    // The BezierGrid component and toPathCoords will use svgDimensions which includes padding.
    // For simplicity, let's assume the plotting area for the x-axis (time) directly maps to svgDimensions.width for now.
    // A more precise calculation might need to subtract horizontal padding from svgDimensions.width
    // if the path doesn't start at the very edge.
    // For toPathCoords, x:0 is left edge, x:1 is right edge of the *control point space*.
    // We want the line to be at elapsedRatio of the *visual width* available for the graph.
    // Let's use a simplified approach: map elapsedRatio directly to the SVG's width used for the path.
    // The path itself is drawn using toPathCoords, which maps [0,1] x-coordinates to the SVG canvas.
    // So, elapsedRatio (which is already 0 to 1 if elapsedTime <= duration) can be used with toPathCoords.
    const { x: lineX } = toPathCoords({ x: elapsedRatio, y: 0 }, svgDimensions); 
    elapsedLineX = lineX;
  }
  
  // Calculate current temperature point position
  let currentTempCoords: { x: number; y: number } | null = null;
  if (isRunning && elapsedLineX !== null && currentTemp !== undefined && tempRange.max > 0) {
    // Normalize currentTemp: y=0 is max temp, y=1 is 0 temp for toPathCoords
    // So, we need (tempRange.max - currentTemp) / tempRange.max to get the correct normalized Y
    // However, toPathCoords expects y=0 for the bottom of the graph (min temp usually 0) and y=1 for top (max temp).
    // Control points are {x: 0-1 (time), y: 0-1 (temp where 0 is min, 1 is max)}
    // So, currentTemp / tempRange.max should be the correct normalized Y for toPathCoords
    const normalizedCurrentTempY = currentTemp / tempRange.max;
    currentTempCoords = toPathCoords({ x: (elapsedTime || 0) / duration, y: normalizedCurrentTempY }, svgDimensions);
  }
  
  const isPointControlsDisabled = selectedPointIndex === null || 
    !isSelectable(selectedPointIndex, controlPoints.length) || 
    readonly;
  
  return (
    <div className="space-y-2 select-none">
      {!readonly && (
        <div className="flex flex-row gap-4 mb-2">
          <div className="w-1/2">
            <Label htmlFor="max-temp">Max Temperature (°C)</Label>
            <Input
              id="max-temp"
              type="number"
              value={tempRange.max}
              onChange={(e) => handleTempRangeChange('max', e.target.value)}
              min={0}
              max={400}
              step={1}
            />
          </div>
        </div>
      )}
      <svg
        ref={svgRef}
        className={cn("w-full h-80 touch-none", className)}
        onMouseMove={handleMouseMove}
        onMouseUp={handleMouseUp}
        onMouseLeave={handleSvgMouseLeave}
        onClick={handleSvgClick}
        onDoubleClick={handleDoubleClick}
        onTouchMove={handleTouchMove}
        onTouchEnd={handleTouchEnd}
        style={{ padding: '15px 25px 50px 15px' }}
      >
        <rect
          x="0"
          y="0"
          width={svgDimensions.width}
          height={svgDimensions.height}
          fill="transparent"
        />
        
        <BezierGrid
          svgDimensions={svgDimensions}
          max={tempRange.max}
          duration={duration}
          showGridLabels={showGridLabels}
        />
        
        <path 
          d={pathData} 
          className="bezier-curve-path"
          stroke="blue"
          strokeWidth="2"
          fill="none"
        />
        
        {/* Progress Line */}
        {isRunning && elapsedLineX !== null && (
          <line
            x1={elapsedLineX}
            y1={toPathCoords({x:0, y:0}, svgDimensions).y} // Top of the graph area (y=0 normalized)
            x2={elapsedLineX}
            y2={toPathCoords({x:0, y:1}, svgDimensions).y} // Bottom of the graph area (y=1 normalized)
            stroke="orange"
            strokeWidth="2"
            strokeDasharray="4 2"
          />
        )}

        {/* Current Temperature Marker */}
        {isRunning && currentTempCoords && (
          <circle
            cx={currentTempCoords.x}
            cy={currentTempCoords.y}
            r="4"
            fill="red"
            stroke="white"
            strokeWidth="1"
          />
        )}

        <ControlPoints
          controlPoints={controlPoints}
          svgDimensions={svgDimensions}
          selectedPointIndex={selectedPointIndex}
          readonly={readonly || false}
          handlePointMouseDown={handlePointMouseDown}
          handlePointClick={handlePointClick}
          handleHandleMouseDown={handleHandleMouseDown}
          handleTouchStart={handleTouchStart}
        />
      </svg>
      
    </div>
  );
};

export default BezierEditor;
