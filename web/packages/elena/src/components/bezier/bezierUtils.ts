// Utility functions for the Bezier editor

// Define constants at the top
const PADDING_X = 10;
const PADDING_Y = 20;
const PADDING_BOTTOM = 80;
const PADDING_RIGHT = 50; // Extra padding on the right to prevent clipping

// Convert normalized coordinates (0-1) to SVG viewport coordinates
export const toSvgCoords = (
  point: { x: number; y: number },
  svgDimensions: { width: number; height: number }
) => {
  const usableWidth = svgDimensions.width - PADDING_X - (PADDING_X + PADDING_RIGHT);
  const usableHeight = svgDimensions.height - PADDING_Y - PADDING_BOTTOM;
  
  return {
    x: PADDING_X + (point.x * usableWidth),
    y: PADDING_Y + (point.y * usableHeight)
  };
};

// Convert SVG viewport coordinates to normalized coordinates (0-1)
export const toNormalizedCoords = (
  x: number, 
  y: number, 
  svgDimensions: { width: number; height: number }
) => {
  const usableWidth = svgDimensions.width - PADDING_X - (PADDING_X + PADDING_RIGHT);
  const usableHeight = svgDimensions.height - PADDING_Y - PADDING_BOTTOM;
  
  return {
    x: Math.max(0, Math.min(1, (x - PADDING_X) / usableWidth)),
    y: Math.max(0, Math.min(1, (y - PADDING_Y) / usableHeight))
  };
};

// Check if a control point is selectable (not the first or last point)
export const isSelectable = (index: number, totalPoints: number): boolean => {
  return index > 0 && index < totalPoints - 1;
};

// Convert a point from normalized to SVG path coordinates
export const toPathCoords = (
  point: { x: number; y: number },
  svgDimensions: { width: number; height: number }
) => {
  const usableWidth = svgDimensions.width - PADDING_X - (PADDING_X + PADDING_RIGHT);
  const usableHeight = svgDimensions.height - PADDING_Y - PADDING_BOTTOM;
  
  return {
    x: PADDING_X + (point.x * usableWidth),
    y: PADDING_Y + (point.y * usableHeight)
  };
};
