import React, { useState, useCallback, useRef } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, ResponsiveContainer } from 'recharts';
import Draggable, { DraggableEvent, DraggableData } from 'react-draggable';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { FileUp, FileDown, Plus, Trash2 } from 'lucide-react';
import { useToast } from '@/hooks/use-toast';

interface ControlPoint {
  x: number;
  y: number;
  id: string;
}

interface CurveData {
  points: ControlPoint[];
  duration: number;
  max: number;
}

const CurveEditor = () => {
  const [points, setPoints] = useState<ControlPoint[]>([
    { x: 0, y: 0, id: '1' },
    { x: 100, y: 100, id: '2' }
  ]);
  const [duration, setDuration] = useState(100);
  const [max, setMax] = useState(100);
  const chartRef = useRef<HTMLDivElement>(null);
  const { toast } = useToast();

  // Convert chart coordinates to screen coordinates
  const chartToScreen = useCallback((chartX: number, chartY: number) => {
    if (!chartRef.current) return { x: 0, y: 0 };
    
    const rect = chartRef.current.getBoundingClientRect();
    console.log('Chart rect:', rect);
    console.log('Converting chart coords:', { chartX, chartY });
    
    // Try to find the actual SVG element inside the chart
    const svgElement = chartRef.current.querySelector('svg');
    if (svgElement) {
      const svgRect = svgElement.getBoundingClientRect();
      console.log('SVG rect:', svgRect);
      
      // Try to find the actual chart area (the area with the grid)
      const chartArea = svgElement.querySelector('.recharts-cartesian-grid');
      if (chartArea) {
        const chartAreaRect = chartArea.getBoundingClientRect();
        console.log('Chart area rect:', chartAreaRect);
        
        // Use the actual chart area for positioning
        const chartWidth = chartAreaRect.width;
        const chartHeight = chartAreaRect.height;
        
        const screenX = chartAreaRect.left - rect.left + (chartX / duration) * chartWidth;
        const screenY = (chartAreaRect.top - rect.top) + (1 - chartY / max) * chartHeight;
        
        console.log('Calculated screen pos:', { screenX, screenY });
        console.log('Chart area offset from container:', chartAreaRect.top - rect.top);
        console.log('Y calculation: (1 - chartY/max) * chartHeight =', (1 - chartY / max) * chartHeight);
        return { x: screenX, y: screenY };
      }
    }
    
    // Fallback to original calculation
    const margin = { top: 20, right: 30, left: 20, bottom: 20 };
    const axisSpace = { left: 60, bottom: 40 };
    
    const chartWidth = rect.width - margin.left - margin.right - axisSpace.left;
    const chartHeight = rect.height - margin.top - margin.bottom - axisSpace.bottom;
    
    const screenX = margin.left + axisSpace.left + (chartX / duration) * chartWidth;
    const screenY = margin.top + (1 - chartY / max) * chartHeight;
    
    return { x: screenX, y: screenY };
  }, [duration, max]);

  // Convert screen coordinates to chart coordinates
  const screenToChart = useCallback((screenX: number, screenY: number) => {
    if (!chartRef.current) return { x: 0, y: 0 };
    
    const rect = chartRef.current.getBoundingClientRect();
    // Use same dimensions as chartToScreen
    const margin = { top: 20, right: 30, left: 20, bottom: 20 };
    const axisSpace = { left: 60, bottom: 40 };
    
    const chartWidth = rect.width - margin.left - margin.right - axisSpace.left;
    const chartHeight = rect.height - margin.top - margin.bottom - axisSpace.bottom;
    
    const chartX = Math.max(0, Math.min(duration, ((screenX - margin.left - axisSpace.left) / chartWidth) * duration));
    const chartY = Math.max(0, Math.min(max, (1 - (screenY - margin.top) / chartHeight) * max));
    
    return { x: chartX, y: chartY };
  }, [duration, max]);

  const handleDrag = useCallback((pointId: string, e: DraggableEvent, data: DraggableData) => {
    const chartCoords = screenToChart(data.x, data.y);
    updatePoint(pointId, chartCoords.x, chartCoords.y);
  }, [screenToChart]);

  // Generate smooth curve data for visualization
  const generateCurveData = useCallback(() => {
    const sortedPoints = [...points].sort((a, b) => a.x - b.x);
    const data = [];
    
    for (let i = 0; i <= duration; i += duration / 100) {
      let y = 0;
      
      // Simple linear interpolation between points
      if (sortedPoints.length >= 2) {
        for (let j = 0; j < sortedPoints.length - 1; j++) {
          const p1 = sortedPoints[j];
          const p2 = sortedPoints[j + 1];
          
          if (i >= p1.x && i <= p2.x) {
            const t = (i - p1.x) / (p2.x - p1.x);
            y = p1.y + t * (p2.y - p1.y);
            break;
          }
        }
        
        // Handle points outside the range
        if (i < sortedPoints[0].x) {
          y = sortedPoints[0].y;
        } else if (i > sortedPoints[sortedPoints.length - 1].x) {
          y = sortedPoints[sortedPoints.length - 1].y;
        }
      }
      
      data.push({ x: i, y: Math.max(0, Math.min(max, y)) });
    }
    
    return data;
  }, [points, duration, max]);

  const addPoint = () => {
    const newPoint: ControlPoint = {
      x: duration / 2,
      y: max / 2,
      id: Date.now().toString()
    };
    setPoints([...points, newPoint]);
  };

  const removePoint = (id: string) => {
    if (points.length > 2) {
      setPoints(points.filter(p => p.id !== id));
    } else {
      toast({
        title: "Cannot remove point",
        description: "At least 2 control points are required.",
        variant: "destructive"
      });
    }
  };

  const handleChartDoubleClick = (e: React.MouseEvent) => {
    if (!chartRef.current) return;
    
    const rect = chartRef.current.getBoundingClientRect();
    const mouseX = e.clientX - rect.left;
    const mouseY = e.clientY - rect.top;
    
    const chartCoords = screenToChart(mouseX, mouseY);
    
    const newPoint: ControlPoint = {
      x: chartCoords.x,
      y: chartCoords.y,
      id: Date.now().toString()
    };
    
    setPoints([...points, newPoint]);
    
    toast({
      title: "Point added",
      description: `New control point added at (${chartCoords.x.toFixed(1)}, ${chartCoords.y.toFixed(1)})`
    });
  };

  const handlePointDoubleClick = (pointId: string, e: React.MouseEvent) => {
    e.stopPropagation();
    
    const sortedPoints = [...points].sort((a, b) => a.x - b.x);
    const point = points.find(p => p.id === pointId);
    if (!point) return;
    
    const isFirstOrLast = point.id === sortedPoints[0].id || point.id === sortedPoints[sortedPoints.length - 1].id;
    
    if (isFirstOrLast) {
      toast({
        title: "Cannot remove point",
        description: "First and last points cannot be removed.",
        variant: "destructive"
      });
      return;
    }
    
    removePoint(pointId);
    
    toast({
      title: "Point removed",
      description: "Control point removed successfully."
    });
  };

  const updatePoint = (id: string, newX: number, newY: number) => {
    setPoints(points.map(p => {
      if (p.id !== id) return p;
      
      // Sort points to identify first and last
      const sortedPoints = [...points].sort((a, b) => a.x - b.x);
      const isFirst = p.id === sortedPoints[0].id;
      const isLast = p.id === sortedPoints[sortedPoints.length - 1].id;
      
      // Constrain coordinates
      const constrainedY = Math.max(0, Math.min(max, newY));
      let constrainedX = Math.max(0, Math.min(duration, newX));
      
      // Stick first and last points to boundaries
      if (isFirst) constrainedX = 0;
      if (isLast) constrainedX = duration;
      
      return { ...p, x: constrainedX, y: constrainedY };
    }));
  };

  const downloadJSON = () => {
    const data: CurveData = {
      points,
      duration,
      max
    };
    
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'curve-data.json';
    a.click();
    URL.revokeObjectURL(url);
    
    toast({
      title: "Downloaded",
      description: "Curve data saved as JSON file."
    });
  };

  const uploadJSON = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const data: CurveData = JSON.parse(e.target?.result as string);
        if (data.points && Array.isArray(data.points)) {
          setPoints(data.points);
          if (data.duration) setDuration(data.duration);
          if (data.max) setMax(data.max);
          
          toast({
            title: "Uploaded",
            description: "Curve data loaded successfully."
          });
        }
      } catch (error) {
        toast({
          title: "Error",
          description: "Invalid JSON file format.",
          variant: "destructive"
        });
      }
    };
    reader.readAsText(file);
    
    // Reset the input
    event.target.value = '';
  };

  const curveData = generateCurveData();

  return (
    <div className="w-full max-w-6xl mx-auto p-6 space-y-6">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center justify-between">
            Curve Editor
            <div className="flex gap-2">
              <Button onClick={addPoint} size="sm" variant="outline">
                <Plus className="w-4 h-4 mr-2" />
                Add Point
              </Button>
              <Button onClick={downloadJSON} size="sm" variant="outline">
                <FileDown className="w-4 h-4 mr-2" />
                Download
              </Button>
              <label className="cursor-pointer">
                <Button size="sm" variant="outline" asChild>
                  <span>
                    <FileUp className="w-4 h-4 mr-2" />
                    Upload
                  </span>
                </Button>
                <input
                  type="file"
                  accept=".json"
                  onChange={uploadJSON}
                  className="hidden"
                />
              </label>
            </div>
          </CardTitle>
        </CardHeader>
        <CardContent className="space-y-6">
          {/* Parameters */}
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div className="space-y-2">
              <Label htmlFor="duration">Duration (X-axis max)</Label>
              <Input
                id="duration"
                type="number"
                value={duration}
                onChange={(e) => setDuration(Number(e.target.value))}
                min="1"
              />
            </div>
            <div className="space-y-2">
              <Label htmlFor="max">Max Value (Y-axis max)</Label>
              <Input
                id="max"
                type="number"
                value={max}
                onChange={(e) => setMax(Number(e.target.value))}
                min="1"
              />
            </div>
          </div>

          {/* Chart */}
          <div className="h-96 w-full relative" ref={chartRef} onDoubleClick={handleChartDoubleClick}>
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={curveData} margin={{ top: 20, right: 30, left: 20, bottom: 20 }}>
                <CartesianGrid strokeDasharray="3 3" className="stroke-muted" />
                <XAxis 
                  dataKey="x" 
                  domain={[0, duration]}
                  type="number"
                  scale="linear"
                  label={{ value: 'Duration', position: 'insideBottom', offset: -10 }}
                />
                <YAxis 
                  domain={[0, max]}
                  label={{ value: 'Value', angle: -90, position: 'insideLeft' }}
                />
                <Line 
                  type="linear" 
                  dataKey="y" 
                  stroke="hsl(var(--primary))" 
                  strokeWidth={2}
                  dot={false}
                />
              </LineChart>
            </ResponsiveContainer>
            
            {/* Draggable Control Points */}
            {points.map((point) => {
              const screenPos = chartToScreen(point.x, point.y);
              return (
                <div
                  key={point.id}
                  className="absolute"
                  style={{
                    left: screenPos.x,
                    top: screenPos.y,
                    transform: 'translate(-50%, -50%)',
                    zIndex: 10
                  }}
                >
                  <Draggable
                    position={{ x: 0, y: 0 }}
                    onDrag={(e, data) => {
                      const chartCoords = screenToChart(screenPos.x + data.x, screenPos.y + data.y);
                      updatePoint(point.id, chartCoords.x, chartCoords.y);
                    }}
                  >
                    <div 
                      className="w-3 h-3 bg-primary border-2 border-background rounded-full cursor-move shadow-sm hover:scale-110 transition-transform" 
                      onDoubleClick={(e) => handlePointDoubleClick(point.id, e)}
                    />
                  </Draggable>
                </div>
              );
            })}
          </div>

          {/* Control Points List */}
          <div className="space-y-4">
            <h3 className="text-lg font-medium">Control Points</h3>
            <div className="grid gap-3">
              {points.map((point, index) => (
                <div key={point.id} className="flex items-center gap-4 p-3 border rounded-lg">
                  <span className="text-sm font-medium w-16">Point {index + 1}</span>
                  <div className="flex items-center gap-2">
                    <Label htmlFor={`x-${point.id}`} className="text-xs">X:</Label>
                    <Input
                      id={`x-${point.id}`}
                      type="number"
                      value={point.x}
                      onChange={(e) => updatePoint(point.id, Number(e.target.value), point.y)}
                      className="w-20"
                      min="0"
                      max={duration}
                    />
                  </div>
                  <div className="flex items-center gap-2">
                    <Label htmlFor={`y-${point.id}`} className="text-xs">Y:</Label>
                    <Input
                      id={`y-${point.id}`}
                      type="number"
                      value={point.y}
                      onChange={(e) => updatePoint(point.id, point.x, Number(e.target.value))}
                      className="w-20"
                      min="0"
                      max={max}
                    />
                  </div>
                  <Button
                    onClick={() => removePoint(point.id)}
                    size="sm"
                    variant="outline"
                    disabled={points.length <= 2}
                  >
                    <Trash2 className="w-4 h-4" />
                  </Button>
                </div>
              ))}
            </div>
          </div>
        </CardContent>
      </Card>
    </div>
  );
};

export default CurveEditor;