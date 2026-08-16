import React, { useState, useEffect } from 'react';
import { T } from '@/i18n';
import { Button } from '@/components/ui/button';
import { Download, Upload, Grid3X3 } from 'lucide-react';
import { useLayout } from '@/contexts/LayoutContext';
import { LayoutContainer } from './LayoutContainer';
import { WidgetPalette } from './WidgetPalette';


interface GenericCanvasProps {
  pageId: string;
  pageName: string;
  isEditMode?: boolean;
  showControls?: boolean;
  className?: string;
}

const GenericCanvasComponent: React.FC<GenericCanvasProps> = ({
  pageId,
  pageName,
  isEditMode = false,
  showControls = true,
  className = '',
}) => {
  const { 
    loadedPages,
    loadPageLayout,
    addWidgetToPage,
    removeWidgetFromPage,
    moveWidgetInPage,
    updatePageContainerColumns,
    updatePageContainerSettings,
    addPageContainer,
    removePageContainer,
    movePageContainer,
    exportPageLayout, 
    importPageLayout,
    saveToApi,
    isLoading
  } = useLayout();
  const layout = loadedPages.get(pageId);

  // Load the page layout on mount
  useEffect(() => {
    if (!layout) {
      loadPageLayout(pageId, pageName);
    }
  }, [pageId, pageName, layout, loadPageLayout]);

  const [selectedContainer, setSelectedContainer] = useState<string | null>(null);
  const [showWidgetPalette, setShowWidgetPalette] = useState(false);
  const [targetContainerId, setTargetContainerId] = useState<string | null>(null);
  const [targetColumn, setTargetColumn] = useState<number | undefined>(undefined);
  const [isSaving, setIsSaving] = useState(false);
  const [saveStatus, setSaveStatus] = useState<'idle' | 'success' | 'error'>('idle');

  if (isLoading || !layout) {
    return (
      <div className={`flex items-center justify-center min-h-[400px] ${className}`}>
        <div className="text-center">
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-500 mx-auto mb-4"></div>
          <p className="text-slate-500 dark:text-slate-400">Loading {pageName.toLowerCase()}...</p>
        </div>
      </div>
    );
  }

  const handleSelectContainer = (containerId: string) => {
    setSelectedContainer(containerId);
  };

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

  const handleCanvasClick = () => {
    if (isEditMode) {
      setSelectedContainer(null);
    }
  };

  const handleExportLayout = async () => {
    try {
      const jsonData = await exportPageLayout(pageId);
      const blob = new Blob([jsonData], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `${layout.name.toLowerCase().replace(/\s+/g, '-')}-layout.json`;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    } catch (error) {
      console.error('Failed to export layout:', error);
    }
  };

  const handleImportLayout = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (file) {
        const reader = new FileReader();
        reader.onload = async (e) => {
          try {
            const jsonData = e.target?.result as string;
            await importPageLayout(pageId, jsonData);
            setSelectedContainer(null);
          } catch (error) {
            console.error('Failed to import layout:', error);
            alert('Failed to import layout. Please check the file format.');
          }
        };
        reader.readAsText(file);
      }
    };
    input.click();
  };

  const handleSaveToApi = async () => {
    if (isSaving) return;
    
    setIsSaving(true);
    setSaveStatus('idle');
    
    try {
      const success = await saveToApi();
      if (success) {
        setSaveStatus('success');
        setTimeout(() => setSaveStatus('idle'), 2000); // Clear success status after 2s
      } else {
        setSaveStatus('error');
        setTimeout(() => setSaveStatus('idle'), 3000); // Clear error status after 3s
      }
    } catch (error) {
      console.error('Failed to save to API:', error);
      setSaveStatus('error');
      setTimeout(() => setSaveStatus('idle'), 3000);
    } finally {
      setIsSaving(false);
    }
  };

  const totalWidgets = layout.containers.reduce((total, container) => {
    const getContainerWidgetCount = (cont: any): number => {
      let count = cont.widgets.length;
      cont.children.forEach((child: any) => {
        count += getContainerWidgetCount(child);
      });
      return count;
    };
    return total + getContainerWidgetCount(container);
  }, 0);

  return (
    <div className={`${className.includes('p-0') ? 'space-y-2' : 'space-y-6'} ${className}`}>
      
      {/* Header with Controls */}
      {showControls && (
        <div className="glass-card p-4">
          <div className="flex items-center justify-between">
            <div>
              <h2 className="text-xl font-semibold glass-text">
                {layout.name}
              </h2>
            
            </div>

            {/* Edit Mode Controls */}
            {isEditMode && (
              <div className="flex items-center gap-2 flex-wrap justify-end">
                <Button
                  onClick={async () => {
                    try {
                      await addPageContainer(pageId);
                    } catch (error) {
                      console.error('Failed to add container:', error);
                    }
                  }}
                  size="sm"
                  className="glass-button status-gradient-connected text-white border-0"
                  title="Add container"
                >
                  <Grid3X3 className="h-4 w-4 mr-2" />
                  <T>Add Container</T>
                </Button>

                <Button
                  onClick={handleSaveToApi}
                  size="sm"
                  disabled={isSaving}
                  className={`glass-button ${
                    saveStatus === 'success' 
                      ? 'bg-green-500 text-white' 
                      : saveStatus === 'error'
                      ? 'bg-red-500 text-white'
                      : 'bg-blue-500 text-white'
                  }`}
                  title="Save layout to server"
                >
                  {isSaving ? (
                    <>
                      <div className="h-4 w-4 mr-2 animate-spin rounded-full border-2 border-white border-t-transparent"></div>
                      <T>Saving...</T>
                    </>
                  ) : saveStatus === 'success' ? (
                    <>
                      <svg className="h-4 w-4 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
                      </svg>
                      <T>Saved!</T>
                    </>
                  ) : saveStatus === 'error' ? (
                    <>
                      <svg className="h-4 w-4 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
                      </svg>
                      <T>Failed</T>
                    </>
                  ) : (
                    <>
                      <svg className="h-4 w-4 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 7H5a2 2 0 00-2 2v9a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-3m-1 4l-3-3m0 0l-3 3m3-3v12" />
                      </svg>
                      <T>Save</T>
                    </>
                  )}
                </Button>

                <Button
                  onClick={handleImportLayout}
                  size="sm"
                  className="glass-button"
                  title="Import layout"
                >
                  <Upload className="h-4 w-4 mr-2" />
                  <T>Import</T>
                </Button>

                <Button
                  onClick={handleExportLayout}
                  size="sm"
                  className="glass-button"
                  title="Export layout"
                >
                  <Download className="h-4 w-4 mr-2" />
                  <T>Export</T>
                </Button>
              </div>
            )}
          </div>

          {/* Layout Info */}
          <div className="mt-3 pt-3 border-t border-slate-300/30 dark:border-white/10">
            <p className="text-xs text-slate-500 dark:text-slate-400">
              <T>Containers</T>: {layout.containers.length} | <T>Widgets</T>: {totalWidgets} | <T>Last updated</T>: {new Date(layout.updatedAt).toLocaleString()}
            </p>
          </div>
        </div>
      )}

      {/* Container Canvas */}
      <div className={`glass-card ${className.includes('p-0') ? 'p-1 sm:p-2' : 'p-4'}`}>
        <div 
          className={className.includes('p-0') ? 'space-y-2' : 'space-y-4'}
          onClick={handleCanvasClick}
        >
          {layout.containers
            .sort((a, b) => (a.order || 0) - (b.order || 0))
            .map((container, index, array) => (
                <LayoutContainer
                key={container.id}
                container={container}
                isEditMode={isEditMode}
                pageId={pageId}
                selectedContainerId={selectedContainer}
                onSelect={handleSelectContainer}
                onAddWidget={handleAddWidget}
                isCompactMode={className.includes('p-0')}
                onRemoveWidget={async (widgetId) => {
                  try {
                    await removeWidgetFromPage(pageId, widgetId);
                  } catch (error) {
                    console.error('Failed to remove widget:', error);
                  }
                }}
                onMoveWidget={async (widgetId, direction) => {
                  try {
                    await moveWidgetInPage(pageId, widgetId, direction);
                  } catch (error) {
                    console.error('Failed to move widget:', error);
                  }
                }}
                onUpdateColumns={async (containerId, columns) => {
                  try {
                    await updatePageContainerColumns(pageId, containerId, columns);
                  } catch (error) {
                    console.error('Failed to update columns:', error);
                  }
                }}
                onUpdateSettings={async (containerId, settings) => {
                  try {
                    await updatePageContainerSettings(pageId, containerId, settings);
                  } catch (error) {
                    console.error('Failed to update settings:', error);
                  }
                }}
                onAddContainer={async (parentId) => {
                  try {
                    await addPageContainer(pageId, parentId);
                  } catch (error) {
                    console.error('Failed to add container:', error);
                  }
                }}
                onMoveContainer={async (containerId, direction) => {
                  try {
                    await movePageContainer(pageId, containerId, direction);
                  } catch (error) {
                    console.error('Failed to move container:', error);
                  }
                }}
                canMoveContainerUp={index > 0}
                canMoveContainerDown={index < array.length - 1}
                onRemoveContainer={async (containerId) => {
                  try {
                    await removePageContainer(pageId, containerId);
                  } catch (error) {
                    console.error('Failed to remove container:', error);
                  }
                }}
              />
            ))}

          {layout.containers.length === 0 && (
            <div className="text-center py-12 text-slate-500 dark:text-slate-400">
              {isEditMode ? (
                <>
                  <Grid3X3 className="h-12 w-12 mx-auto mb-4 opacity-50" />
                  <p className="text-lg font-medium mb-2">
                    <T>No containers yet</T>
                  </p>
                  <p className="text-sm">
                    <T>Click "Add Container" to start building your layout</T>
                  </p>
                </>
              ) : (
                <>
                  <p className="text-lg font-medium mb-2">
                    <T>Empty Layout</T>
                  </p>
                  <p className="text-sm">
                    <T>Switch to edit mode to add containers</T>
                  </p>
                </>
              )}
            </div>
          )}
        </div>
      </div>

      {/* Widget Palette Modal */}
      <WidgetPalette
        isVisible={showWidgetPalette}
        onClose={() => {
          setShowWidgetPalette(false);
          setTargetContainerId(null);
        }}
        onWidgetAdd={handleWidgetAdd}
      />
    </div>
  );
};

export const GenericCanvas = GenericCanvasComponent;