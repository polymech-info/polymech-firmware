import React from 'react';
import { Button } from '@/components/ui/button';
import { Plus } from 'lucide-react';
import { T } from '@/i18n';
import { GenericCanvas } from '@/components/hmi/GenericCanvas';
import { useLayout } from '@/contexts/LayoutContext';

interface ExtensionSlotProps {
  id: string;                    // Unique identifier for this slot
  currentPageId: string;         // The ID of the page containing this slot
  title?: string;                 // Display name for the slot
  isEditMode?: boolean;          // Whether edit mode is enabled
  canvasConfig?: {
    defaultColumns?: number;
    defaultGap?: number;
    showTitle?: boolean;
  };
}



export const ExtensionSlot: React.FC<ExtensionSlotProps> = ({
  id,
  currentPageId,
  title = '',
  isEditMode = false,
  canvasConfig,
}) => {
  const { loadPageLayout, getLoadedPageLayout, updatePageContainerColumns } = useLayout();
  
  const slotPageId = `${currentPageId}-slot-${id}`;
  const slotLayout = getLoadedPageLayout(slotPageId);
  const layoutName = title ? `${title} Canvas` : 'Canvas';
  
  
  React.useEffect(() => {
    // Load the slot layout if it doesn't exist
    if (!slotLayout) {
      loadPageLayout(slotPageId, layoutName);
    }
  }, [slotPageId, slotLayout, loadPageLayout, layoutName]);
  
  const handleAddCanvas = async () => {
    try {
      // Load or create the layout for this slot
      await loadPageLayout(slotPageId, layoutName);
      
      // Apply default configuration if specified
      if (canvasConfig?.defaultColumns) {
        // Wait a bit for the layout to be loaded, then update the first container
        setTimeout(async () => {
          const layout = getLoadedPageLayout(slotPageId);
          if (layout && layout.containers.length > 0) {
            const firstContainer = layout.containers[0];
            await updatePageContainerColumns(slotPageId, firstContainer.id, canvasConfig.defaultColumns!);
          }
        }, 100);
      }
    } catch (error) {
      console.error('Failed to add canvas to slot:', error);
    }
  };
  
  // A canvas exists if we have a layout (even if empty)
  const hasCanvas = !!slotLayout;
  const hasWidgets = slotLayout && slotLayout.containers.some(c => c.widgets.length > 0 || c.children.length > 0);
  
  // Only one canvas per slot - can add only if no canvas exists
  const canAddCanvas = !hasCanvas;
  
  // Only show canvas if in edit mode OR if it has widgets (so users can see their widgets in view mode)
  const shouldShowCanvas = slotLayout && (isEditMode || hasWidgets);
  
  // Extension slots are now URL-specific, so they always show for their respective URLs
  
  return (
    <div className="extension-slot" data-slot-id={id}>
      {/* Render existing canvas if it should be visible */}
      {shouldShowCanvas && (
        <div className="mb-2">
          <GenericCanvas
            pageId={slotPageId}
            pageName={slotLayout.name}
            isEditMode={isEditMode} // Use the actual edit mode state
            showControls={isEditMode} // Only show controls in edit mode
            className="p-0" // Remove padding from GenericCanvas in extension slots
          />
        </div>
      )}
      
      {/* Add Canvas Button (only in edit mode when no canvas exists) */}
      {isEditMode && canAddCanvas && (
        <div className="extension-slot-controls border-2 border-dashed border-blue-300 dark:border-blue-600 rounded-lg p-4 text-center mb-4 bg-blue-50/20 dark:bg-blue-900/20">
          <p className="text-sm text-slate-600 dark:text-slate-400 mb-2">
            <T>Extension Slot</T>: {title}
          </p>
          <Button
            onClick={handleAddCanvas}
            size="sm"
            className="bg-blue-500 hover:bg-blue-600 text-white"
          >
            <Plus className="h-4 w-4 mr-2" />
            <T>Add Canvas Here</T>
          </Button>
        </div>
      )}
    </div>
  );
};
