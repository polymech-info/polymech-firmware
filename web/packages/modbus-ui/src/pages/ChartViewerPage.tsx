import React from 'react';
import RealTimeChart from '@/components/RealTimeChart';
import { T } from '@/i18n';
import ConnectionManager from '@/components/ConnectionManager';

const ChartViewerPage: React.FC = () => {
  return (
    <div className="p-4 bg-background text-foreground min-h-screen">
      <div className="flex justify-between items-start mb-4">
        <h1 className="text-2xl font-bold">
          <T>Chart Viewer</T>
        </h1>
        <ConnectionManager />
      </div>
      <RealTimeChart />
    </div>
  );
};

export default ChartViewerPage; 