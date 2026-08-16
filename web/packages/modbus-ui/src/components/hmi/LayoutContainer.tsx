import React, { useState } from 'react';
import { cn } from '@/lib/utils';
import { Button } from '@/components/ui/button';
import { Plus, Minus, Grid3X3, Trash2, Settings, ArrowUp, ArrowDown } from 'lucide-react';
import { LayoutContainer as LayoutContainerType, WidgetInstance } from '@/lib/unifiedLayoutManager';
import { widgetRegistry } from '@/lib/widgetRegistry';
import { WidgetSettingsManager } from '@/components/widgets/WidgetSettingsManager';
import { WidgetMovementControls } from '@/components/widgets/WidgetMovementControls';
import { useLayout } from '@/contexts/LayoutContext';
import CollapsibleSection from '@/components/CollapsibleSection';
import { ContainerSettingsManager } from '@/components/containers/ContainerSettingsManager';

interface LayoutContainerProps {
  container: LayoutContainerType;
  isEditMode: boolean;
  pageId: string;
  selectedContainerId?: string | null;
  onSelect?: (containerId: string) => void;
  onAddWidget?: (containerId: string, targetColumn?: number) => void;
  onRemoveWidget?: (widgetInstanceId: string) => void;
  onMoveWidget?: (widgetInstanceId: string, direction: 'up' | 'down' | 'left' | 'right') => void;
  onUpdateColumns?: (containerId: string, columns: number) => void;
  onUpdateSettings?: (containerId: string, settings: Partial<LayoutContainerType['settings']>) => void;
  onAddContainer?: (parentContainerId: string) => void;
  onRemoveContainer?: (containerId: string) => void;
  onMoveContainer?: (containerId: string, direction: 'up' | 'down') => void;
  canMoveContainerUp?: boolean;
  canMoveContainerDown?: boolean;
  depth?: number;
  isCompactMode?: boolean;
}

