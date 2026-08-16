import React, { useState, type ReactNode, useEffect } from 'react';
import { ChevronDown, ChevronUp } from 'lucide-react';
import { Button } from '@/components/ui/button';
import { Card, CardHeader, CardTitle, CardContent } from '@/components/ui/card'; // Optional: if we want to style it like a card by default

interface CollapsibleSectionProps {
  title: ReactNode;
  children: ReactNode;
  initiallyOpen?: boolean;
  storageKey?: string; // New prop for localStorage key
  className?: string;
  headerClassName?: string;
  headerContent?: ReactNode;
  titleClassName?: string;
  buttonClassName?: string;
  contentClassName?: string;
  asCard?: boolean; // New prop to decide if it should render as a Card
  onStateChange?: (isOpen: boolean) => void; // New prop for state change callback
}

const CollapsibleSection: React.FC<CollapsibleSectionProps> = ({
  title,
  children,
  initiallyOpen = true,
  storageKey,
  className = '',
  headerClassName = 'flex justify-between items-center p-2 cursor-pointer border-b border-border', // Changed p-3 to p-2
  headerContent,
  titleClassName = 'text-md font-semibold',
  buttonClassName = 'p-1 h-auto', // Made button smaller
  contentClassName = 'p-2', // Changed p-3 to p-2
  asCard = false, // Default to not rendering as a card
  onStateChange, // Destructure new prop
}) => {
  const [isOpen, setIsOpen] = useState(() => {
    if (storageKey) {
      try {
        const storedState = localStorage.getItem(storageKey);
        if (storedState !== null) {
          return JSON.parse(storedState) as boolean;
        }
      } catch (error) {
        console.error(`Error reading CollapsibleSection state from localStorage for key "${storageKey}":`, error);
      }
    }
    return initiallyOpen;
  });

  useEffect(() => {
    if (storageKey) {
      try {
        localStorage.setItem(storageKey, JSON.stringify(isOpen));
        if (onStateChange) { // Call onStateChange when state is synced from localStorage
          onStateChange(isOpen);
        }
      } catch (error) {
        console.error(`Error writing CollapsibleSection state to localStorage for key "${storageKey}":`, error);
      }
    }
  }, [isOpen, storageKey, onStateChange]);

  const toggleOpen = () => {
    const newState = !isOpen;
    setIsOpen(newState);
    if (onStateChange) { // Call onStateChange when toggling
      onStateChange(newState);
    }
  };

  if (asCard) {
    return (
      <Card className={`shadow-md ${className}`}>
        <CardHeader className={`cursor-pointer ${headerClassName}`} onClick={toggleOpen}>
          <div className="flex justify-between items-center w-full">
            <CardTitle className={titleClassName}>{title}</CardTitle>
            <div className="flex items-center gap-2">
              {headerContent}
              <Button variant="ghost" size="sm" onClick={(e) => { e.stopPropagation(); toggleOpen(); }} className={buttonClassName}>
                {isOpen ? <ChevronUp className="h-5 w-5" /> : <ChevronDown className="h-5 w-5" />}
              </Button>
            </div>
          </div>
        </CardHeader>
        {isOpen && <CardContent className={contentClassName}>{children}</CardContent>}
      </Card>
    );
  }

  return (
    <div className={`rounded-lg shadow-md border border-border bg-card ${className}`}>
      <div className={`${headerClassName}`} onClick={toggleOpen}>
        <h3 className={titleClassName}>{title}</h3>
        <div className="flex items-center gap-2">
          {headerContent}
          <Button variant="ghost" size="sm" onClick={(e) => { e.stopPropagation(); toggleOpen(); }} className={buttonClassName}>
            {isOpen ? <ChevronUp className="h-5 w-5" /> : <ChevronDown className="h-5 w-5" />}
          </Button>
        </div>
      </div>
      {isOpen && <div className={contentClassName}>{children}</div>}
    </div>
  );
};

export default CollapsibleSection; 