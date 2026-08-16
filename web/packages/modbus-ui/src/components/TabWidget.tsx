import React, { useState, useEffect } from 'react';
import { NavLink, Outlet, useLocation, useNavigate } from 'react-router-dom';
import { Plus, X } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { cn } from '@/lib/utils';

interface TabConfig {
  id: string;
  label: string;
  route: string;
}

interface TabWidgetProps {
  widgetId?: string;
  basePath?: string;
  initialTabs?: TabConfig[];
  onTabsChange?: (tabs: TabConfig[]) => void;
  maxTabs?: number;
  className?: string;
  isEditMode?: boolean;
}

const TabWidget: React.FC<TabWidgetProps> = ({
  widgetId = 'tab-widget',
  basePath = '',
  initialTabs = [{ id: 'tab1', label: 'Tab 1', route: 'tab1' }],
  onTabsChange,
  maxTabs = 10,
  className = '',
  isEditMode = false,
}) => {
  const location = useLocation();
  const navigate = useNavigate();
  
  const [tabs, setTabs] = useState<TabConfig[]>(initialTabs);
  const [editingTab, setEditingTab] = useState<string | null>(null);
  const [editLabel, setEditLabel] = useState('');

  // Load tabs from localStorage on mount
  useEffect(() => {
    const savedTabs = localStorage.getItem(`${widgetId}-tabs`);
    if (savedTabs) {
      try {
        const parsedTabs = JSON.parse(savedTabs);
        setTabs(parsedTabs);
      } catch (error) {
        console.error('Failed to load saved tabs:', error);
      }
    }
  }, [widgetId]);

  // Save tabs to localStorage when changed
  useEffect(() => {
    localStorage.setItem(`${widgetId}-tabs`, JSON.stringify(tabs));
    onTabsChange?.(tabs);
  }, [tabs, widgetId, onTabsChange]);

  // Track if component has mounted to avoid initial navigation
  const [hasMounted, setHasMounted] = useState(false);

  useEffect(() => {
    const timer = setTimeout(() => setHasMounted(true), 1000); // Delay to avoid disrupting edit mode
    return () => clearTimeout(timer);
  }, []);

  // Navigate to first tab if on base path (but only after initial mount delay)
  useEffect(() => {
    if (!hasMounted || isEditMode) return; // Don't navigate when not mounted or in edit mode
    
    const currentPath = location.pathname;
    const expectedBasePath = basePath || location.pathname.split('/').slice(0, -1).join('/');
    
    if (currentPath === expectedBasePath && tabs.length > 0) {
      const lastActiveTab = localStorage.getItem(`${widgetId}-last-tab`) || tabs[0].route;
      const tabExists = tabs.some(tab => tab.route === lastActiveTab);
              const targetTab = tabExists ? lastActiveTab : tabs[0].route;
        navigate(targetTab, { replace: true, relative: "path" });
    }
  }, [hasMounted, location.pathname, navigate, tabs, basePath, widgetId, isEditMode]);

  const handleTabClick = (tabRoute: string) => {
    localStorage.setItem(`${widgetId}-last-tab`, tabRoute);
  };

  const addTab = () => {
    if (tabs.length >= maxTabs) return;
    
    const newTabId = `tab${Date.now()}`;
    const newTab: TabConfig = {
      id: newTabId,
      label: `Tab ${tabs.length + 1}`,
      route: newTabId,
    };
    
    setTabs(prev => [...prev, newTab]);
    
    // Navigate to new tab (use relative navigation)
    setTimeout(() => {
      navigate(newTab.route, { relative: "path" });
    }, 0);
  };

  const removeTab = (tabId: string) => {
    if (tabs.length <= 1) return; // Keep at least one tab
    
    const tabToRemove = tabs.find(tab => tab.id === tabId);
    const newTabs = tabs.filter(tab => tab.id !== tabId);
    setTabs(newTabs);
    
    // If we're removing the current tab, navigate to first remaining tab
    if (tabToRemove && location.pathname.endsWith(tabToRemove.route)) {
      navigate(newTabs[0].route, { replace: true, relative: "path" });
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

  const getCurrentTabRoute = () => {
    const pathSegments = location.pathname.split('/');
    return pathSegments[pathSegments.length - 1];
  };

  const currentTabRoute = getCurrentTabRoute();

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
                <NavLink
                  to={tab.route}
                  onClick={() => handleTabClick(tab.route)}
                  className={({ isActive }) => cn(
                    "flex items-center gap-2 rounded-md px-3 py-2 text-sm font-medium transition-colors relative",
                    isActive || currentTabRoute === tab.route
                      ? "bg-primary text-primary-foreground" 
                      : "text-muted-foreground hover:bg-muted/50 hover:text-foreground"
                  )}
                >
                  <span 
                    onDoubleClick={() => startEditingTab(tab.id)}
                    className="cursor-pointer"
                  >
                    {tab.label}
                  </span>
                  {tabs.length > 1 && (
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
                </NavLink>
              )}
            </div>
          ))}
        </div>
        
        {/* Controls */}
        <div className="flex items-center gap-2">
          {tabs.length < maxTabs && (
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
      </div>

      {/* Tab Content */}
      <div className="min-h-[200px]">
        <Outlet context={{ isEditMode, currentTab: currentTabRoute }} />
      </div>
    </div>
  );
};

export default TabWidget;
