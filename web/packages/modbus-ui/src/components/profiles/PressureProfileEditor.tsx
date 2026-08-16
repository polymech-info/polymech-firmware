import React, { useState, useEffect } from 'react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import BezierEditor from '@/components/profiles/bezier/BezierEditor';
import { ControlPoint, PressureProfile } from '@/types';
import { Save } from 'lucide-react';
import { toast } from '@/components/ui/use-toast';
import { transformUIControlPointsToService } from '../../lib/profile-transformers';
import { T, translate } from '../../i18n';
import MarkdownEditor from '@/components/MarkdownEditor';
import { PressureProfileSavePayload, RegisterData } from '@polymech/client-ts';
import { useModbus } from '@/contexts/ModbusContext';

interface PressureProfileEditorProps {
    onSubmit: (data: PressureProfileSavePayload) => void;
    initialData?: PressureProfile;
}

const defaultControlPoints: ControlPoint[] = [
    { x: 0, y: 0 },
    { x: 1, y: 0 }
];

const PressureProfileEditor: React.FC<PressureProfileEditorProps> = ({ onSubmit, initialData }) => {
    const [name, setName] = useState(initialData?.name || '');
    const [description, setDescription] = useState(initialData?.description || '');
    const [duration, setDuration] = useState(initialData?.duration || 30 * 60 * 1000);
    const [controlPoints, setControlPoints] = useState<ControlPoint[]>(
        initialData?.controlPoints ?
            initialData.controlPoints.map(cp => ({ x: cp.x / 1000, y: cp.y / 1000 })) :
            defaultControlPoints
    );
    const [maxPressure, setMaxPressure] = useState(initialData?.max || 100);

    // Target registers state
    const [selectedTargetRegisters, setSelectedTargetRegisters] = useState<number[]>([]);

    useEffect(() => {
        const initialValues = initialData?.targetRegisters || [];
        const newSelected: number[] = [];
        for (let i = 0; i < 8; i++) {
            newSelected.push(initialValues[i] !== undefined ? initialValues[i] : 0);
        }
        setSelectedTargetRegisters(newSelected);
    }, [initialData?.targetRegisters]);

    const handleDurationChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const val = parseInt(e.target.value);
        if (!isNaN(val)) setDuration(val * 1000); // Input in seconds, store in ms
    };

    const handleSubmit = () => {
        const payload: PressureProfileSavePayload = {
            slot: initialData?.slot || 0,
            name,
            description,
            duration,
            controlPoints: transformUIControlPointsToService(controlPoints),
            targetRegisters: selectedTargetRegisters,
            max: maxPressure
        };
        onSubmit(payload);
    };

    return (
        <div className="space-y-4 glass-panel p-4 rounded-xl">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                <div>
                    <Label htmlFor="profile-name" className="text-slate-700 dark:text-white"><T>Profile Name</T></Label>
                    <Input
                        id="profile-name"
                        placeholder={translate("E.g., High Pressure Ramp")}
                        value={name}
                        onChange={(e) => setName(e.target.value)}
                        className="glass-input"
                    />
                </div>
                <div>
                    <Label htmlFor="profile-duration" className="text-slate-700 dark:text-white"><T>Duration (seconds)</T></Label>
                    <Input
                        id="profile-duration"
                        type="number"
                        value={duration / 1000}
                        onChange={handleDurationChange}
                        className="glass-input"
                    />
                </div>
            </div>

            <div>
                <Label className="text-slate-700 dark:text-white"><T>Description</T></Label>
                <MarkdownEditor
                    value={description}
                    onValueChange={setDescription}
                />
            </div>




            <div className="mt-4">
                <Label className="text-slate-700 dark:text-white mb-2 block"><T>Pressure Curve</T></Label>
                <BezierEditor
                    controlPoints={controlPoints}
                    onChange={setControlPoints}
                    height={400}
                    max={100}
                    hideMaxInput={true}
                    yAxisLabel="Pressure (%)"
                    unit="%"
                />
            </div>

            <div className="flex justify-end gap-2 mt-4">
                <Button onClick={handleSubmit} className="gap-2">
                    <Save className="h-4 w-4" />
                    <T>Save Profile</T>
                </Button>
            </div>
        </div>
    );
};

export default PressureProfileEditor;
