import React, { useState, useRef, useEffect } from 'react';
import { ControlPoint } from '@/lib/api';
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
  minTemp,
  maxTemp,
  className,
  readonly = false,
  duration = 120,
  onTempRangeChange,
}) => {
  const [activePointIndex, setActivePointIndex] = useState<number | null>(null);
  const [activeHandle, setActiveHandle] = useState<'point' | 'handle' | null>(null);
  const [selectedPointIndex, setSelectedPointIndex] = useState<number | null>(null);
  const svgRef = useRef<SVGSVGElement>(null);
  const [svgDimensions, setSvgDimensions] = useState({ width: 0, height: 0 });
  const [isDragging, setIsDragging] = useState(false);
  const [tempRange, setTempRange] = useState({ min: minTemp, max: maxTemp });
  
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
    setTempRange({ min: minTemp, max: maxTemp });
  }, [minTemp, maxTemp]);
  
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
    
    // Ensure min < max
    if (newRange.min >= newRange.max) {
      if (key === 'min') {
        newRange.min = newRange.max - 1;
      } else {
        newRange.max = newRange.min + 1;
      }
    }
    
    setTempRange(newRange);
    
    if (onTempRangeChange) {
      onTempRangeChange(newRange.min, newRange.max);
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
        handleX: normalizedCoords.x,
        handleY: normalizedCoords.y
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
      y: normalizedCoords.y,
      type: 'linear'
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
        handleX: normalizedCoords.x,
        handleY: normalizedCoords.y
      };
    }
    
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
  
  const changePointType = (type: ControlPointType) => {
    if (selectedPointIndex === null || readonly) return;
    
    const newPoints = [...controlPoints];
    const point = newPoints[selectedPointIndex];
    
    if (selectedPointIndex === 0 || selectedPointIndex === newPoints.length - 1) {
      return;
    }
    
    switch (type) {
      case 'linear':
        newPoints[selectedPointIndex] = {
          x: point.x,
          y: point.y,
          type: 'linear'
        };
        break;
      case 'quadratic':
        newPoints[selectedPointIndex] = {
          x: point.x,
          y: point.y,
          handleX: point.x + 0.05,
          handleY: point.y,
          type: 'quadratic'
        };
        break;
      case 'cubic':
        newPoints[selectedPointIndex] = {
          x: point.x,
          y: point.y,
          handleX: point.x + 0.05,
          handleY: point.y,
          type: 'cubic'
        };
        break;
    }
    
    onChange(newPoints);
  };
  
  const getSelectedPointType = (): ControlPointType => {
    if (selectedPointIndex === null) return 'linear';
    
    const point = controlPoints[selectedPointIndex];
    return (point.type as ControlPointType) || 'linear';
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
  
  const isPointControlsDisabled = selectedPointIndex === null || 
    !isSelectable(selectedPointIndex, controlPoints.length) || 
    readonly;
  
  return (
    <div className="space-y-2 select-none">
      {!readonly && (
        <div className="flex flex-row gap-4 mb-2">
          <div className="w-1/2">
            <Label htmlFor="min-temp">Min Temperature (°C)</Label>
            <Input
              id="min-temp"
              type="number"
              value={tempRange.min}
              onChange={(e) => handleTempRangeChange('min', e.target.value)}
              min={-50}
              max={tempRange.max - 1}
              step={1}
            />
          </div>
          <div className="w-1/2">
            <Label htmlFor="max-temp">Max Temperature (°C)</Label>
            <Input
              id="max-temp"
              type="number"
              value={tempRange.max}
              onChange={(e) => handleTempRangeChange('max', e.target.value)}
              min={tempRange.min + 1}
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
          minTemp={tempRange.min}
          maxTemp={tempRange.max}
          duration={duration}
        />
        
        <path 
          d={pathData} 
          className="bezier-curve-path"
          stroke="blue"
          strokeWidth="2"
          fill="none"
        />
        
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
      
      <PointTypeControls
        selectedType={getSelectedPointType()}
        onChange={changePointType}
        disabled={isPointControlsDisabled}
      />
    </div>
  );
};

export default BezierEditor;
