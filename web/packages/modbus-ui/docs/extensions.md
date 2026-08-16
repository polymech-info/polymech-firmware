# Extension Slots System - Canvas Insertion Design

## Overview

Simple system for adding widget canvases to existing pages using dedicated extension slot components. When in edit mode, users can add a canvas before or after known extension points.

## Concept

### Extension Slot Component
A dedicated component that marks insertion points on any page:

```typescript
<ExtensionSlot 
  id="coils-page-header" 
  position="after-header"
  pageId="coils-page"
/>
```

### Edit Mode Behavior
When edit mode is enabled:
1. Extension slots become visible with "Add Canvas" buttons
2. User clicks "Add Canvas Before" or "Add Canvas After"  
3. New canvas is inserted at that position
4. Canvas uses existing layout system (containers + widgets)

## Architecture

### Core Components

#### ExtensionSlot Component
```typescript
// src/components/layout/ExtensionSlot.tsx
interface ExtensionSlotProps {
  id: string;                    // Unique identifier for this slot
  pageId: string;                // Page this slot belongs to
  position: string;              // Descriptive position (e.g., "after-header")
  title?: string;                // Display name for the slot
  allowMultiple?: boolean;       // Allow multiple canvases at this slot
  canvasConfig?: {
    defaultColumns?: number;
    defaultGap?: number;
    showTitle?: boolean;
  };
}

const ExtensionSlot: React.FC<ExtensionSlotProps> = ({
  id,
  pageId, 
  position,
  title,
  allowMultiple = false,
  canvasConfig
}) => {
  const { isEditMode } = useLayout();
  const { getSlotCanvases, addCanvasToSlot } = useExtensionSlots();
  
  const canvases = getSlotCanvases(pageId, id);
  const canAddMore = allowMultiple || canvases.length === 0;
  
  return (
    <div className="extension-slot" data-slot-id={id}>
      {/* Render existing canvases */}
      {canvases.map(canvas => (
        <GenericCanvas
          key={canvas.id}
          pageId={pageId}
          pageName={canvas.name}
          isEditMode={isEditMode}
          showControls={isEditMode}
          canvasId={canvas.id}
          slotId={id}
        />
      ))}
      
      {/* Add Canvas Button (Edit Mode Only) */}
      {isEditMode && canAddMore && (
        <div className="extension-slot-controls border-2 border-dashed border-blue-300 rounded-lg p-4 text-center">
          <p className="text-sm text-slate-600 mb-2">
            Extension Slot: {title || position}
          </p>
          <Button
            onClick={() => addCanvasToSlot(pageId, id, canvasConfig)}
            size="sm"
            className="bg-blue-500 text-white"
          >
            <Plus className="h-4 w-4 mr-2" />
            Add Canvas Here
          </Button>
        </div>
      )}
    </div>
  );
};
```

#### Extension Slots Context
```typescript
// src/contexts/ExtensionSlotsContext.tsx
interface ExtensionSlotsContextType {
  getSlotCanvases: (pageId: string, slotId: string) => SlotCanvas[];
  addCanvasToSlot: (pageId: string, slotId: string, config?: CanvasConfig) => Promise<SlotCanvas>;
  removeCanvasFromSlot: (pageId: string, canvasId: string) => Promise<void>;
  updateCanvasConfig: (pageId: string, canvasId: string, config: Partial<CanvasConfig>) => Promise<void>;
}

interface SlotCanvas {
  id: string;
  name: string;
  slotId: string;
  pageId: string;
  config: CanvasConfig;
  createdAt: number;
  updatedAt: number;
}

interface CanvasConfig {
  columns: number;
  gap: number;
  showTitle: boolean;
  title?: string;
  collapsible?: boolean;
}
```

### Storage Schema

