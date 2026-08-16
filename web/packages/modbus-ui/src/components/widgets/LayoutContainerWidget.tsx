import React, { useEffect, useMemo } from 'react';
import { GenericCanvas } from '@/components/hmi/GenericCanvas';

interface LayoutContainerWidgetProps {
  widgetInstanceId: string;
  onPropsChange: (props: Record<string, any>) => void;
  isEditMode?: boolean;
  // Props from widget settings
  nestedPageId?: string;
  nestedPageName?: string;
  showControls?: boolean;
}

const LayoutContainerWidget: React.FC<LayoutContainerWidgetProps> = ({
  widgetInstanceId,
  onPropsChange,
  isEditMode = false,
  nestedPageId,
  nestedPageName = 'Nested Canvas',
  showControls = false,
}) => {
  // Generate a unique pageId for the nested canvas if it doesn't exist.
  const uniqueNestedPageId = useMemo(() => {
    if (nestedPageId) {
      return nestedPageId;
    }
    const newId = `nested-layout-${widgetInstanceId}`;
    return newId;
  }, [nestedPageId, widgetInstanceId]);

  // If a new ID was generated, save it back to the widget's props.
  useEffect(() => {
    if (!nestedPageId) {
      setTimeout(() => {
        onPropsChange({ nestedPageId: uniqueNestedPageId });
      }, 0);
    }
  }, [nestedPageId, uniqueNestedPageId, onPropsChange]);

  if (!nestedPageId) {
    return <div className="p-4 text-center text-slate-500">Initializing nested layout...</div>;
  }

  return (
    <GenericCanvas
      pageId={uniqueNestedPageId}
      pageName={nestedPageName}
      isEditMode={isEditMode}
      showControls={showControls}
      className="p-0"
    />
  );
};

export default LayoutContainerWidget;
