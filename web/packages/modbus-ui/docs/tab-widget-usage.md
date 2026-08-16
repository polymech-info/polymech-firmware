# Tab Widget Usage

The Tab Widget provides a tabbed interface with individual layout containers for each tab, similar to the AdvancedPage implementation but as a reusable widget.

## Features

- **Dynamic Tab Management**: Add/remove tabs with a plus button
- **Nested Routing**: Each tab creates its own route under the current URL
- **Individual Layout Containers**: Each tab has its own layout area for widgets
- **Persistent State**: Tab configuration and last active tab are saved to localStorage
- **Edit Mode**: Double-click tab labels to rename them
- **Configurable**: Maximum tabs, default columns, and other settings

## Usage in Routes

The TabWidget integrates with React Router and creates nested routes:

```
/your-page/
├── tab1 (first tab)
├── tab2 (second tab)
└── custom-tab-name (additional tabs)
```

## Widget Properties

- `widgetId`: Unique identifier for the widget instance (default: 'tab-widget')
- `initialTabs`: Array of initial tab configurations
- `maxTabs`: Maximum number of tabs allowed (default: 10)
- `defaultColumns`: Default number of columns in each tab's layout (default: 2)
- `showTitle`: Whether to show container titles (default: false)

## Tab Configuration

Each tab has:
- `id`: Unique identifier
- `label`: Display name (editable by double-clicking)
- `route`: URL route segment

## Layout Integration

Each tab contains a layout container that:
- Supports adding/removing widgets in edit mode
- Has configurable column layout
- Persists widget arrangement
- Integrates with the existing widget system

## Example Usage

The widget is registered in the widget registry and can be added to any layout container through the widget palette.

## Technical Implementation

- **TabWidget**: Main component handling tab navigation and management
- **TabContent**: Layout container for each tab's content
- **TabWidgetContainer**: Wrapper component that sets up routing
- Uses React Router's nested routing with `<Outlet>` for tab content
- Leverages existing LayoutContext for widget management
- Follows the same patterns as AdvancedPage for consistency
