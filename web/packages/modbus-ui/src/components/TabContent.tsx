import React, { useEffect, useState } from 'react';
import { useOutletContext } from 'react-router-dom';
import { useLayout } from '@/contexts/LayoutContext';
import { LayoutContainer } from './hmi/LayoutContainer';
import { WidgetPalette } from './hmi/WidgetPalette';

interface TabContentContext {
  isEditMode?: boolean;
  currentTab?: string;
}

interface TabContentProps {
  tabId: string;
  widgetId?: string;
  defaultColumns?: number;
  showTitle?: boolean;
  className?: string;
}

const TabContent: React.FC<TabContentProps> = ({
  tabId,
  widgetId = 'tab-widget',
  defaultColumns = 2,
  showTitle = false,
  className = '',
}) => {
  const context = useOutletContext<TabContentContext>();
  const { isEditMode = false, currentTab } = context || {};
  
  const {
    loadPageLayout,
    getLoadedPageLayout,
    addWidgetToPage,
    removeWidgetFromPage,
    moveWidgetInPage,
    updatePageContainerColumns,
    addPageContainer,
    removePageContainer,
    isLoading,
  } = useLayout();

  // Create unique page ID for this tab
  const pageId = `${widgetId}-${tabId}`;
  const pageName = `Tab ${tabId}`;

  const layout = getLoadedPageLayout(pageId);
  
  // Widget palette state
  const [showWidgetPalette, setShowWidgetPalette] = useState(false);
  const [targetContainerId, setTargetContainerId] = useState<string | null>(null);
  const [targetColumn, setTargetColumn] = useState<number | undefined>(undefined);

  // Load the tab layout on mount or when tab changes
  useEffect(() => {
    if (currentTab === tabId || !currentTab) {
      loadPageLayout(pageId, pageName);
    }
  }, [pageId, pageName, loadPageLayout, currentTab, tabId]);

  // Don't render if this isn't the current tab (for performance)
  if (currentTab && currentTab !== tabId) {
    return null;
  }

  if (isLoading || !layout) {
    return (
      <div className={`flex items-center justify-center min-h-[200px] ${className}`}>
        <div className="text-center">
          <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-blue-500 mx-auto mb-2"></div>
          <p className="text-slate-500 dark:text-slate-400 text-sm">Loading tab content...</p>
        </div>
      </div>
    );
  }

  // Ensure we have at least one container
  if (layout.containers.length === 0) {
    // Add default container
    addPageContainer(pageId);
    return null; // Will re-render after container is added
  }

  const mainContainer = layout.containers[0];

  // Handle widget addition
  const handleAddWidget = (containerId: string, columnIndex?: number) => {
    setTargetContainerId(containerId);
    setTargetColumn(columnIndex);
    setShowWidgetPalette(true);
  };

  const handleWidgetAdd = async (widgetId: string) => {
    if (targetContainerId) {
      try {
        await addWidgetToPage(pageId, targetContainerId, widgetId, targetColumn);
      } catch (error) {
        console.error('Failed to add widget:', error);
      }
    }
    setShowWidgetPalette(false);
    setTargetContainerId(null);
    setTargetColumn(undefined);
  };

  return (
    <div className={className}>
      <LayoutContainer
        key={mainContainer.id}
        container={mainContainer}
        isEditMode={isEditMode}
        pageId={pageId}
        onAddWidget={handleAddWidget}
        onRemoveWidget={(widgetInstanceId) => {
          removeWidgetFromPage(pageId, widgetInstanceId);
        }}
        onMoveWidget={(widgetInstanceId, direction) => {
          moveWidgetInPage(pageId, widgetInstanceId, direction);
        }}
        onUpdateColumns={(containerId, columns) => {
          updatePageContainerColumns(pageId, containerId, columns);
        }}
        onAddContainer={(parentContainerId) => {
          addPageContainer(pageId, parentContainerId);
        }}
        onRemoveContainer={(containerId) => {
          if (layout.containers.length > 1) {
            removePageContainer(pageId, containerId);
          }
        }}
      />
      
      {/* Widget Palette */}
      <WidgetPalette
        isVisible={showWidgetPalette}
        onClose={() => setShowWidgetPalette(false)}
        onWidgetAdd={handleWidgetAdd}
      />
    </div>
  );
};

export default TabContent;
