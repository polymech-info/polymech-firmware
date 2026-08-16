import React from 'react';
import { useOutletContext } from 'react-router-dom';
import { GenericCanvas } from '@/components/hmi/GenericCanvas';

interface OutletContext {
  isEditMode: boolean;
}

const PlaygroundPage: React.FC = () => {
  const { isEditMode = false } = useOutletContext<OutletContext>();

  return (
    <GenericCanvas
      pageId="playground-layout"
      pageName="HMI Playground"
      isEditMode={isEditMode}
      showControls={isEditMode}
    />
  );
};

export default PlaygroundPage;
