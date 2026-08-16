import React from "react";
import { ControlPoint } from "@/types";
import { toSvgCoords, isSelectable } from "./bezierUtils";
import { cn } from "@/lib/utils";

interface ControlPointsProps {
  controlPoints: ControlPoint[];
  svgDimensions: { width: number; height: number };
  selectedPointIndex: number | null;
  readonly: boolean;
  handlePointMouseDown: (index: number, e: React.MouseEvent) => void;
  handlePointClick: (index: number, e: React.MouseEvent) => void;
  handleHandleMouseDown: (index: number, e: React.MouseEvent) => void;
  handleTouchStart: (index: number, handle: 'point' | 'handle', e: React.TouchEvent) => void;
}

const ControlPoints: React.FC<ControlPointsProps> = ({
  controlPoints,
  svgDimensions,
  selectedPointIndex,
  readonly,
  handlePointMouseDown,
  handlePointClick,
  handleHandleMouseDown,
  handleTouchStart
}) => {
  return (
    <>
      {controlPoints.map((point, index) => {
        const svgPoint = toSvgCoords(point, svgDimensions);

        const nextPoint = index < controlPoints.length - 1 ? controlPoints[index + 1] : null;

        const handleX = point.handleX !== undefined
          ? point.handleX
          : (nextPoint ? point.x + (nextPoint.x - point.x) / 3 : point.x + 0.1);

        const handleY = point.handleY !== undefined
          ? point.handleY
          : (nextPoint ? point.y + (nextPoint.y - point.y) / 3 : point.y);

        const svgHandle = toSvgCoords({ x: handleX, y: handleY }, svgDimensions);

        const isSelected = selectedPointIndex === index;
        const pointType = point.type || 'linear';

        return (
          <g key={`control-${index}`}>
            <circle
              cx={svgPoint.x}
              cy={svgPoint.y}
              r={readonly ? 3 : (isSelected ? 7 : 5)}
              className={cn(
                " ",
                isSelected
                  ? "bezier-control-point-selected"
                  : "bezier-control-point",
                isSelectable(index, controlPoints.length) && !readonly && "cursor-pointer"
              )}
              onMouseDown={(e) => handlePointMouseDown(index, e)}
              onClick={(e) => handlePointClick(index, e)}
              onTouchStart={(e) => handleTouchStart(index, 'point', e)}
              data-point-id={index}
            />

            {isSelectable(index, controlPoints.length) && !readonly && pointType !== 'linear' && (
              <>
                <line
                  x1={svgPoint.x}
                  y1={svgPoint.y}
                  x2={svgHandle.x}
                  y2={svgHandle.y}
                  className="bezier-control-line"
                />
                <circle
                  cx={svgHandle.x}
                  cy={svgHandle.y}
                  r={3}
                  className={cn(
                    "bezier-handle-point",
                    !readonly && "cursor-move"
                  )}
                  onMouseDown={(e) => handleHandleMouseDown(index, e)}
                  onTouchStart={(e) => handleTouchStart(index, 'handle', e)}
                />
              </>
            )}
          </g>
        );
      })}
    </>
  );
};

export default ControlPoints;
