export interface WidgetInstance {
  id: string;
  widgetId: string; // References the widget registry ID
  props?: Record<string, any>;
  order?: number;
}

export interface LayoutContainer {
  id: string;
  type: 'container';
  columns: number;
  gap: number;
  widgets: WidgetInstance[];
  children: LayoutContainer[];
  order?: number;
  settings?: {
    collapsible?: boolean;
    collapsed?: boolean;
    title?: string;
    showTitle?: boolean;
  };
}

export interface PageLayout {
  id: string;
  name: string;
  containers: LayoutContainer[];
  createdAt: number;
  updatedAt: number;
}

export interface RootLayoutData {
  pages: Record<string, PageLayout>;
  version: string;
  lastUpdated: number;
}

export const clearLayoutCache = () => {
  // @ts-ignore - accessing private static for reset
  UnifiedLayoutManager._cachedRootData = null;
};

import { layoutStorage } from './layoutStorage';

export class UnifiedLayoutManager {
  private static readonly VERSION = '1.0.0';
  private static _cachedRootData: RootLayoutData | null = null;
  private static _loadPromise: Promise<RootLayoutData> | null = null;

  // Generate unique IDs
  static generateId(): string {
    return `${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
  }

  static generateContainerId(): string {
    return `container-${this.generateId()}`;
  }

  static generateWidgetId(): string {
    return `widget-${this.generateId()}`;
  }

  // Load root data from storage (API with localStorage fallback)
  static async loadRootData(): Promise<RootLayoutData> {
    if (this._cachedRootData) {
      return this._cachedRootData;
    }

    if (this._loadPromise) {
      return this._loadPromise;
    }

    this._loadPromise = (async () => {
      try {
        const data = await layoutStorage.load();
        if (data) {
          this._cachedRootData = {
            pages: data.pages || {},
            version: data.version || this.VERSION,
            lastUpdated: data.lastUpdated || Date.now()
          };
          return this._cachedRootData;
        }
      } catch (error) {
        console.error('Failed to load unified layouts:', error);
      } finally {
        this._loadPromise = null;
      }

      // Return default structure
      this._cachedRootData = {
        pages: {},
        version: this.VERSION,
        lastUpdated: Date.now()
      };
      return this._cachedRootData;
    })();

    return this._loadPromise;
  }

  // Save root data to storage (localStorage cache only)
  static async saveRootData(data: RootLayoutData): Promise<void> {
    try {
      data.lastUpdated = Date.now();
      this._cachedRootData = data;
      await layoutStorage.save(data);
    } catch (error) {
      console.error('Failed to save unified layouts:', error);
    }
  }

  // Manual save to API
  static async saveToApi(data?: RootLayoutData): Promise<boolean> {
    try {
      const dataToSave = data || await this.loadRootData();
      dataToSave.lastUpdated = Date.now();
      return await layoutStorage.saveToApiOnly(dataToSave);
    } catch (error) {
      console.error('Failed to save layouts to API:', error);
      return false;
    }
  }

  // Get or create page layout
  static async getPageLayout(pageId: string, defaultName?: string): Promise<PageLayout> {
    const rootData = await this.loadRootData();

    if (!rootData.pages[pageId]) {
      rootData.pages[pageId] = {
        id: pageId,
        name: defaultName || `Page ${pageId}`,
        containers: [
          {
            id: this.generateContainerId(),
            type: 'container',
            columns: 1,
            gap: 16,
            widgets: [],
            children: [],
            order: 0
          }
        ],
        createdAt: Date.now(),
        updatedAt: Date.now()
      };
      await this.saveRootData(rootData);
    }

    return rootData.pages[pageId];
  }

  // Save page layout
  static async savePageLayout(layout: PageLayout): Promise<void> {
    const rootData = await this.loadRootData();
    layout.updatedAt = Date.now();
    rootData.pages[layout.id] = layout;
    await this.saveRootData(rootData);
  }

  // Find container by ID in layout (recursive)
  static findContainer(containers: LayoutContainer[], containerId: string): LayoutContainer | null {
    for (const container of containers) {
      if (container.id === containerId) {
        return container;
      }
      const found = this.findContainer(container.children, containerId);
      if (found) return found;
    }
    return null;
  }

  // Add widget to specific container
  static addWidgetToContainer(
    layout: PageLayout,
    containerId: string,
    widgetId: string,
    targetColumn?: number
  ): WidgetInstance {
    const container = this.findContainer(layout.containers, containerId);

    if (!container) {
      throw new Error(`Container ${containerId} not found`);
    }

    let order = container.widgets.length;

    if (targetColumn !== undefined && targetColumn >= 0 && targetColumn < container.columns) {
      const occupiedPositionsInColumn = container.widgets
        .map((_, index) => index)
        .filter(index => index % container.columns === targetColumn);

      let targetRow = 0;
      while (occupiedPositionsInColumn.includes(targetRow * container.columns + targetColumn)) {
        targetRow++;
      }

      order = targetRow * container.columns + targetColumn;

      if (order > container.widgets.length) {
        order = container.widgets.length;
      }
    }

    const newWidget: WidgetInstance = {
      id: this.generateWidgetId(),
      widgetId,
      props: {},
      order
    };

    container.widgets.splice(order, 0, newWidget);
    container.widgets.forEach((w, i) => w.order = i);

    return newWidget;
  }



  // Remove widget from container
  static removeWidgetFromContainer(layout: PageLayout, widgetInstanceId: string): boolean {
    const removeFromContainers = (containers: LayoutContainer[]): boolean => {
      for (const container of containers) {
        const widgetIndex = container.widgets.findIndex(w => w.id === widgetInstanceId);
        if (widgetIndex >= 0) {
          container.widgets.splice(widgetIndex, 1);
          container.widgets.forEach((w, i) => w.order = i);
          return true;
        }
        if (removeFromContainers(container.children)) {
          return true;
        }
      }
      return false;
    };

    return removeFromContainers(layout.containers);
  }



  // Update widget props
  static updateWidgetProps(layout: PageLayout, widgetInstanceId: string, props: Record<string, any>): boolean {
    const findAndUpdateWidget = (containers: LayoutContainer[]): boolean => {
      for (const container of containers) {
        const widget = container.widgets.find(w => w.id === widgetInstanceId);
        if (widget) {
          widget.props = { ...widget.props, ...props };
          return true;
        }
        if (findAndUpdateWidget(container.children)) {
          return true;
        }
      }
      return false;
    };

    return findAndUpdateWidget(layout.containers);
  }



  // Update container columns
  static updateContainerColumns(layout: PageLayout, containerId: string, columns: number): boolean {
    const container = this.findContainer(layout.containers, containerId);

    if (container) {
      container.columns = Math.max(1, Math.min(12, columns));
      return true;
    }
    return false;
  }



  // Update container gap
  static async updateContainerGap(pageId: string, containerId: string, gap: number): Promise<void> {
    const layout = await this.getPageLayout(pageId);
    const container = this.findContainer(layout.containers, containerId);

    if (container) {
      container.gap = Math.max(0, gap);
      await this.savePageLayout(layout);
    }
  }

  // Update container settings
  static updateContainerSettings(layout: PageLayout, containerId: string, settings: Partial<LayoutContainer['settings']>): boolean {
    const container = this.findContainer(layout.containers, containerId);

    if (container) {
      container.settings = { ...container.settings, ...settings };
      return true;
    }
    return false;
  }



  // Add new container
  static addContainer(
    layout: PageLayout,
    parentContainerId?: string,
    columns: number = 1
  ): LayoutContainer {
    const newContainer: LayoutContainer = {
      id: this.generateContainerId(),
      type: 'container',
      columns,
      gap: 16,
      widgets: [],
      children: [],
      order: 0
    };

    if (parentContainerId) {
      const parentContainer = this.findContainer(layout.containers, parentContainerId);
      if (parentContainer) {
        newContainer.order = parentContainer.children.length;
        parentContainer.children.push(newContainer);
      }
    } else {
      newContainer.order = layout.containers.length;
      layout.containers.push(newContainer);
    }

    return newContainer;
  }



  // Remove container
  static removeContainer(layout: PageLayout, containerId: string, allowRemoveLastContainer: boolean = false): boolean {
    // Don't allow removing the last root container (unless explicitly allowed)
    if (!allowRemoveLastContainer && layout.containers.length === 1 && layout.containers[0].id === containerId) {
      return false;
    }

    // Remove from root containers
    const originalLength = layout.containers.length;
    layout.containers = layout.containers.filter(c => c.id !== containerId);

    // Remove from nested containers (recursive)
    const removeFromChildren = (containers: LayoutContainer[]) => {
      containers.forEach(container => {
        container.children = container.children.filter(c => c.id !== containerId);
        removeFromChildren(container.children);
      });
    };

    removeFromChildren(layout.containers);

    // Return true if something was removed
    return layout.containers.length !== originalLength;
  }

  // Move a root-level container up or down
  static moveRootContainer(layout: PageLayout, containerId: string, direction: 'up' | 'down'): boolean {
    const containerIndex = layout.containers.findIndex(c => c.id === containerId);

    if (containerIndex === -1) {
      return false; // Container not found at root level
    }

    const targetIndex = direction === 'up' ? containerIndex - 1 : containerIndex + 1;

    // Check if the move is within the bounds of the array
    if (targetIndex < 0 || targetIndex >= layout.containers.length) {
      return false;
    }

    // Swap the container with its target
    const temp = layout.containers[containerIndex];
    layout.containers[containerIndex] = layout.containers[targetIndex];
    layout.containers[targetIndex] = temp;

    // After swapping, update the order property for all root containers
    layout.containers.forEach((container, index) => {
      container.order = index;
    });

    return true;
  }



  // Move widget in any direction within its container (grid-aware)
  static moveWidgetInContainer(
    layout: PageLayout,
    widgetInstanceId: string,
    direction: 'up' | 'down' | 'left' | 'right'
  ): boolean {

    // Find the widget and its container
    const findWidgetContainer = (containers: LayoutContainer[]): LayoutContainer | null => {
      for (const container of containers) {
        if (container.widgets.some(w => w.id === widgetInstanceId)) {
          return container;
        }
        const found = findWidgetContainer(container.children);
        if (found) return found;
      }
      return null;
    };

    const container = findWidgetContainer(layout.containers);
    if (!container) return false;

    const widgetIndex = container.widgets.findIndex(w => w.id === widgetInstanceId);
    if (widgetIndex === -1) return false;

    const { columns } = container;
    const totalWidgets = container.widgets.length;

    let targetIndex = -1;

    switch (direction) {
      case 'up':
        targetIndex = widgetIndex - columns;
        break;
      case 'down':
        targetIndex = widgetIndex + columns;
        break;
      case 'left':
        // Prevent wrapping from first to last column
        if (widgetIndex % columns !== 0) {
          targetIndex = widgetIndex - 1;
        }
        break;
      case 'right':
        // Prevent wrapping from last to first column
        if ((widgetIndex + 1) % columns !== 0) {
          targetIndex = widgetIndex + 1;
        }
        break;
    }

    // Check if target is valid
    if (targetIndex >= 0 && targetIndex < totalWidgets) {
      // Perform the swap
      [container.widgets[widgetIndex], container.widgets[targetIndex]] =
        [container.widgets[targetIndex], container.widgets[widgetIndex]];

      // Update order values
      container.widgets.forEach((w, i) => w.order = i);
      return true;
    }

    return false;
  }

  // Get grid position of a widget (row, column)
  static getWidgetGridPosition(
    layout: PageLayout,
    widgetInstanceId: string
  ): { row: number; col: number; containerColumns: number } | null {
    // Find the widget and its container
    const findWidgetContainer = (containers: LayoutContainer[]): LayoutContainer | null => {
      for (const container of containers) {
        if (container.widgets.some(w => w.id === widgetInstanceId)) {
          return container;
        }
        const found = findWidgetContainer(container.children);
        if (found) return found;
      }
      return null;
    };

    const container = findWidgetContainer(layout.containers);
    if (!container) return null;

    const widgetIndex = container.widgets.findIndex(w => w.id === widgetInstanceId);
    if (widgetIndex === -1) return null;

    const row = Math.floor(widgetIndex / container.columns);
    const col = widgetIndex % container.columns;

    return { row, col, containerColumns: container.columns };
  }

  // Move widget to specific grid position (row, column)
  static moveWidgetToGridPosition(
    layout: PageLayout,
    widgetInstanceId: string,
    targetRow: number,
    targetCol: number
  ): boolean {
    // Find the widget and its container
    const findWidgetContainer = (containers: LayoutContainer[]): LayoutContainer | null => {
      for (const container of containers) {
        if (container.widgets.some(w => w.id === widgetInstanceId)) {
          return container;
        }
        const found = findWidgetContainer(container.children);
        if (found) return found;
      }
      return null;
    };

    const container = findWidgetContainer(layout.containers);
    if (!container) return false;

    const widgetIndex = container.widgets.findIndex(w => w.id === widgetInstanceId);
    if (widgetIndex === -1) return false;

    // Validate target position
    if (targetRow < 0 || targetCol < 0 || targetCol >= container.columns) {
      return false;
    }

    const targetIndex = targetRow * container.columns + targetCol;

    // Don't move to the same position
    if (targetIndex === widgetIndex) {
      return false;
    }

    // For positions beyond current widget array, we can extend the array
    // But we need to ensure we don't create gaps
    if (targetIndex >= container.widgets.length) {
      // Only allow if it's the next logical position (no gaps)
      if (targetIndex > container.widgets.length) {
        return false;
      }
    }

    // Move the widget
    const widget = container.widgets.splice(widgetIndex, 1)[0];

    if (targetIndex >= container.widgets.length) {
      // Append at the end
      container.widgets.push(widget);
    } else {
      // Insert at target position
      container.widgets.splice(targetIndex, 0, widget);
    }

    // Update order values
    container.widgets.forEach((w, i) => w.order = i);

    return true;
  }



  // Move widget within container or between containers (existing method for drag-drop)
  static async moveWidget(
    pageId: string,
    widgetInstanceId: string,
    targetContainerId: string,
    newOrder: number
  ): Promise<void> {
    const layout = await this.getPageLayout(pageId);

    // First, remove widget from current location
    let widget: WidgetInstance | null = null;
    const removeFromContainers = (containers: LayoutContainer[]): boolean => {
      for (const container of containers) {
        const widgetIndex = container.widgets.findIndex(w => w.id === widgetInstanceId);
        if (widgetIndex >= 0) {
          widget = container.widgets.splice(widgetIndex, 1)[0];
          // Reorder remaining widgets
          container.widgets.forEach((w, i) => w.order = i);
          return true;
        }
        if (removeFromContainers(container.children)) {
          return true;
        }
      }
      return false;
    };

    const wasRemoved = removeFromContainers(layout.containers);
    if (wasRemoved && widget) {
      // Add to target container
      const targetContainer = this.findContainer(layout.containers, targetContainerId);
      if (targetContainer) {
        const widgetToMove = widget as WidgetInstance;
        widgetToMove.order = Math.max(0, Math.min(newOrder, targetContainer.widgets.length));
        targetContainer.widgets.splice(widgetToMove.order, 0, widgetToMove);
        // Reorder all widgets in target container
        targetContainer.widgets.forEach((w, i) => w.order = i);
      }
    }

    await this.savePageLayout(layout);
  }

  // Get container widget count (including nested)
  static getContainerWidgetCount(container: LayoutContainer): number {
    let count = container.widgets.length;
    container.children.forEach(child => {
      count += this.getContainerWidgetCount(child);
    });
    return count;
  }

  // Export layout to JSON (exports full RootLayoutData structure for consistency with API/filesystem)
  static async exportPageLayout(_pageId: string): Promise<string> {
    const rootData = await this.loadRootData();
    return JSON.stringify(rootData, null, 2);
  }

  // Import layout from JSON (handles both RootLayoutData structure and legacy PageLayout)
  static async importPageLayout(pageId: string, jsonData: string): Promise<PageLayout> {
    try {
      const parsedData = JSON.parse(jsonData);

      let targetPageLayout: PageLayout;

      // Check if this is RootLayoutData structure (has 'pages' property)
      if (parsedData.pages && typeof parsedData.pages === 'object') {
        // This is RootLayoutData structure - import all pages
        const rootData = parsedData as RootLayoutData;

        // Load existing root data
        const existingRootData = await this.loadRootData();

        // Merge all pages from imported data
        Object.keys(rootData.pages).forEach(importedPageId => {
          existingRootData.pages[importedPageId] = rootData.pages[importedPageId];
        });

        existingRootData.lastUpdated = Date.now();

        // Save the merged root data
        await this.saveRootData(existingRootData);

        // Return the specific page layout (create default if not found)
        if (existingRootData.pages[pageId]) {
          targetPageLayout = existingRootData.pages[pageId];
        } else {
          // If the requested pageId wasn't in the import, return the first available page or create default
          const firstPageId = Object.keys(rootData.pages)[0];
          if (firstPageId) {
            targetPageLayout = rootData.pages[firstPageId];
            targetPageLayout.id = pageId; // Update ID to match target
            existingRootData.pages[pageId] = targetPageLayout;
            await this.saveRootData(existingRootData);
          } else {
            throw new Error('No valid page layouts found in imported data.');
          }
        }
      } else {
        // Legacy format - single PageLayout
        targetPageLayout = parsedData as PageLayout;

        // Basic validation
        if (!targetPageLayout.id || !targetPageLayout.containers) {
          throw new Error('Invalid layout data for import.');
        }

        // Ensure the ID in the JSON matches the target pageId
        if (targetPageLayout.id !== pageId) {
          console.warn(`[ULM] Mismatch between target pageId (${pageId}) and imported ID (${targetPageLayout.id}). Overwriting ID.`);
          targetPageLayout.id = pageId;
        }

        // Load the existing root data
        const rootData = await this.loadRootData();

        // Update the specific page layout
        rootData.pages[pageId] = targetPageLayout;
        rootData.lastUpdated = Date.now();

        // Save the updated root data
        await this.saveRootData(rootData);
      }

      console.log('[ULM] Saving imported layout to storage:', targetPageLayout);

      return targetPageLayout;
    } catch (error) {
      console.error(`[ULM] Error importing page layout for ${pageId}:`, error);
      throw error;
    }
  }

  // Clean up invalid widgets from layout
  static async cleanupInvalidWidgets(pageId: string, validWidgetIds: Set<string>): Promise<void> {
    const layout = await this.getPageLayout(pageId);
    let hasChanges = false;

    const cleanContainers = (containers: LayoutContainer[]) => {
      containers.forEach(container => {
        const originalLength = container.widgets.length;
        container.widgets = container.widgets.filter(widget => validWidgetIds.has(widget.widgetId));
        if (container.widgets.length !== originalLength) {
          hasChanges = true;
          container.widgets.forEach((w, i) => w.order = i);
        }
        cleanContainers(container.children);
      });
    };

    cleanContainers(layout.containers);

    if (hasChanges) {
      console.log(`Cleaned up invalid widgets from page ${pageId}`);
      await this.savePageLayout(layout);
    }
  }

  // Convenience methods for common layouts
  static async getPlaygroundLayout(): Promise<PageLayout> {
    return await this.getPageLayout('playground-layout', 'Playground Layout');
  }

  static async getDashboardLayout(): Promise<PageLayout> {
    return await this.getPageLayout('dashboard-layout', 'Dashboard Layout');
  }
}