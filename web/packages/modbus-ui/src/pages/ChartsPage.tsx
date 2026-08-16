import React from 'react';
import RealTimeChart from '@/components/RealTimeChart';
import { T } from '@/i18n';

const ChartsPage: React.FC = () => {
  return (
    <div>
      <h1 className="text-2xl font-bold mb-4"><T>Real-time Charts</T></h1>
      <RealTimeChart />
    </div>
  );
};

export default ChartsPage; 