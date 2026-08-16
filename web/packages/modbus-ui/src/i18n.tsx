import React, { useState, useEffect, ReactNode, useMemo } from 'react';
// import { translations as generatedTranslations } from '../src/translations'; // Generated translations

// --- Language Configuration ---
type LangCode = 'en' | 'fr' | 'sw' | 'de' | 'es' | 'it' | 'ja' | 'ko' | 'pt' | 'ru' | 'tr' | 'zh' | 'nl';
const DEFAULT_LANG: LangCode = 'en';

export const supportedLanguages = [
  { code: 'en', name: 'English' },
  { code: 'fr', name: 'Français' },
  { code: 'sw', name: 'Kiswahili' },
  { code: 'de', name: 'Deutsch' },
  { code: 'es', name: 'Español' },
  { code: 'it', name: 'Italiano' },
  { code: 'nl', name: 'Nederlands' }
];

// --- Caching and Loading ---
const translationCache: { [lang: string]: Record<string, string> } = {};
const loadingPromises: { [lang: string]: Promise<Record<string, string>> } = {};
const translationsLoaded: { [lang: string]: boolean } = {};

// --- Pub/Sub for updates ---
let listeners: (() => void)[] = [];
const subscribe = (callback: () => void) => {
  listeners.push(callback);
  return () => {
    listeners = listeners.filter(l => l !== callback);
  };
};
const notify = () => {
  listeners.forEach(l => l());
};

const loadTranslations = (lang: LangCode): Promise<Record<string, string>> => {
  if (translationsLoaded[lang]) {
    return Promise.resolve(translationCache[lang] || {});
  }

  return loadingPromises[lang] || (() => {
    const promise = import(`./i18n/${lang}.json`)
      .then(module => {
        const translations = module.default;
        // Merge with existing auto-collected keys, but prioritize loaded translations
        const existingCache = translationCache[lang] || {};
        translationCache[lang] = { ...existingCache, ...translations };
        translationsLoaded[lang] = true;
        delete loadingPromises[lang];
        notify(); // Notify components to re-render with new translations
        return translationCache[lang];
      })
      .catch(error => {
        console.warn(`Could not load translations for language: ${lang}`, error);
        // Don't mark as loaded if it failed, but keep auto-collected keys
        delete loadingPromises[lang];
        notify(); // Still notify to update state
        return translationCache[lang] || {};
      });
    
    loadingPromises[lang] = promise;
    return promise;
  })();
};

const getCurrentLangInternal = (): LangCode => {
  if (typeof window !== 'undefined') {
    const params = new URLSearchParams(window.location.search);
    const langParam = params.get('lang') as LangCode;
    if (langParam && langParam.length === 2) {
      return langParam;
    }

    const browserLangs = navigator.languages || [navigator.language];
    for (const lang of browserLangs) {
      const shortLang = lang.split('-')[0] as LangCode;
      if (shortLang && shortLang.length === 2) {
        return shortLang;
      }
    }
  }
  return DEFAULT_LANG;
};
export const getCurrentLang = getCurrentLangInternal;


// Helper function to get a translated string directly (SYNC)
export const translate = (textKey: string, langParam?: LangCode): string => {
  const langToUse = langParam || getCurrentLangInternal();
  
  // 1. Try loaded translations for current language
  if (translationsLoaded[langToUse] && translationCache[langToUse] && translationCache[langToUse][textKey] !== undefined) {
    return translationCache[langToUse][textKey];
  }

  // 2. If not found and lang is not default, try loaded translations for default language
  if (langToUse !== DEFAULT_LANG && translationsLoaded[DEFAULT_LANG] && translationCache[DEFAULT_LANG] && translationCache[DEFAULT_LANG][textKey] !== undefined) {
    return translationCache[DEFAULT_LANG][textKey];
  }

  // 3. Auto-populate cache with key for dictionary building (only if not already loaded)
  if (!translationCache[langToUse]) {
    translationCache[langToUse] = {};
  }
  if (translationCache[langToUse][textKey] === undefined) {
    translationCache[langToUse][textKey] = textKey; // Store key as value for now
  }

  // 4. Ultimate fallback: return the key itself
  return textKey;
};

interface TProps {
  children: ReactNode;
}

const T: React.FC<TProps> = ({ children }) => {
  const [, forceUpdate] = useState(0);

  useEffect(() => {
    return subscribe(() => {
        forceUpdate(c => c + 1);
    });
  }, []);

  const currentLang = getCurrentLangInternal();
  useEffect(() => {
    loadTranslations(currentLang);
  }, [currentLang]);

  const textKey = useMemo(() => {
    if (typeof children === 'string') return children;
    if (React.isValidElement(children) && typeof children.props.children === 'string') {
      return children.props.children;
    }
    return null;
  }, [children]);

  if (!textKey) {
    return <>{children}</>;
  }

  const translatedString = translate(textKey, currentLang);

  if (React.isValidElement(children) && typeof children.props.children === 'string') {
    return React.cloneElement(children, {}, translatedString);
  }
  return <>{translatedString}</>;
};

export const getTranslationCache = (lang?: LangCode): Record<string, string> | Record<string, Record<string, string>> => {
  if (lang) {
    return translationCache[lang] || {};
  }
  return translationCache;
};

// Helper function to download collected translations as JSON file
export const downloadTranslations = (lang: LangCode = 'en') => {
  const translations = translationCache[lang] || {};
  const isLoaded = translationsLoaded[lang];
  
  const sortedTranslations = Object.keys(translations)
    .sort()
    .reduce((result, key) => {
      result[key] = translations[key];
      return result;
    }, {} as Record<string, string>);
  
  const jsonString = JSON.stringify(sortedTranslations, null, 2);
  const blob = new Blob([jsonString], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  
  const link = document.createElement('a');
  link.href = url;
  link.download = `${lang}.json`;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
  URL.revokeObjectURL(url);
  
  console.log(`Downloaded ${Object.keys(sortedTranslations).length} translation keys for language: ${lang} (${isLoaded ? 'loaded from file' : 'auto-collected'})`);
};

// Helper function to check translation loading status
export const getTranslationStatus = (lang?: LangCode) => {
  if (lang) {
    return {
      loaded: translationsLoaded[lang] || false,
      keysCount: Object.keys(translationCache[lang] || {}).length
    };
  }
  
  const status: Record<string, { loaded: boolean; keysCount: number }> = {};
  for (const langCode of Object.keys(translationCache)) {
    status[langCode] = {
      loaded: translationsLoaded[langCode] || false,
      keysCount: Object.keys(translationCache[langCode] || {}).length
    };
  }
  return status;
};

if (typeof window !== 'undefined') {
  (window as any).getTranslationCache = getTranslationCache;
  (window as any).translate = translate; 
  (window as any).getCurrentLang = getCurrentLang;
  (window as any).downloadTranslations = downloadTranslations;
  (window as any).getTranslationStatus = getTranslationStatus;
}

export { T };
