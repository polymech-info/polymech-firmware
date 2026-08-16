import { NICE_TIME_INTERVALS_MS } from './SamplePlot'; // Assuming SamplePlot.ts is in the same directory

export const MIN_PIXEL_SPACING_FOR_MARKERS = 60; // Minimum pixels between timeline markers

export const calculateNiceTimeInterval = (totalDurationMs: number, timelineWidthPx: number): number => {
  if (totalDurationMs <= 0 || timelineWidthPx <= 0) {
    return NICE_TIME_INTERVALS_MS[0]; // Default to 1s if no duration/width
  }
  const maxMarkers = Math.floor(timelineWidthPx / MIN_PIXEL_SPACING_FOR_MARKERS);
  if (maxMarkers <= 1) { // If can't even fit 2 markers, use a large interval or just one end marker logic elsewhere
    return totalDurationMs > NICE_TIME_INTERVALS_MS[NICE_TIME_INTERVALS_MS.length -1] ? totalDurationMs : NICE_TIME_INTERVALS_MS[NICE_TIME_INTERVALS_MS.length -1];
  }
  const minIntervalMs = totalDurationMs / maxMarkers;

  for (const niceInterval of NICE_TIME_INTERVALS_MS) {
    if (niceInterval >= minIntervalMs) {
      return niceInterval;
    }
  }
  // If no nice interval is small enough (e.g., very long duration, small width), 
  // return the largest nice interval or calculate a dynamic one.
  // For simplicity, return largest or a fraction of total duration if very large.
  return Math.max(NICE_TIME_INTERVALS_MS[NICE_TIME_INTERVALS_MS.length - 1], Math.ceil(totalDurationMs / 10 / 1000) * 1000); 
};

export const formatMarkerLabel = (ms: number): string => {
  if (ms < 0) ms = 0;
  const totalSeconds = Math.floor(ms / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;

  if (hours > 0) {
    return `${hours}h${minutes > 0 ? `${minutes}m` : ''}`.trim(); 
  }
  if (minutes > 0) {
    return `${minutes}m${seconds > 0 ? `${seconds}s` : ''}`.trim(); 
  }
  return `${seconds}s`;
}; 