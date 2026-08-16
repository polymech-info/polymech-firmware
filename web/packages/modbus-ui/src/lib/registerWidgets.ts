import { widgetRegistry } from './widgetRegistry';
import { 
  Monitor, 
  Settings, 
  BarChart3, 
  Gauge, 
  Thermometer,
  Cpu,
  Gamepad2,
  MessageSquare,
  Server,
  Info,
  Search,
  ArrowDownToDot,
  Folder,
  ListFilter,
  Layout,
} from 'lucide-react';

// Import your components
import CustomWidgets from '@/components/CustomWidgets';
import ConnectionPanel from '@/components/ConnectionPanel';
import SettingsDisplay from '@/components/SettingsDisplay';
import RealTimeChart from '@/components/RealTimeChart';
import VFDControls from '@/components/VFDControls';
import PressCylinderControl from '@/components/PressCylinderControl';
import ProfilePlayback from '@/components/ProfilePlayback';
import SystemInfoPanel from '@/components/SystemInfoPanel';
import DisplayMessagesPanel from '@/components/DisplayMessagesPanel';
import JoystickControls from '@/components/JoystickControls';
import PotentiometerControls from '@/components/PotentiometerControls';
import LEDBars from '@/components/LEDBars';
import AddressPickerWidget from '@/components/AddressPickerWidget';
import PlungerControlWidget from '@/components/PlungerControlWidget';
import CassandraControllerCard from '@/components/CassandraControllerCard';
import SimpleTabWidget from '@/components/SimpleTabWidget';
import LogViewerWidget from '@/components/widgets/LogViewerWidget';
import ChartWidget from '@/components/widgets/ChartWidget';
import LayoutContainerWidget from '@/components/widgets/LayoutContainerWidget';

