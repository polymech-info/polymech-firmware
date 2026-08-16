import React from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { T } from '../i18n';
import CollapsibleSection from './CollapsibleSection';
import { Button } from './ui/button';
import { X } from 'lucide-react';

interface DisplayMessagesPanelProps {
  id?: string;
  collapsible?: boolean;
  minimal?: boolean;
}


const DisplayMessagesPanel: React.FC<DisplayMessagesPanelProps> = ({ id, collapsible = true, minimal = false }) => {
  const { displayMessages, removeDisplayMessage, clearDisplayMessages } = useModbus();

  const clearButton = displayMessages &&
    displayMessages.length > 0 && (
      <Button
        variant="outline"
        size="sm"
        className="h-6 px-2 glass-button"
        onClick={(e) => {
          e.stopPropagation();
          clearDisplayMessages();
        }}
        title="Clear all messages"
      >
        <T>Clear</T>
      </Button>
    );

  const content = displayMessages && displayMessages.length > 0 ? (
    <ul className="space-y-2 pt-2">
      {displayMessages.map((msg) => (
        <li key={msg.id} className="flex items-center justify-between text-xs md:text-sm font-mono p-2 glass-card break-all">
          <div>
            {(msg as any).timestamp && (
              <span className="text-slate-500 dark:text-slate-400 mr-2">
                {new Date((msg as any).timestamp).toLocaleTimeString()}
              </span>
            )}
            <span className="font-bold mr-2 text-indigo-600 dark:text-cyan-400">{`[${msg.id}]`}</span>
            <span className="text-slate-700 dark:text-white">{msg.message}</span>
          </div>
          <Button
            variant="ghost"
            size="icon"
            className="h-6 w-6 ml-2 flex-shrink-0"
            onClick={() => removeDisplayMessage(msg.id)}
            title="Dismiss message"
          >
            <X className="h-4 w-4" />
          </Button>
        </li>
      ))}
    </ul>
  ) : (
    <div className="text-xs md:text-sm font-mono p-2 text-slate-500 dark:text-slate-400 italic">
      <T>No new system messages.</T>
    </div>
  );

  if (minimal) {
    // In minimal mode, we might want to show the clear button if there are messages, 
    // but the prompt says "no system messages" title. 
    // We'll render the content and maybe the clear button if needed, but for now just the content 
    // to strictly follow "no extra container". 
    // Actually, let's keep the clear button if there are messages, floating or just above?
    // The user request was "minimal = no extra container, border, ... or 'system messages'".
    // This implies stripping the header. We'll return just the content.
    return (
      <div id={id} className="">
        {content}
      </div>
    );
  }

  if (collapsible) {
    return (
      <CollapsibleSection
        id={id}
        title={<T>System Messages</T>}
        initiallyOpen={true}
        storageKey="displayMessagesPanelOpen"
        className="shadow-none md:shadow-lg"
        headerContent={clearButton}
      >
        {content}
      </CollapsibleSection>
    );
  }

  return (
    <div className={`rounded-lg shadow-none md:shadow-md border border-border bg-card`} id={id}>
      <div className="flex justify-between items-center p-3 md:p-4 border-b border-border">
        <div className="text-md md:text-lg font-semibold"><T>System Messages</T></div>
        <div className="flex items-center gap-2">
          {clearButton}
        </div>
      </div>
      <div className="p-3 md:p-4">
        {content}
      </div>
    </div>
  );
};

export default DisplayMessagesPanel; 