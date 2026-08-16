import React, { createContext, useContext, useState, useEffect, ReactNode, useCallback, useMemo } from 'react';
import { UnifiedLayoutManager, PageLayout, WidgetInstance, LayoutContainer } from '@/lib/unifiedLayoutManager';
import { widgetRegistry } from '@/lib/widgetRegistry';

interface LayoutContextType {
  // Generic page management
  loadPageLayout: (pageId: string, defaultName?: string) => Promise<void>;
  getLoadedPageLayout: (pageId: string) => PageLayout | null;
  clearPageLayout: (pageId: string) => Promise<void>;

  // Generic page actions
  addWidgetToPage: (pageId: string, containerId: string, widgetId: string, targetColumn?: number) => Promise<WidgetInstance>;
  removeWidgetFromPage: (pageId: string, widgetInstanceId: string) => Promise<void>;
  moveWidgetInPage: (pageId: string, widgetInstanceId: string, direction: 'up' | 'down' | 'left' | 'right') => Promise<void>;
  updatePageContainerColumns: (pageId: string, containerId: string, columns: number) => Promise<void>;
  updatePageContainerSettings: (pageId: string, containerId: string, settings: Partial<LayoutContainer['settings']>) => Promise<void>;
  addPageContainer: (pageId: string, parentContainerId?: string) => Promise<LayoutContainer>;
  removePageContainer: (pageId: string, containerId: string) => Promise<void>;
  movePageContainer: (pageId: string, containerId: string, direction: 'up' | 'down') => Promise<void>;
  updateWidgetProps: (pageId: string, widgetInstanceId: string, props: Record<string, any>) => Promise<void>;
  exportPageLayout: (pageId: string) => Promise<string>;
  importPageLayout: (pageId: string, jsonData: string) => Promise<PageLayout>;

  // Manual save
  saveToApi: () => Promise<boolean>;

  // State
  isLoading: boolean;
  loadedPages: Map<string, PageLayout>;
}

const LayoutContext = createContext<LayoutContextType | undefined>(undefined);

interface LayoutProviderProps {
  children: ReactNode;
}