export function registerAllWidgets() {
  // Clear existing registrations (useful for HMR)
  widgetRegistry.clear();

  // Control widgets
  widgetRegistry.register({
    component: CustomWidgets,
    metadata: {
      id: 'custom-widgets',
      name: 'Custom Widgets',
      category: 'custom',
      description: 'Watched items and custom controls',
      icon: Monitor,
      defaultProps: {},
      minSize: { width: 300, height: 200 },
      resizable: true,
      tags: ['watch', 'custom', 'control']
    }
  });

  widgetRegistry.register({
    component: VFDControls,
    metadata: {
      id: 'vfd-controls',
      name: 'VFD Controls',
      category: 'control',
      description: 'Variable Frequency Drive controls',
      icon: Gauge,
      defaultProps: {},
      minSize: { width: 250, height: 150 },
      resizable: true,
      tags: ['vfd', 'motor', 'control']
    }
  });

  widgetRegistry.register({
    component: PressCylinderControl,
    metadata: {
      id: 'press-cylinder',
      name: 'Press Cylinder Control',
      category: 'control',
      description: 'Press cylinder control interface',
      icon: Cpu,
      defaultProps: {},
      minSize: { width: 300, height: 200 },
      resizable: true,
      tags: ['press', 'cylinder', 'control']
    }
  });

  widgetRegistry.register({
    component: JoystickControls,
    metadata: {
      id: 'joystick-controls',
      name: 'Joystick Controls',
      category: 'control',
      description: 'Virtual joystick interface',
      icon: Gamepad2,
      defaultProps: {},
      minSize: { width: 200, height: 200 },
      resizable: false,
      tags: ['joystick', 'control', 'input']
    }
  });

  widgetRegistry.register({
    component: PotentiometerControls,
    metadata: {
      id: 'potentiometer-controls',
      name: 'Potentiometer Controls',
      category: 'control',
      description: 'Virtual potentiometer interface',
      icon: Gauge,
      defaultProps: {},
      minSize: { width: 200, height: 150 },
      resizable: true,
      tags: ['potentiometer', 'control', 'input']
    }
  });

  widgetRegistry.register({
    component: PlungerControlWidget,
    metadata: {
      id: 'plunger-control',
      name: 'Plunger Control',
      category: 'control',
      description: 'Complete plunger control interface with VFD monitoring',
      icon: ArrowDownToDot,
      defaultProps: {},
      minSize: { width: 400, height: 350 },
      resizable: true,
      tags: ['plunger', 'control', 'vfd', 'hydraulic']
    }
  });

  // Display widgets
  widgetRegistry.register({
    component: ConnectionPanel,
    metadata: {
      id: 'connection-panel',
      name: 'Connection Panel',
      category: 'system',
      description: 'System connection and info panel',
      icon: Server,
      defaultProps: {},
      minSize: { width: 300, height: 150 },
      resizable: true,
      tags: ['connection', 'system', 'info']
    }
  });

  widgetRegistry.register({
    component: SystemInfoPanel,
    metadata: {
      id: 'system-info',
      name: 'System Information',
      category: 'display',
      description: 'System status and information',
      icon: Info,
      defaultProps: {},
      minSize: { width: 250, height: 100 },
      resizable: true,
      tags: ['system', 'info', 'status']
    }
  });

  widgetRegistry.register({
    component: LEDBars,
    metadata: {
      id: 'led-bars',
      name: 'LED Status Bars',
      category: 'display',
      description: 'LED status indicator bars',
      icon: BarChart3,
      defaultProps: {},
      minSize: { width: 200, height: 100 },
      resizable: true,
      tags: ['led', 'status', 'indicator']
    }
  });

  widgetRegistry.register({
    component: DisplayMessagesPanel,
    metadata: {
      id: 'display-messages',
      name: 'Display Messages',
      category: 'display',
      description: 'System messages and notifications',
      icon: MessageSquare,
      defaultProps: {},
      minSize: { width: 300, height: 150 },
      resizable: true,
      tags: ['messages', 'notifications', 'display']
    }
  });

  // Chart widgets
  widgetRegistry.register({
    component: RealTimeChart,
    metadata: {
      id: 'realtime-chart',
      name: 'Real-time Chart',
      category: 'chart',
      description: 'Real-time data visualization',
      icon: BarChart3,
      defaultProps: {},
      minSize: { width: 400, height: 300 },
      resizable: true,
      tags: ['chart', 'realtime', 'data']
    }
  });

  // Profile widgets
  widgetRegistry.register({
    component: ProfilePlayback,
    metadata: {
      id: 'profile-playback',
      name: 'Profile Playback',
      category: 'control',
      description: 'Temperature profile playback controls',
      icon: Thermometer,
      defaultProps: {},
      minSize: { width: 300, height: 200 },
      resizable: true,
      tags: ['profile', 'temperature', 'control']
    }
  });

  // Settings widgets
  widgetRegistry.register({
    component: SettingsDisplay,
    metadata: {
      id: 'settings-display',
      name: 'Settings Panel',
      category: 'system',
      description: 'System settings configuration',
      icon: Settings,
      defaultProps: {},
      minSize: { width: 400, height: 300 },
      resizable: true,
      tags: ['settings', 'config', 'system']
    }
  });

  // Address selection widget
  widgetRegistry.register({
    component: AddressPickerWidget,
    metadata: {
      id: 'address-picker',
      name: 'Address Picker',
      category: 'control',
      description: 'Select register or coil addresses',
      icon: Search,
      defaultProps: {
        slaveId: 0,
        selectedAddress: null,
        selectedSource: 'auto'
      },
      configSchema: {
        slaveId: {
          type: 'number',
          label: 'Slave ID',
          description: 'Modbus slave ID to filter addresses (0 = show all)',
          min: 0,
          max: 247,
          default: 0
        },
        selectedAddress: {
          type: 'number',
          label: 'Default Address',
          description: 'Pre-selected address (optional)',
          min: 0,
          max: 65535,
          default: null
        },
        selectedSource: {
          type: 'select',
          label: 'Address Type',
          description: 'Type of address (register or coil)',
          options: [
            { value: 'auto', label: 'Auto-detect' },
            { value: 'register', label: 'Register' },
            { value: 'coil', label: 'Coil' }
          ],
          default: 'auto'
        }
      },
      minSize: { width: 300, height: 200 },
      resizable: true,
      tags: ['address', 'picker', 'selection', 'modbus']
    }
  });

  // Cassandra controller widget
  widgetRegistry.register({
    component: CassandraControllerCard,
    metadata: {
      id: 'cassandra-controller',
      name: 'Cassandra Controller',
      category: 'control',
      description: 'Temperature controller display and control',
      icon: Thermometer,
      defaultProps: {
        slaveId: 1,
        name: 'Controller 1'
      },
      configSchema: {
        slaveId: {
          type: 'number',
          label: 'Slave ID',
          description: 'Controller slave ID',
          min: 1,
          max: 247,
          default: 1
        },
        name: {
          type: 'text',
          label: 'Controller Name',
          description: 'Display name for the controller',
          default: 'Controller 1'
        }
      },
      minSize: { width: 350, height: 200 },
      resizable: true,
      tags: ['controller', 'temperature', 'cassandra']
    }
  });

  // Layout widgets
  widgetRegistry.register({
    component: SimpleTabWidget,
    metadata: {
      id: 'tab-widget',
      name: 'Tab Widget',
      category: 'custom',
      description: 'Multi-tab container with individual layout areas for each tab',
      icon: Folder,
      defaultProps: {
        initialTabs: [{ id: 'tab1', label: 'Tab 1' }],
        maxTabs: 10,
        defaultColumns: 2,
        showTitle: false,
        enableEditMode: false
      },
      configSchema: {
        maxTabs: {
          type: 'number',
          label: 'Max Tabs',
          description: 'Maximum number of tabs allowed',
          min: 1,
          max: 20,
          default: 10
        },
        defaultColumns: {
          type: 'number',
          label: 'Default Columns',
          description: 'Default number of columns in each tab',
          min: 1,
          max: 6,
          default: 2
        },
        showTitle: {
          type: 'boolean',
          label: 'Show Container Titles',
          description: 'Show titles for layout containers',
          default: false
        },
        enableEditMode: {
          type: 'boolean',
          label: 'Enable Edit Mode',
          description: 'Allow adding/removing widgets in tabs',
          default: false
        }
      },
      minSize: { width: 400, height: 300 },
      resizable: true,
      tags: ['tabs', 'layout', 'container', 'routing']
    }
  });

  widgetRegistry.register({
    component: LayoutContainerWidget,
    metadata: {
      id: 'layout-container-widget',
      name: 'Nested Layout Container',
      category: 'custom',
      description: 'A widget that contains its own independent layout canvas.',
      icon: Layout,
      defaultProps: {
        nestedPageName: 'Nested Container',
        showControls: true,
      },
      configSchema: {
        nestedPageName: {
          type: 'text',
          label: 'Canvas Name',
          description: 'The display name for the nested layout canvas.',
          default: 'Nested Container'
        },
        showControls: {
          type: 'boolean',
          label: 'Show Canvas Controls',
          description: 'Show the main controls (Add Container, Save, etc.) inside this nested canvas.',
          default: true
        }
      },
      minSize: { width: 300, height: 200 },
      resizable: true,
      tags: ['layout', 'container', 'nested', 'canvas']
    }
  });

  // System widgets
  widgetRegistry.register({
    component: LogViewerWidget,
    metadata: {
      id: 'log-viewer',
      name: 'Log Viewer',
      category: 'system',
      description: 'Real-time WebSocket log viewer with filtering and search',
      icon: ListFilter,
      defaultProps: {
        height: 400,
        autoScroll: true,
        defaultTab: 'all',
        showSearch: true,
        showFilters: true,
        showControls: true,
        maxLogEntries: 1000
      },
      configSchema: {
        height: {
          type: 'number',
          label: 'Height (px)',
          description: 'Widget height in pixels',
          min: 200,
          max: 800,
          default: 400
        },
        autoScroll: {
          type: 'boolean',
          label: 'Auto Scroll',
          description: 'Automatically scroll to new log entries',
          default: true
        },
        defaultTab: {
          type: 'select',
          label: 'Default Tab',
          description: 'Default log level tab to show',
          options: [
            { value: 'all', label: 'All Logs' },
            { value: 'error', label: 'Errors' },
            { value: 'warning', label: 'Warnings' },
            { value: 'info', label: 'Info' },
            { value: 'debug', label: 'Debug' },
            { value: 'trace', label: 'Trace' },
            { value: 'verbose', label: 'Verbose' }
          ],
          default: 'all'
        },
        showSearch: {
          type: 'boolean',
          label: 'Show Search',
          description: 'Show search input field',
          default: true
        },
        showFilters: {
          type: 'boolean',
          label: 'Show Filters',
          description: 'Show component filter dropdown',
          default: true
        },
        showControls: {
          type: 'boolean',
          label: 'Show Controls',
          description: 'Show control buttons (clear, download, etc.)',
          default: true
        },
        maxLogEntries: {
          type: 'number',
          label: 'Max Log Entries',
          description: 'Maximum number of log entries to keep in memory',
          min: 100,
          max: 5000,
          default: 1000
        }
      },
      minSize: { width: 400, height: 300 },
      resizable: true,
      tags: ['logs', 'debug', 'websocket', 'system', 'monitoring']
    }
  });

  // Chart widgets
  widgetRegistry.register({
    component: ChartWidget,
    metadata: {
      id: 'chart-widget',
      name: 'Chart Widget',
      category: 'chart',
      description: 'Minimal real-time chart - add registers/coils as needed',
      icon: BarChart3,
      defaultProps: {
        height: 300,
        duration: 300,
        yMax: 100,
        yMin: 0,
        refreshRateMs: 1000,
        seriesData: '[]'
      },
      configSchema: {
        height: {
          type: 'number',
          label: 'Height (px)',
          description: 'Chart height in pixels',
          min: 200,
          max: 600,
          default: 300
        },
        duration: {
          type: 'number',
          label: 'Duration (s)',
          description: 'Time window to display in seconds',
          min: 30,
          max: 1800,
          default: 300
        },
        yMax: {
          type: 'number',
          label: 'Y-Axis Max',
          description: 'Maximum value for Y-axis',
          default: 100
        },
        yMin: {
          type: 'number',
          label: 'Y-Axis Min',
          description: 'Minimum value for Y-axis',
          default: 0
        },
        refreshRateMs: {
          type: 'number',
          label: 'Refresh Rate (ms)',
          description: 'Data refresh interval in milliseconds',
          min: 100,
          max: 5000,
          default: 1000
        }
      },
      minSize: { width: 300, height: 250 },
      resizable: true,
      tags: ['chart', 'realtime', 'data', 'visualization', 'monitoring']
    }
  });
}
