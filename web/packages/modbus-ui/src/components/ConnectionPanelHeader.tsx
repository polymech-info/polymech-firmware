
import { useModbus } from '@/contexts/ModbusContext';
import { useLayout } from '@/contexts/LayoutContext';
import { Button } from '@/components/ui/button';
import { Download, Globe, Loader2, Plug, Power, RotateCw, Edit3, Eye, Trash2, ExternalLink } from 'lucide-react';
import { cn } from '@/lib/utils';
import ThemeToggle from '@/components/ThemeToggle'
import { T, getCurrentLang, getTranslationCache, translate, supportedLanguages } from '../i18n';

import {
    DropdownMenu,
    DropdownMenuContent,
    DropdownMenuItem,
    DropdownMenuTrigger,
} from "@/components/ui/dropdown-menu"
import { ENABLE_HMI_EDIT } from '@/constants';

interface ConnectionPanelHeaderProps {
    onEditModeToggle?: (enabled: boolean) => void;
    isEditMode?: boolean;
    currentPageId?: string;
}

const ConnectionPanelHeader: React.FC<ConnectionPanelHeaderProps> = ({
    onEditModeToggle,
    isEditMode = false,
    currentPageId
}) => {
    const {
        isConnected,
        connectToServer,
        disconnectFromServer,
        abortConnectionAttempt,
        connecting,
        updateRegister
    } = useModbus();
    const { clearPageLayout } = useLayout();
    const currentLang = getCurrentLang();

    const pageIdWithLayout = currentPageId ? `${currentPageId}-layout` : undefined;

    const handleLanguageChange = (langCode: string) => {
        const currentUrl = window.location.href;
        const url = new URL(currentUrl);
        url.searchParams.set('lang', langCode);
        window.location.href = url.toString();
    };

    const handleDownloadEnglishTranslations = () => {
        const englishTranslations = getTranslationCache('en');
        const jsonString = JSON.stringify(englishTranslations, null, 2);
        const blob = new Blob([jsonString], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'translations_en.json';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    };

    const handleConnect = async () => {
        // This would need server URL logic if we want to make it fully functional
        // For now, we'll assume the connection logic is handled elsewhere
        if (connectToServer) await connectToServer();
    };

    const handleDisconnect = () => {
        if (disconnectFromServer) disconnectFromServer();
    };

    const handleAbort = () => {
        if (abortConnectionAttempt) abortConnectionAttempt();
    };

    const handleReset = async () => {
        if (updateRegister) {
            try {
                await updateRegister(100, 1);
            } catch (error) {
                // Handle error silently
            }
        }
    };

    const handleClearLayout = async () => {
        if (pageIdWithLayout && window.confirm(translate('Are you sure you want to clear all widgets from this layout?'))) {
            try {
                await clearPageLayout(pageIdWithLayout);
            } catch (error) {
                console.error('Failed to clear layout:', error);
            }
        }
    };

    return (
        <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-3 sm:gap-2">
            <a
                href="https://polymech.info/en/resources/cassandra/"
                target="_blank"
                rel="noopener noreferrer"
                className="flex items-center gap-2 hover:opacity-80 transition-opacity group"
            >
                <h2 className="text-lg md:text-xl font-bold text-gradient text-center sm:text-left cursor-pointer group-hover:underline decoration-blue-500/50 underline-offset-4">
                    <T>PolyMech - Cassandra</T>
                </h2>
                <ExternalLink className="h-4 w-4 md:h-5 md:w-5 text-muted-foreground group-hover:text-blue-500 transition-colors" />
            </a>

            <div className="flex flex-col items-center sm:items-end gap-2">
                {/* First Row: Language, Downloads, Theme, Status, and Connection Controls */}
                <div className="flex items-center flex-wrap justify-center sm:justify-end gap-x-1 md:gap-x-2 gap-y-1">
                    <DropdownMenu>
                        <DropdownMenuTrigger asChild>
                            <Button
                                variant="ghost"
                                size="sm"
                                className="flex items-center gap-1 text-xs md:text-sm p-1 md:p-1.5 h-auto"
                                onClick={(e) => e.stopPropagation()}
                            >
                                <Globe className="h-4 w-4 md:h-5 md:w-5" />
                                <span className="hidden sm:inline">{supportedLanguages.find(l => l.code === currentLang)?.name}</span>
                            </Button>
                        </DropdownMenuTrigger>
                        <DropdownMenuContent>
                            {supportedLanguages.map((lang) => (
                                <DropdownMenuItem key={lang.code} onSelect={() => handleLanguageChange(lang.code)}>
                                    {lang.name}
                                </DropdownMenuItem>
                            ))}
                        </DropdownMenuContent>
                    </DropdownMenu>
                    <Button
                        variant="ghost"
                        size="sm"
                        onClick={(e) => {
                            e.stopPropagation();
                            handleDownloadEnglishTranslations();
                        }}
                        className="p-1 md:p-1.5 h-auto"
                        title={translate("Download English Translations")}
                    >
                        <Download className="h-4 w-4 md:h-5 md:w-5" />
                    </Button>
                    <ThemeToggle />

                    {/* Status indicator */}
                    <div className="flex items-center gap-x-1 md:gap-x-2">
                        <div className={cn(
                            "w-2.5 h-2.5 md:w-3 md:h-3 rounded-full",
                            isConnected ? "bg-green-500 animate-pulse-glow" : "bg-red-500"
                        )}></div>
                        <span className="text-xs md:text-sm font-mono">
                            {isConnected ? <T>ONLINE</T> : <T>OFFLINE</T>}
                        </span>
                    </div>

                    {/* Connection Controls */}
                    {isConnected ? (
                        <>
                            <Button
                                variant="outline"
                                size="sm"
                                onClick={(e) => {
                                    e.stopPropagation();
                                    handleReset();
                                }}
                                className="glass-button flex items-center gap-2 text-xs md:text-sm p-1 md:p-1.5 h-auto"
                            >
                                <RotateCw className="h-4 w-4" />
                                <span className="hidden sm:inline">{translate('Reset')}</span>
                            </Button>
                            <Button
                                variant="destructive"
                                size="sm"
                                onClick={(e) => {
                                    e.stopPropagation();
                                    handleDisconnect();
                                }}
                                className="flex items-center gap-2 text-xs md:text-sm status-gradient-error text-white border-0 hover:shadow-lg transition-all duration-300 p-1 md:p-1.5 h-auto"
                            >
                                <Power className="h-4 w-4" />
                                <span className="hidden sm:inline">{translate('Disconnect')}</span>
                            </Button>

                            {/* HMI Edit Button - moved here after Disconnect */}
                            {onEditModeToggle && ENABLE_HMI_EDIT && (
                                <>
                                    <Button
                                        onClick={(e) => {
                                            e.stopPropagation();
                                            onEditModeToggle(!isEditMode);
                                        }}
                                        size="sm"
                                        className={`flex items-center gap-2 text-xs md:text-sm p-1 md:p-1.5 h-auto transition-all duration-200 ${isEditMode
                                                ? 'bg-blue-500 hover:bg-blue-600 text-white border-0'
                                                : 'glass-button hover:bg-blue-500/10 hover:text-blue-600 dark:hover:text-blue-400'
                                            }`}
                                        title={isEditMode ? 'Exit edit mode' : 'Enter edit mode'}
                                    >
                                        {isEditMode ? (
                                            <>
                                                <Eye className="h-4 w-4" />
                                                <span className="hidden sm:inline"><T>View</T></span>
                                            </>
                                        ) : (
                                            <>
                                                <Edit3 className="h-4 w-4" />
                                                <span className="hidden sm:inline"><T>Edit</T></span>
                                            </>
                                        )}
                                    </Button>

                                    {/* Clear Layout Button - only visible in edit mode */}
                                    {isEditMode && pageIdWithLayout && (
                                        <Button
                                            onClick={(e) => {
                                                e.stopPropagation();
                                                handleClearLayout();
                                            }}
                                            size="sm"
                                            variant="destructive"
                                            className="flex items-center gap-2 text-xs md:text-sm p-1 md:p-1.5 h-auto transition-all duration-200"
                                            title="Clear all widgets from layout"
                                        >
                                            <Trash2 className="h-4 w-4" />
                                            <span className="hidden sm:inline"><T>Clear</T></span>
                                        </Button>
                                    )}
                                </>
                            )}
                        </>
                    ) : (
                        <Button
                            variant="default"
                            size="sm"
                            onClick={(e) => {
                                e.stopPropagation();
                                connecting ? handleAbort() : handleConnect();
                            }}
                            className="status-gradient-connected text-white border-0 flex items-center gap-2 text-xs md:text-sm hover:shadow-lg transition-all duration-300 p-1 md:p-1.5 h-auto"
                        >
                            {connecting ? (
                                <Loader2 className="h-4 w-4 animate-spin" />
                            ) : (
                                <Plug className="h-4 w-4" />
                            )}
                            <span className="hidden sm:inline">{connecting ? translate('Cancel') : translate('Connect')}</span>
                        </Button>
                    )}
                </div>

            </div>
        </div>
    );
}

export default ConnectionPanelHeader; 