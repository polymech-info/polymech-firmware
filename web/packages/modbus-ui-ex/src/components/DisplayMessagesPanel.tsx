import React from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { T } from '../i18n';
import CollapsibleSection from './CollapsibleSection';
import { Button } from './ui/button';
import { X } from 'lucide-react';

const DisplayMessagesPanel = () => {
  const { displayMessages, removeDisplayMessage, clearDisplayMessages } = useModbus();

  return (
    <CollapsibleSection
      title={<T>System Messages</T>}
      initiallyOpen={true}
      storageKey="displayMessagesPanelOpen"
      className="mt-3 md:mt-4"
      headerContent={
        displayMessages &&
        displayMessages.length > 0 && (
          <Button
            variant="outline"
            size="sm"
            className="h-6 px-2"
            onClick={(e) => {
              e.stopPropagation();
              clearDisplayMessages();
            }}
            title="Clear all messages"
          >
            <T>Clear</T>
          </Button>
        )
      }
    >
      {displayMessages && displayMessages.length > 0 ? (
        <ul className="space-y-2 pt-2">
          {displayMessages.map((msg) => (
            <li key={msg.id} className="flex items-center justify-between text-xs md:text-sm font-mono p-2 bg-black/20 rounded-md break-all">
              <div>
                {(msg as any).timestamp && (
                  <span className="text-muted-foreground mr-2">
                    {new Date((msg as any).timestamp).toLocaleTimeString()}
                  </span>
                )}
                <span className="font-bold mr-2 text-primary">{`[${msg.id}]`}</span>
                <span>{msg.message}</span>
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
        <div className="text-xs md:text-sm font-mono p-2 text-muted-foreground italic">
          <T>No new system messages.</T>
        </div>
      )}
    </CollapsibleSection>
  );
};

export default DisplayMessagesPanel; 