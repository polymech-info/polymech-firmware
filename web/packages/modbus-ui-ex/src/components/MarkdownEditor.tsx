import React, { useEffect, useRef, useState } from 'react';
import { Crepe } from '@milkdown/crepe';
import { getMarkdown, replaceAll } from '@milkdown/utils';
import { UseFormReturn } from 'react-hook-form';

import '@milkdown/crepe/theme/common/style.css';
import '@milkdown/crepe/theme/frame.css';
import { T } from '@/i18n';

interface MarkdownEditorProps {
  value: string;
  form: UseFormReturn<any>;
  fieldName: string;
}

const MarkdownEditor: React.FC<MarkdownEditorProps> = ({ value, form, fieldName }) => {
  const [activeTab, setActiveTab] = useState<'editor' | 'raw'>('editor');
  const ref = useRef<HTMLDivElement>(null);
  const crepeRef = useRef<Crepe | null>(null);
  const debounceTimer = useRef<NodeJS.Timeout>();
  const isUpdatingFromOutside = useRef(false);

  // Effect for editor setup and mutation observer
  useEffect(() => {
    if (activeTab !== 'editor' || !ref.current) {
      crepeRef.current?.destroy();
      crepeRef.current = null;
      return;
    }

    const crepe = new Crepe({
      root: ref.current,
      defaultValue: value || '',
    });

    crepe.create().then(() => {
      crepeRef.current = crepe;
    });

    const observer = new MutationObserver(() => {
      if (isUpdatingFromOutside.current || !crepeRef.current?.editor) return;

      clearTimeout(debounceTimer.current);
      debounceTimer.current = setTimeout(() => {
        const markdown = crepeRef.current!.editor.action(getMarkdown());
        form.setValue(fieldName, markdown, { shouldDirty: true, shouldValidate: true });
      }, 400);
    });

    observer.observe(ref.current, {
      childList: true,
      subtree: true,
      characterData: true,
    });

    return () => {
      observer.disconnect();
      if (debounceTimer.current) {
        clearTimeout(debounceTimer.current);
      }
      crepeRef.current?.destroy();
      crepeRef.current = null;
    };
  }, [form, fieldName, activeTab]); // Only re-initialize if these fundamental props change

  // Effect for handling external value changes
  useEffect(() => {
    if (activeTab !== 'editor' || !crepeRef.current?.editor) return;

    const editorContent = crepeRef.current.editor.action(getMarkdown());

    if (value !== editorContent) {
      isUpdatingFromOutside.current = true;
      crepeRef.current.editor.action(replaceAll(value || ''));
      // Allow DOM to update and be ignored by the observer
      setTimeout(() => {
        isUpdatingFromOutside.current = false;
      }, 100);
    }
  }, [value, activeTab]);

  const handleRawChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    form.setValue(fieldName, e.target.value, { shouldDirty: true, shouldValidate: true });
  };

  return (
    <div className="border rounded-md">
      <div className="flex border-b">
        <button
          type="button"
          onClick={() => setActiveTab('editor')}
          className={`px-4 py-2 ${
            activeTab === 'editor'
              ? 'border-b-2 border-blue-500 text-blue-600'
              : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
          }`}
        >
          <T>Markdown Editor</T>
        </button>
        <button
          type="button"
          onClick={() => setActiveTab('raw')}
          className={`px-4 py-2 ${
            activeTab === 'raw'
              ? 'border-b-2 border-blue-500 text-blue-600'
              : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
          }`}
        >
          <T>Source</T>
        </button>
      </div>
      {activeTab === 'editor' && <div ref={ref} className="milkdown-editor p-2" />}
      {activeTab === 'raw' && (
        <textarea
          value={value || ''}
          onChange={handleRawChange}
          className="w-full p-2 border-0 rounded-b-md focus:ring-0"
          style={{ height: '200px' }}
          aria-label="Raw markdown input"
        />
      )}
    </div>
  );
};

export default MarkdownEditor; 