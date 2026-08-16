import React, { useState, useEffect } from 'react';
import { Plus, X } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { cn } from '@/lib/utils';
import { useLayout } from '@/contexts/LayoutContext';
import { LayoutContainer } from './hmi/LayoutContainer';
import { WidgetPalette } from './hmi/WidgetPalette';

interface TabConfig {
  id: string;
  label: string;
}

interface SimpleTabWidgetProps {
  widgetId?: string;
  initialTabs?: TabConfig[];
  maxTabs?: number;
  defaultColumns?: number;
  showTitle?: boolean;
  className?: string;
  enableEditMode?: boolean;
}

const SimpleTabWidget: React.FC<SimpleTabWidgetProps> = ({
  widgetId = 'simple-tab-widget',
  initialTabs = [{ id: 'tab1', label: 'Tab 1' }],
  maxTabs = 10,
  defaultColumns = 2,
  showTitle = false,
  className = '',
  enableEditMode = false,
}) => {
  const [tabs, setTabs] = useState<TabConfig[]>(initialTabs);
  const [activeTabId, setActiveTabId] = useState<string>(initialTabs[0]?.id || 'tab1');
  const [editingTab, setEditingTab] = useState<string | null>(null);
  const [editLabel, setEditLabel] = useState('');
  const [forceRefresh, setForceRefresh] = useState(0);

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

  // Widget palette state
  const [showWidgetPalette, setShowWidgetPalette] = useState(false);
  const [targetContainerId, setTargetContainerId] = useState<string | null>(null);
  const [targetColumn, setTargetColumn] = useState<number | undefined>(undefined);

  // Load tabs from localStorage on mount
  useEffect(() => {
    const savedTabs = localStorage.getItem(`${widgetId}-tabs`);
    const savedActiveTab = localStorage.getItem(`${widgetId}-active-tab`);
    
    if (savedTabs) {
      try {
        const parsedTabs = JSON.parse(savedTabs);
        setTabs(parsedTabs);
        
        if (savedActiveTab && parsedTabs.some((tab: TabConfig) => tab.id === savedActiveTab)) {
          setActiveTabId(savedActiveTab);
        } else if (parsedTabs.length > 0) {
          setActiveTabId(parsedTabs[0].id);
        }
      } catch (error) {
        console.error('Failed to load saved tabs:', error);
      }
    }
  }, [widgetId]);

  // Save tabs to localStorage when changed
  useEffect(() => {
    localStorage.setItem(`${widgetId}-tabs`, JSON.stringify(tabs));
  }, [tabs, widgetId]);

  // Save active tab to localStorage when changed
  useEffect(() => {
    localStorage.setItem(`${widgetId}-active-tab`, activeTabId);
  }, [activeTabId, widgetId]);

  // Load layout for each tab
  useEffect(() => {
    const loadTabLayouts = async () => {
      for (const tab of tabs) {
        const pageId = `${widgetId}-${tab.id}`;
        try {
          await loadPageLayout(pageId, `Tab ${tab.label}`);
        } catch (error) {
          console.error(`Failed to load layout for tab ${tab.id}:`, error);
        }
      }
    };
    
    loadTabLayouts();
  }, [tabs, widgetId, loadPageLayout]);

  const addTab = () => {
    if (tabs.length >= maxTabs) return;
    
    const newTabId = `tab${Date.now()}`;
    const newTab: TabConfig = {
      id: newTabId,
      label: `Tab ${tabs.length + 1}`,
    };
    
    setTabs(prev => [...prev, newTab]);
    setActiveTabId(newTabId);
  };

  const removeTab = (tabId: string) => {
    if (tabs.length <= 1) return; // Keep at least one tab
    
    const newTabs = tabs.filter(tab => tab.id !== tabId);
    setTabs(newTabs);
    
    // If we're removing the active tab, switch to first remaining tab
    if (activeTabId === tabId) {
      setActiveTabId(newTabs[0].id);
    }
  };

  const startEditingTab = (tabId: string) => {
    const tab = tabs.find(t => t.id === tabId);
    if (tab) {
      setEditingTab(tabId);
      setEditLabel(tab.label);
    }
  };

  const saveTabLabel = () => {
    if (editingTab && editLabel.trim()) {
      setTabs(prev => prev.map(tab => 
        tab.id === editingTab 
          ? { ...tab, label: editLabel.trim() }
          : tab
      ));
    }
    setEditingTab(null);
    setEditLabel('');
  };

  const cancelEditingTab = () => {
    setEditingTab(null);
    setEditLabel('');
  };

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      saveTabLabel();
    } else if (e.key === 'Escape') {
      cancelEditingTab();
    }
  };

  // Handle widget addition
  const handleAddWidget = (containerId: string, columnIndex?: number) => {
    setTargetContainerId(containerId);
    setTargetColumn(columnIndex);
    setShowWidgetPalette(true);
  };

  const handleWidgetAdd = async (selectedWidgetId: string) => {
    if (targetContainerId) {
      try {
        const pageId = `${widgetId}-${activeTabId}`;
        
        // Ensure the layout is loaded before adding widget
        const currentLayout = getLoadedPageLayout(pageId);
        if (!currentLayout) {
          await loadPageLayout(pageId, `Tab ${tabs.find(t => t.id === activeTabId)?.label || activeTabId}`);
        }
        
        await addWidgetToPage(pageId, targetContainerId, selectedWidgetId, targetColumn);
        
        // Force a small delay to ensure layout updates, then trigger re-render
        await new Promise(resolve => setTimeout(resolve, 100));
        setForceRefresh(prev => prev + 1);
      } catch (error) {
        console.error('Failed to add widget:', error);
      }
    }
    setShowWidgetPalette(false);
    setTargetContainerId(null);
    setTargetColumn(undefined);
  };

  // Get current tab's layout
  const currentPageId = `${widgetId}-${activeTabId}`;
  const layout = getLoadedPageLayout(currentPageId);

  if (isLoading || !layout) {
    return (
      <div className={`flex items-center justify-center min-h-[200px] ${className}`}>
        <div className="text-center">
          <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-blue-500 mx-auto mb-2"></div>
          <p className="text-slate-500 dark:text-slate-400 text-sm">Loading tabs...</p>
        </div>
      </div>
    );
  }

  // Ensure current tab has at least one container
  if (layout.containers.length === 0) {
    addPageContainer(currentPageId);
    return null;
  }

  const mainContainer = layout.containers[0];

  return (
    <div className={cn('space-y-4', className)}>
      {/* Tab Navigation */}
      <div className="flex items-center gap-2 border-b border-border pb-2">
        <div className="flex flex-wrap items-center gap-2 flex-1">
          {tabs.map(tab => (
            <div key={tab.id} className="flex items-center group">
              {editingTab === tab.id ? (
                <div className="flex items-center gap-1">
                  <Input
                    value={editLabel}
                    onChange={(e) => setEditLabel(e.target.value)}
                    onBlur={saveTabLabel}
                    onKeyDown={handleKeyPress}
                    className="h-8 w-24 text-sm"
                    autoFocus
                  />
                </div>
              ) : (
                <button
                  onClick={() => setActiveTabId(tab.id)}
                  className={cn(
                    "flex items-center gap-2 rounded-md px-3 py-2 text-sm font-medium transition-colors relative",
                    activeTabId === tab.id
                      ? "bg-primary text-primary-foreground" 
                      : "text-muted-foreground hover:bg-muted/50 hover:text-foreground"
                  )}
                >
                  <span 
                    onDoubleClick={enableEditMode ? () => startEditingTab(tab.id) : undefined}
                    className={enableEditMode ? "cursor-pointer" : ""}
                  >
                    {tab.label}
                  </span>
                  {enableEditMode && tabs.length > 1 && (
                    <Button
                      variant="ghost"
                      size="sm"
                      className="h-4 w-4 p-0 opacity-0 group-hover:opacity-100 hover:bg-destructive hover:text-destructive-foreground ml-1"
                      onClick={(e) => {
                        e.preventDefault();
                        e.stopPropagation();
                        removeTab(tab.id);
                      }}
                    >
                      <X className="h-3 w-3" />
                    </Button>
                  )}
                </button>
              )}
            </div>
          ))}
        </div>
        
        {/* Add Tab Button - only show in edit mode */}
        {enableEditMode && tabs.length < maxTabs && (
          <Button
            variant="outline"
            size="sm"
            onClick={addTab}
            className="h-8 w-8 p-0"
            title="Add Tab"
          >
            <Plus className="h-4 w-4" />
          </Button>
        )}
      </div>

      {/* Tab Content */}
      <div className="min-h-[200px]">
        <LayoutContainer
          key={`${mainContainer.id}-${activeTabId}-${forceRefresh}`}
          container={mainContainer}
          isEditMode={enableEditMode}
          pageId={currentPageId}
          onAddWidget={handleAddWidget}
          onRemoveWidget={async (widgetInstanceId) => {
            await removeWidgetFromPage(currentPageId, widgetInstanceId);
            setForceRefresh(prev => prev + 1);
          }}
          onMoveWidget={(widgetInstanceId, direction) => {
            moveWidgetInPage(currentPageId, widgetInstanceId, direction);
          }}
          onUpdateColumns={(containerId, columns) => {
            updatePageContainerColumns(currentPageId, containerId, columns);
          }}
          onAddContainer={(parentContainerId) => {
            addPageContainer(currentPageId, parentContainerId);
          }}
          onRemoveContainer={(containerId) => {
            if (layout.containers.length > 1) {
              removePageContainer(currentPageId, containerId);
            }
          }}
        />
      </div>
      
      {/* Widget Palette */}
      <WidgetPalette
        isVisible={showWidgetPalette}
        onClose={() => setShowWidgetPalette(false)}
        onWidgetAdd={handleWidgetAdd}
      />
    </div>
  );
};

export default SimpleTabWidget;
