import React from 'react';
import LogViewer from '@/components/LogViewer';

interface LogViewerWidgetProps {
  widgetInstanceId?: string;
  onPropsChange?: (newProps: Record<string, any>) => void;
  
  // Widget-specific props with defaults
  height?: number;
  autoScroll?: boolean;
  defaultTab?: string;
  showSearch?: boolean;
  showFilters?: boolean;
  showControls?: boolean;
  maxLogEntries?: number;
}

const LogViewerWidget: React.FC<LogViewerWidgetProps> = ({
  widgetInstanceId,
  onPropsChange,
  height = 400,
  autoScroll = true,
  defaultTab = 'all',
  showSearch = true,
  showFilters = true,
  showControls = true,
  maxLogEntries = 1000,
}) => {
  return (
    <div 
      className="w-full" 
      style={{ height: `${height}px` }}
    >
      <LogViewer />
    </div>
  );
};

export default LogViewerWidget;






