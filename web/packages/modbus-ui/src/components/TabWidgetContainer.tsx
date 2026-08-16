import React from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import TabWidget from './TabWidget';
import TabContent from './TabContent';

interface TabConfig {
  id: string;
  label: string;
  route: string;
}

interface TabWidgetContainerProps {
  widgetId?: string;
  initialTabs?: TabConfig[];
  maxTabs?: number;
  defaultColumns?: number;
  showTitle?: boolean;
  className?: string;
  isEditMode?: boolean;
}

const TabWidgetContainer: React.FC<TabWidgetContainerProps> = ({
  widgetId = 'tab-widget',
  initialTabs = [{ id: 'tab1', label: 'Tab 1', route: 'tab1' }],
  maxTabs = 10,
  defaultColumns = 2,
  showTitle = false,
  className = '',
  isEditMode = false,
}) => {
  const [tabs, setTabs] = React.useState<TabConfig[]>(initialTabs);

  const handleTabsChange = (newTabs: TabConfig[]) => {
    setTabs(newTabs);
  };

  return (
    <div className={className}>
      <Routes>
        <Route
          path="*"
          element={
            <TabWidget
              widgetId={widgetId}
              initialTabs={initialTabs}
              onTabsChange={handleTabsChange}
              maxTabs={maxTabs}
              className={className}
              isEditMode={isEditMode}
            />
          }
        >
          {/* Generate routes for each tab */}
          {tabs.map(tab => (
            <Route
              key={tab.id}
              path={tab.route}
              element={
                <TabContent
                  tabId={tab.id}
                  widgetId={widgetId}
                  defaultColumns={defaultColumns}
                  showTitle={showTitle}
                />
              }
            />
          ))}
          {/* Default redirect to first tab */}
          <Route 
            index 
            element={<Navigate to={tabs[0]?.route || 'tab1'} replace />} 
          />
        </Route>
      </Routes>
    </div>
  );
};

export default TabWidgetContainer;
