
import { Buffer } from 'buffer';
import { Readable, Writable, Transform, Stream } from 'stream-browserify';

// Add Buffer to the global window object
window.Buffer = Buffer;

// Extend Window interface to avoid TypeScript errors
declare global {
  interface Window {
    Buffer: typeof Buffer;
    Stream: typeof Stream;
    Readable: typeof Readable;
    Writable: typeof Writable;
    Transform: typeof Transform;
    stream: { Transform: typeof Transform };
    process: any; // Using any to avoid complex Process type issues
    global: Window;
    __dirname: string;
    __filename: string;
    require: any; // Using any to avoid NodeRequire type issues
  }
}

// Add Stream classes to the global window object
window.Stream = Stream;
window.Readable = Readable;
window.Writable = Writable;
window.Transform = Transform;

// Add the stream namespace with Transform
window.stream = {
  Transform: Transform
};

// Make stream available globally
window.global = window;

// Mock Node.js process object
window.process = window.process || {
  env: {},
  browser: true,
  platform: 'browser',
  cwd: () => '/',
  binding: function() {
    return { modules: {} };
  }
};

// Mock __dirname and __filename for CommonJS modules
if (typeof window.__dirname === 'undefined') {
  Object.defineProperty(window, '__dirname', { value: '/' });
}

if (typeof window.__filename === 'undefined') {
  Object.defineProperty(window, '__filename', { value: '/index.js' });
}

// Mock Node.js module resolution
window.require = function(name: string) {
  console.warn(`Mock require called for: ${name}`);
  if (name === 'util') return {};
  if (name === 'events') return { EventEmitter: class {} };
  return {};
};