#### Extended Layout Structure
```typescript
// Storage in layout.json
{
  "pages": {
    "coils-page": {
      "id": "coils-page",
      "name": "Coils Page",
      "containers": [],              // Empty for non-canvas pages
      "extensionSlots": {            // New field for extension slot canvases
        "coils-page-header": [
          {
            "id": "canvas-123",
            "name": "Header Dashboard",
            "slotId": "coils-page-header", 
            "pageId": "coils-page",
            "config": { "columns": 2, "gap": 16, "showTitle": true },
            "layout": {               // Standard layout structure
              "containers": [
                {
                  "id": "container-456",
                  "widgets": [
                    { "id": "widget-789", "widgetId": "status-chart" }
                  ]
                }
              ]
            }
          }
        ]
      }
    }
  }
}
```

## Implementation Plan

### Phase 1: Core Infrastructure (3 days)

#### Day 1: Extension Slot Component
```
□ Create ExtensionSlot component with add canvas functionality
□ Implement basic slot registration and canvas creation
□ Add edit mode integration with show/hide controls
□ Test with simple slot placement
```

#### Day 2: Storage Integration  
```
□ Extend UnifiedLayoutManager for extension slot canvases
□ Add ExtensionSlotsContext for state management
□ Implement slot canvas CRUD operations
□ Add persistence to layout.json structure
```

#### Day 3: Canvas Integration
```
□ Modify GenericCanvas to work within extension slots
□ Add slot-aware canvas identification
□ Implement canvas removal from slots
□ Add canvas configuration options
```

### Phase 2: Page Integration (2 days)

#### Day 4: Page Enhancement
```
□ Add ExtensionSlot components to CoilsPage
□ Add ExtensionSlot components to RegistersPage  
□ Add ExtensionSlot components to SettingsDisplay
□ Test canvas addition/removal on each page
```

#### Day 5: Polish & Testing
```
□ Add visual feedback for slot boundaries
□ Implement canvas title editing
□ Add canvas reordering within slots
□ Performance testing and optimization
```

## Usage Examples

### CoilsPage with Extension Slots
```typescript
// src/pages/CoilsPage.tsx
const CoilsPage = () => {
  return (
    <div className="coils-page">
      {/* Header Extension Slot */}
      <ExtensionSlot 
        id="coils-header"
        pageId="coils-page" 
        position="page-header"
        title="Header Widgets"
        canvasConfig={{ defaultColumns: 3, showTitle: true }}
      />
      
      {/* Existing Coils Content */}
      <CoilsTable />
      <CoilsControls />
      
      {/* Footer Extension Slot */}
      <ExtensionSlot 
        id="coils-footer"
        pageId="coils-page"
        position="page-footer" 
        title="Footer Widgets"
        allowMultiple={true}
      />
    </div>
  );
};
```

### RegistersPage with Extension Slots
```typescript
// src/pages/RegistersPage.tsx  
const RegistersPage = () => {
  return (
    <div className="registers-page">
      <ExtensionSlot 
        id="registers-sidebar"
        pageId="registers-page"
        position="sidebar"
        title="Sidebar Dashboard"
        canvasConfig={{ defaultColumns: 1, collapsible: true }}
      />
      
      <div className="main-content">
        <RegistersTable />
        
        <ExtensionSlot 
          id="registers-inline"
          pageId="registers-page"
          position="after-table"
          title="Analysis Widgets"
        />
      </div>
    </div>
  );
};
```

## Benefits

### ✅ Advantages
- **Simple Implementation** - Leverages existing layout system
- **Predictable UX** - Clear insertion points, no guessing
- **Maintainable** - Standard React components, no DOM manipulation
- **Flexible** - Can add multiple slots per page with different configs
- **Compatible** - Works with existing canvas and widget systems

### 🎯 User Experience
1. **View Mode** - Extension slots are invisible, page looks normal
2. **Edit Mode** - Slots show "Add Canvas Here" buttons with clear labels
3. **Canvas Added** - Full canvas functionality within the slot
4. **Persistent** - Canvases save to API with rest of layout data

This approach gives you **80% of the functionality with 20% of the complexity** compared to universal DOM insertion! 🚀