const LayoutContainerComponent: React.FC<LayoutContainerProps> = ({
  container,
  isEditMode,
  pageId,
  selectedContainerId,
  onSelect,
  onAddWidget,
  onRemoveWidget,
  onMoveWidget,
  onUpdateColumns,
  onUpdateSettings,
  onAddContainer,
  onRemoveContainer,
  onMoveContainer,
  canMoveContainerUp,
  canMoveContainerDown,
  depth = 0,
  isCompactMode = false,
}) => {
  const maxDepth = 3; // Limit nesting depth
  const canNest = depth < maxDepth;
  const isSelected = selectedContainerId === container.id;
  const [showContainerSettings, setShowContainerSettings] = useState(false);

  // Generate responsive grid classes based on container.columns
  const getGridClasses = (columns: number) => {
    const baseClass = "grid gap-4"; // Always grid with gap
    
    // Mobile: always 1 column, Desktop: respect container.columns
    switch (columns) {
      case 1: return `${baseClass} grid-cols-1`;
      case 2: return `${baseClass} grid-cols-1 md:grid-cols-2`;
      case 3: return `${baseClass} grid-cols-1 md:grid-cols-2 lg:grid-cols-3`;
      case 4: return `${baseClass} grid-cols-1 md:grid-cols-2 lg:grid-cols-4`;
      case 5: return `${baseClass} grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-5`;
      case 6: return `${baseClass} grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-6`;
      default: 
        // For 7+ columns, use a more conservative approach
        return `${baseClass} grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 2xl:grid-cols-${Math.min(columns, 12)}`;
    }
  };

  const gridClasses = getGridClasses(container.columns);

  // Extract container content rendering logic
  const renderContainerContent = () => (
    <>
      {/* Grid Column Indicators (only in edit mode when selected) */}
      {isEditMode && isSelected && container.widgets.length === 0 && container.children.length === 0 && (
        <>
          {Array.from({ length: container.columns }, (_, i) => (
            <div
              key={i}
              className="border-2 border-dashed border-blue-300 dark:border-blue-600 rounded-lg min-h-[80px] flex items-center justify-center text-blue-500 dark:text-blue-400 text-sm cursor-pointer hover:bg-blue-100/20 dark:hover:bg-blue-800/20 transition-colors"
              onDoubleClick={(e) => {
                e.stopPropagation();
                onAddWidget?.(container.id, i);
              }}
              title={`Double-click to add widget to column ${i + 1}`}
            >
              Col {i + 1}
            </div>
          ))}
        </>
      )}
      
      {/* Render Widgets */}
      {container.widgets
        .sort((a, b) => (a.order || 0) - (b.order || 0))
        .map((widget, index) => (
          <WidgetItem
            key={widget.id}
            widget={widget}
            isEditMode={isEditMode}
            pageId={pageId}
            canMoveUp={index > 0}
            canMoveDown={index < container.widgets.length - 1}
            onRemove={onRemoveWidget}
            onMove={onMoveWidget}
          />
        ))}

      {/* Add Widget Buttons - one per column for non-empty containers (in edit mode) */}
      {isEditMode && container.widgets.length > 0 && container.children.length === 0 && (
        <>
          {Array.from({ length: container.columns }, (_, colIndex) => (
            <div 
              key={`add-widget-${colIndex}`}
              className="flex items-center justify-center min-h-[60px] border-2 border-dashed border-slate-300 dark:border-slate-600 rounded-lg hover:border-blue-400 dark:hover:border-blue-500 transition-colors cursor-pointer group"
              onClick={(e) => {
                e.stopPropagation();
                onAddWidget?.(container.id, colIndex);
              }}
              onDoubleClick={(e) => {
                e.stopPropagation();
                onAddWidget?.(container.id, colIndex);
              }}
              title={`Click to add widget to column ${colIndex + 1}`}
            >
              <div className="text-center text-slate-500 dark:text-slate-400 group-hover:text-blue-500 dark:group-hover:text-blue-400 transition-colors">
                <Plus className="h-5 w-5 mx-auto mb-1" />
                <p className="text-xs">Add Widget</p>
                <p className="text-xs opacity-60">Col {colIndex + 1}</p>
              </div>
            </div>
          ))}
        </>
      )}

      {/* Render Nested Containers */}
      {container.children
        .sort((a, b) => (a.order || 0) - (b.order || 0))
        .map((childContainer) => (
          <div key={childContainer.id} className="col-span-full">
            <LayoutContainer
              container={childContainer}
              isEditMode={isEditMode}
              pageId={pageId}
              selectedContainerId={selectedContainerId}
              onSelect={onSelect}
              onAddWidget={onAddWidget}
              onRemoveWidget={onRemoveWidget}
              onMoveWidget={onMoveWidget}
              onUpdateColumns={onUpdateColumns}
              onUpdateSettings={onUpdateSettings}
              onAddContainer={onAddContainer}
              onRemoveContainer={onRemoveContainer}
              onMoveContainer={onMoveContainer}
              canMoveContainerUp={canMoveContainerUp}
              canMoveContainerDown={canMoveContainerDown}
              depth={depth + 1}
              isCompactMode={isCompactMode}
            />
          </div>
        ))}

      {/* Empty State - only show when not showing column indicators */}
      {container.widgets.length === 0 && container.children.length === 0 && !(isEditMode && isSelected) && (
        <div 
          className={cn(
            "col-span-full flex items-center justify-center min-h-[80px] text-slate-500 dark:text-slate-400",
            isEditMode && "cursor-pointer hover:bg-slate-100/20 dark:hover:bg-slate-800/20 transition-colors"
          )}
          onDoubleClick={isEditMode ? (e) => {
            e.stopPropagation();
            onSelect?.(container.id);
            setTimeout(() => onAddWidget?.(container.id), 100); // Small delay to ensure selection happens first, no column = append
          } : undefined}
          title={isEditMode ? "Double-click to add widget" : undefined}
        >
          {isEditMode ? (
            <div className="text-center">
              <Plus className="h-8 w-8 mx-auto mb-2 opacity-50" />
              <p className="text-sm">Double-click to add widgets</p>
            </div>
          ) : (
            <p className="text-sm"></p>
          )}
        </div>
      )}
    </>
  );

  return (
    <div className="space-y-0">
      {/* Edit Mode Controls */}
      {isEditMode && (
        <div className={cn(
          "text-white px-2 sm:px-3 py-1 rounded-t-lg text-xs overflow-hidden",
          isSelected ? "bg-blue-500" : "bg-slate-500"
        )}>
          {/* Responsive layout: title and buttons wrap on small screens */}
          <div className="flex items-center justify-between gap-x-2 gap-y-1 min-w-0 flex-wrap">
            <div className="flex items-center gap-1 min-w-0 flex-grow">
              <Grid3X3 className="h-3 w-3 shrink-0" />
              <span className="truncate text-xs font-semibold">
                {container.settings?.showTitle && container.settings?.title 
                  ? container.settings.title 
                  : `Container (${container.columns} col${container.columns !== 1 ? 's' : ''})`}
                {(container.settings?.collapsible || container.settings?.showTitle) && (
                  <span className="ml-1 opacity-75 font-normal">⚙️</span>
                )}
              </span>
            </div>
            
            {/* Minimalist button row - wraps and justifies to the end */}
            <div className="flex items-center gap-0.5 flex-wrap justify-end">
              {/* Move controls for root containers */}
              {depth === 0 && (
                <div className="flex items-center gap-0.5 mr-1 border-r border-white/20 pr-1">
                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={(e) => {
                      e.stopPropagation();
                      onMoveContainer?.(container.id, 'up');
                    }}
                    disabled={!canMoveContainerUp}
                    className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                    title="Move container up"
                  >
                    <ArrowUp className="h-3 w-3" />
                  </Button>
                  <Button
                    variant="ghost"
                    size="sm"
                    onClick={(e) => {
                      e.stopPropagation();
                      onMoveContainer?.(container.id, 'down');
                    }}
                    disabled={!canMoveContainerDown}
                    className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                    title="Move container down"
                  >
                    <ArrowDown className="h-3 w-3" />
                  </Button>
                </div>
              )}
              {/* Column controls */}
              <Button
                variant="ghost"
                size="sm"
                onClick={(e) => {
                  e.stopPropagation();
                  onUpdateColumns?.(container.id, container.columns - 1);
                }}
                disabled={container.columns <= 1}
                className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                title="Decrease columns"
              >
                <Minus className="h-3 w-3" />
              </Button>
              
              <span className="min-w-[12px] text-center text-xs font-medium px-1">{container.columns}</span>
              
              <Button
                variant="ghost"
                size="sm"
                onClick={(e) => {
                  e.stopPropagation();
                  onUpdateColumns?.(container.id, container.columns + 1);
                }}
                disabled={container.columns >= 12}
                className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                title="Increase columns"
              >
                <Plus className="h-3 w-3" />
              </Button>

              {/* Add widget button */}
              <Button
                variant="ghost"
                size="sm"
                onClick={(e) => {
                  e.stopPropagation();
                  onAddWidget?.(container.id);
                }}
                className="h-4 px-1 text-white hover:bg-white/20 shrink-0 ml-1"
                title="Add widget"
              >
                <Plus className="h-3 w-3" />
              </Button>

              {/* Add nested container button */}
              {canNest && (
                <Button
                  variant="ghost"
                  size="sm"
                  onClick={(e) => {
                    e.stopPropagation();
                    onAddContainer?.(container.id);
                  }}
                  className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                  title="Add nested container"
                >
                  <Grid3X3 className="h-3 w-3" />
                </Button>
              )}

              {/* Container settings button */}
              <Button
                variant="ghost"
                size="sm"
                onClick={(e) => {
                  e.stopPropagation();
                  setShowContainerSettings(true);
                }}
                className="h-4 px-1 text-white hover:bg-white/20 shrink-0"
                title="Container settings"
              >
                <Settings className="h-3 w-3" />
              </Button>

              {/* Remove container button */}
              <Button
                variant="ghost"
                size="sm"
                onClick={(e) => {
                  e.stopPropagation();
                  onRemoveContainer?.(container.id);
                }}
                className="h-4 px-1 text-white hover:bg-red-400 shrink-0"
                title="Remove container"
              >
                <Trash2 className="h-3 w-3" />
              </Button>
            </div>
          </div>
        </div>
      )}

      {/* Container Content */}
      <div
        className={cn(
          "relative border-2 border-dashed transition-all duration-200 min-w-0 overflow-hidden",
          isEditMode ? "rounded-b-lg" : "rounded-lg",
          isEditMode && "hover:border-blue-300 cursor-pointer",
          isSelected && isEditMode && "border-blue-500 bg-blue-50/20 dark:bg-blue-900/20",
          !isSelected && isEditMode && "border-slate-300/50 dark:border-white/20",
          !isEditMode && "border-transparent"
        )}
        onClick={(e) => {
          e.stopPropagation();
          if (isEditMode) {
            onSelect?.(container.id);
          }
        }}
      >
      {container.settings?.collapsible ? (
        <CollapsibleSection
          title={
            container.settings?.showTitle 
              ? (container.settings?.title || `Container (${container.columns} col${container.columns !== 1 ? 's' : ''})`)
              : `Container (${container.columns} col${container.columns !== 1 ? 's' : ''})`
          }
          initiallyOpen={!container.settings?.collapsed}
          storageKey={`container-${container.id}-collapsed`}
          className="border-0 rounded-none shadow-none bg-transparent"
          minimal={true}
        >
          <div
            className={cn(
              isCompactMode ? "p-1 sm:p-2 min-h-[80px]" : "p-4 min-h-[120px]",
              gridClasses,
              isEditMode && isSelected && "bg-blue-50/10 dark:bg-blue-900/10"
            )}
            style={{
              gap: `${container.gap}px`,
            }}
          >
            {renderContainerContent()}
          </div>
        </CollapsibleSection>
      ) : (
        <div>
          {/* Title for non-collapsible containers */}
          {container.settings?.showTitle && (
            <div className="px-4 pt-3 pb-1 border-b border-slate-300/30 dark:border-white/10">
              <h3 className="text-sm font-medium text-slate-700 dark:text-white">
                {container.settings?.title || `Container (${container.columns} col${container.columns !== 1 ? 's' : ''})`}
              </h3>
            </div>
          )}
          <div
            className={cn(
              isCompactMode ? "p-1 sm:p-2 min-h-[80px]" : "p-4 min-h-[120px]",
              gridClasses,
              isEditMode && isSelected && "bg-blue-50/10 dark:bg-blue-900/10"
            )}
            style={{
              gap: `${container.gap}px`,
            }}
          >
            {renderContainerContent()}
          </div>
        </div>
      )}
      </div>
      
      {/* Container Settings Dialog */}
      {showContainerSettings && (
        <ContainerSettingsManager
          isOpen={showContainerSettings}
          onClose={() => setShowContainerSettings(false)}
          onSave={(settings) => {
            onUpdateSettings?.(container.id, settings);
            setShowContainerSettings(false);
          }}
          currentSettings={container.settings}
          containerInfo={{
            id: container.id,
            columns: container.columns,
          }}
        />
      )}
    </div>
  );
};