export const LayoutProvider: React.FC<LayoutProviderProps> = ({ children }) => {
  const [loadedPages, setLoadedPages] = useState<Map<string, PageLayout>>(new Map());
  const [isLoading, setIsLoading] = useState(true);

  // Initialize layouts on mount
  useEffect(() => {
    const initializeLayouts = async () => {
      try {
        // Get valid widget IDs from registry
        const validWidgetIds = new Set(widgetRegistry.getAll().map(w => w.metadata.id));

        // Only cleanup if we have widgets registered
        if (validWidgetIds.size > 0) {
          // Clean up all known pages
          const knownPages = ['playground-layout', 'dashboard-layout', 'profile-layout'];
          await Promise.all(knownPages.map(pageId =>
            UnifiedLayoutManager.cleanupInvalidWidgets(pageId, validWidgetIds)
          ));
        }
      } catch (error) {
        console.error('Failed to initialize layouts:', error);
      } finally {
        setIsLoading(false);
      }
    };

    initializeLayouts();
  }, []);

  const saveLayoutToCache = useCallback(async (pageId: string) => {
    // Load existing data from storage first to preserve all pages
    const existingRootData = await UnifiedLayoutManager.loadRootData();

    // Update with current in-memory data
    const allPages: Record<string, PageLayout> = { ...existingRootData.pages };
    loadedPages.forEach((layout, id) => {
      allPages[id] = layout;
    });

    const rootData = {
      pages: allPages,
      version: existingRootData.version || '1.0.0',
      lastUpdated: Date.now()
    };

    await UnifiedLayoutManager.saveRootData(rootData);

    // Force re-render of the specific page with deep clone
    const currentLayout = loadedPages.get(pageId);
    if (currentLayout) {
      // Create a deep clone to ensure React detects the change
      const clonedLayout = JSON.parse(JSON.stringify(currentLayout));
      setLoadedPages(prev => new Map(prev).set(pageId, clonedLayout));
    }
  }, [loadedPages]);

  const loadPageLayout = useCallback(async (pageId: string, defaultName?: string) => {
    // Only load if not already cached
    if (!loadedPages.has(pageId)) {
      try {
        const layout = await UnifiedLayoutManager.getPageLayout(pageId, defaultName);
        setLoadedPages(prev => new Map(prev).set(pageId, layout));
      } catch (error) {
        console.error(`Failed to load page layout ${pageId}:`, error);
      }
    }
  }, [loadedPages]);

  const getLoadedPageLayout = useCallback((pageId: string): PageLayout | null => {
    return loadedPages.get(pageId) || null;
  }, [loadedPages]);

  const clearPageLayout = useCallback(async (pageId: string) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      // Create a fresh empty layout with one empty container
      const clearedLayout: PageLayout = {
        id: pageId,
        name: currentLayout.name,
        containers: [
          {
            id: UnifiedLayoutManager.generateContainerId(),
            type: 'container',
            columns: 1,
            gap: 16,
            widgets: [],
            children: [],
            order: 0
          }
        ],
        createdAt: currentLayout.createdAt,
        updatedAt: Date.now()
      };

      // Update the in-memory cache
      setLoadedPages(prev => new Map(prev).set(pageId, clearedLayout));

      // Save to localStorage cache
      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to clear page layout ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const addWidgetToPage = useCallback(async (pageId: string, containerId: string, widgetId: string, targetColumn?: number): Promise<WidgetInstance> => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const widget = UnifiedLayoutManager.addWidgetToContainer(currentLayout, containerId, widgetId, targetColumn);
      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);

      return widget;
    } catch (error) {
      console.error(`Failed to add widget to page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const removeWidgetFromPage = useCallback(async (pageId: string, widgetInstanceId: string) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const removed = UnifiedLayoutManager.removeWidgetFromContainer(currentLayout, widgetInstanceId);
      if (!removed) {
        throw new Error(`Widget ${widgetInstanceId} not found`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to remove widget from page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const moveWidgetInPage = useCallback(async (pageId: string, widgetInstanceId: string, direction: 'up' | 'down' | 'left' | 'right') => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const moved = UnifiedLayoutManager.moveWidgetInContainer(currentLayout, widgetInstanceId, direction);
      if (!moved) {
        throw new Error(`Failed to move widget ${widgetInstanceId}`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to move widget in page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const updatePageContainerColumns = useCallback(async (pageId: string, containerId: string, columns: number) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const updated = UnifiedLayoutManager.updateContainerColumns(currentLayout, containerId, columns);
      if (!updated) {
        throw new Error(`Container ${containerId} not found`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to update container columns in page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const updatePageContainerSettings = useCallback(async (pageId: string, containerId: string, settings: Partial<LayoutContainer['settings']>) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const updated = UnifiedLayoutManager.updateContainerSettings(currentLayout, containerId, settings);
      if (!updated) {
        throw new Error(`Container ${containerId} not found`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to update container settings in page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const addPageContainer = useCallback(async (pageId: string, parentContainerId?: string): Promise<LayoutContainer> => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const container = UnifiedLayoutManager.addContainer(currentLayout, parentContainerId);
      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);

      return container;
    } catch (error) {
      console.error(`Failed to add container to page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const removePageContainer = useCallback(async (pageId: string, containerId: string) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      // For extension slots, allow removing the last container (to effectively remove the canvas)
      const isExtensionSlot = pageId.includes('-slot-');

      const removed = UnifiedLayoutManager.removeContainer(currentLayout, containerId, isExtensionSlot);
      if (!removed) {
        throw new Error(`Container ${containerId} not found or cannot be removed`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to remove container from page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const movePageContainer = useCallback(async (pageId: string, containerId: string, direction: 'up' | 'down') => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const moved = UnifiedLayoutManager.moveRootContainer(currentLayout, containerId, direction);
      if (!moved) {
        // This can fail gracefully if the container is at the top/bottom, so no error needed.
        return;
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to move container in page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const updateWidgetProps = useCallback(async (pageId: string, widgetInstanceId: string, props: Record<string, any>) => {
    try {
      const currentLayout = loadedPages.get(pageId);
      if (!currentLayout) {
        throw new Error(`Layout for page ${pageId} not loaded`);
      }

      const updated = UnifiedLayoutManager.updateWidgetProps(currentLayout, widgetInstanceId, props);
      if (!updated) {
        throw new Error(`Widget ${widgetInstanceId} not found`);
      }

      currentLayout.updatedAt = Date.now();

      await saveLayoutToCache(pageId);
    } catch (error) {
      console.error(`Failed to update widget props in page ${pageId}:`, error);
      throw error;
    }
  }, [loadedPages, saveLayoutToCache]);

  const exportPageLayout = useCallback(async (pageId: string): Promise<string> => {
    try {
      return await UnifiedLayoutManager.exportPageLayout(pageId);
    } catch (error) {
      console.error(`Failed to export page layout ${pageId}:`, error);
      throw error;
    }
  }, []);

  const importPageLayout = useCallback(async (pageId: string, jsonData: string): Promise<PageLayout> => {
    try {
      console.log(`[LayoutContext] Importing page layout for ${pageId}...`);
      const layout = await UnifiedLayoutManager.importPageLayout(pageId, jsonData);
      console.log('[LayoutContext] Layout imported successfully from ULM:', layout);
      // Directly update the state with the newly imported layout
      setLoadedPages(prev => {
        const newPages = new Map(prev);
        newPages.set(pageId, layout);
        console.log('[LayoutContext] Updating loadedPages state with new layout for pageId:', pageId, newPages);
        return newPages;
      });
      return layout;
    } catch (error) {
      console.error(`[LayoutContext] Failed to import page layout ${pageId}:`, error);
      throw error;
    }
  }, []);

  const saveToApi = useCallback(async (): Promise<boolean> => {
    try {
      // First ensure all loaded pages are saved to localStorage cache
      const existingRootData = await UnifiedLayoutManager.loadRootData();

      // Update with current in-memory data
      const allPages: Record<string, PageLayout> = { ...existingRootData.pages };
      loadedPages.forEach((layout, id) => {
        allPages[id] = layout;
      });

      const rootData = {
        pages: allPages,
        version: existingRootData.version || '1.0.0',
        lastUpdated: Date.now()
      };

      // Save to localStorage cache first
      await UnifiedLayoutManager.saveRootData(rootData);

      // Then save to API with the exact same data
      return await UnifiedLayoutManager.saveToApi(rootData);
    } catch (error) {
      console.error('Failed to save layouts to API:', error);
      return false;
    }
  }, [loadedPages]);

  const value: LayoutContextType = useMemo(() => ({
    loadPageLayout,
    getLoadedPageLayout,
    clearPageLayout,
    addWidgetToPage,
    removeWidgetFromPage,
    moveWidgetInPage,
    updatePageContainerColumns,
    updatePageContainerSettings,
    addPageContainer,
    removePageContainer,
    movePageContainer,
    updateWidgetProps,
    exportPageLayout,
    importPageLayout,
    saveToApi,
    isLoading,
    loadedPages,
  }), [
    loadPageLayout,
    getLoadedPageLayout,
    clearPageLayout,
    addWidgetToPage,
    removeWidgetFromPage,
    moveWidgetInPage,
    updatePageContainerColumns,
    updatePageContainerSettings,
    addPageContainer,
    removePageContainer,
    movePageContainer,
    updateWidgetProps,
    exportPageLayout,
    importPageLayout,
    saveToApi,
    isLoading,
    loadedPages
  ]);

  return (
    <LayoutContext.Provider value={value}>
      {children}
    </LayoutContext.Provider>
  );
};

export const useLayout = (): LayoutContextType => {
  const context = useContext(LayoutContext);
  if (context === undefined) {
    throw new Error('useLayout must be used within a LayoutProvider');
  }
  return context;
};