interface WidgetItemProps {
  widget: WidgetInstance;
  isEditMode: boolean;
  pageId: string;
  canMoveUp: boolean;
  canMoveDown: boolean;
  onRemove?: (widgetInstanceId: string) => void;
  onMove?: (widgetInstanceId: string, direction: 'up' | 'down' | 'left' | 'right') => void;
}

const WidgetItem: React.FC<WidgetItemProps> = ({
  widget,
  isEditMode,
  pageId,
  canMoveUp,
  canMoveDown,
  onRemove,
  onMove,
}) => {
  const widgetDefinition = widgetRegistry.get(widget.widgetId);
  const { updateWidgetProps } = useLayout();
  const [showSettingsModal, setShowSettingsModal] = useState(false);

  // pageId is now passed as a prop from the parent component

  if (!widgetDefinition) {
    return (
      <div className="relative group border-2 border-red-500 bg-red-100 dark:bg-red-900/20 p-4 rounded-lg">
        <p className="text-red-600 dark:text-red-400 text-sm">
          Widget "{widget.widgetId}" not found in registry
        </p>
        {isEditMode && (
          <Button
            size="icon"
            variant="destructive"
            className="absolute top-2 right-2 h-6 w-6 opacity-0 group-hover:opacity-100 transition-opacity"
            onClick={(e) => {
              e.stopPropagation();
              onRemove?.(widget.id);
            }}
            title="Remove invalid widget"
          >
            <Trash2 className="h-3 w-3" />
          </Button>
        )}
      </div>
    );
  }

  const WidgetComponent = widgetDefinition.component;

  const handleSettingsSave = async (settings: Record<string, any>) => {
    try {
      await updateWidgetProps(pageId, widget.id, settings);
    } catch (error) {
      console.error('Failed to save widget settings:', error);
    }
  };

  return (
    <div className="relative group">
      {/* Edit Mode Controls */}
      {isEditMode && (
        <>
          {/* Widget Info Overlay */}
          <div className="absolute top-0 left-0 right-0 bg-green-500/90 text-white text-xs px-2 py-1 rounded-t-lg z-10">
            <div className="flex items-center justify-between">
              <span>{widgetDefinition.metadata.name}</span>
              <div className="flex items-center gap-1">
                {/* Settings Gear Icon - For widgets with configSchema */}
                {widgetDefinition.metadata.configSchema && (
                  <Button
                    size="icon"
                    variant="ghost"
                    className="h-4 w-4 text-white hover:bg-white/20"
                    onClick={(e) => {
                      e.stopPropagation();
                      setShowSettingsModal(true);
                    }}
                    title="Widget settings"
                  >
                    <Settings className="h-2 w-2" />
                  </Button>
                )}
                
                {/* Remove Button */}
                <Button
                  size="icon"
                  variant="ghost"
                  className="h-4 w-4 text-white hover:bg-white/20"
                  onClick={(e) => {
                    e.stopPropagation();
                    onRemove?.(widget.id);
                  }}
                  title="Remove widget"
                >
                  <Trash2 className="h-2 w-2" />
                </Button>
              </div>
            </div>
          </div>

          {/* Move Controls - Cross Pattern */}
          <WidgetMovementControls
            className="absolute top-8 left-2 z-10"
            onMove={(direction) => onMove?.(widget.id, direction)}
            canMoveUp={canMoveUp}
            canMoveDown={canMoveDown}
          />

        </>
      )}

      {/* Widget Content - Always 100% width */}
      <div className="w-full bg-white dark:bg-slate-800 rounded-lg border border-slate-200 dark:border-slate-700 overflow-hidden">
        <WidgetComponent 
          {...(widget.props || {})}
          widgetInstanceId={widget.id}
          isEditMode={isEditMode}
          onPropsChange={async (newProps: Record<string, any>) => {
            try {
              await updateWidgetProps(pageId, widget.id, newProps);
            } catch (error) {
              console.error('Failed to update widget props:', error);
            }
          }}
        />
      </div>

      {/* Generic Settings Modal */}
      {widgetDefinition.metadata.configSchema && showSettingsModal && (
        <WidgetSettingsManager
          isOpen={showSettingsModal}
          onClose={() => setShowSettingsModal(false)}
          widgetDefinition={widgetDefinition}
          currentProps={widget.props || {}}
          onSave={handleSettingsSave}
        />
      )}
    </div>
  );
};

// Conservative comparison function for React.memo
const areLayoutContainerPropsEqual = (
  prevProps: LayoutContainerProps,
  nextProps: LayoutContainerProps
): boolean => {
  // Conservative approach: only prevent re-render for truly identical props
  // Since we're using deep cloning in LayoutContext, object references will change
  // when the layout data actually changes, so we can be more conservative here
  
  return (
    prevProps.isEditMode === nextProps.isEditMode &&
    prevProps.selectedContainerId === nextProps.selectedContainerId &&
    prevProps.depth === nextProps.depth &&
    prevProps.container === nextProps.container // Reference equality check
  );
};

// Export memoized component with conservative comparison
export const LayoutContainer = React.memo(LayoutContainerComponent, areLayoutContainerPropsEqual);