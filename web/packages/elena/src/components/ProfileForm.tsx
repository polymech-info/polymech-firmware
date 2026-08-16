import React, { useState } from 'react';
import { useForm } from 'react-hook-form';
import { zodResolver } from '@hookform/resolvers/zod';
import { z } from 'zod';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Textarea } from '@/components/ui/textarea';
import {
  Form,
  FormControl,
  FormField,
  FormItem,
  FormLabel,
  FormMessage,
} from '@/components/ui/form';
import BezierEditor from './BezierEditor';
import { ControlPoint, TemperatureProfile } from '@/lib/api';

// Form schema
const formSchema = z.object({
  name: z.string().min(1, { message: 'Name is required' }),
  description: z.string(),
  duration: z.coerce.number().min(1, { message: 'Duration must be at least 1 minute' })
});

type FormData = z.infer<typeof formSchema>;

interface ProfileFormProps {
  onSubmit: (data: Omit<TemperatureProfile, 'id' | 'createdAt' | 'updatedAt'>) => void;
  initialData?: TemperatureProfile;
  minTemp: number;
  maxTemp: number;
}

const defaultControlPoints: ControlPoint[] = [
  { x: 0, y: 0 },
  { x: 0.5, y: 0.5, type: 'linear' },
  { x: 1, y: 1 }
];

const ProfileForm: React.FC<ProfileFormProps> = ({ 
  onSubmit, 
  initialData, 
  minTemp,
  maxTemp
}) => {
  const [controlPoints, setControlPoints] = useState<ControlPoint[]>(
    initialData?.controlPoints || defaultControlPoints
  );
  const [tempRange, setTempRange] = useState({
    min: initialData?.minTemp || minTemp,
    max: initialData?.maxTemp || maxTemp
  });
  
  const form = useForm<FormData>({
    resolver: zodResolver(formSchema),
    defaultValues: {
      name: initialData?.name || '',
      description: initialData?.description || '',
      duration: initialData?.duration || 30
    }
  });
  
  const handleTempRangeChange = (newMinTemp: number, newMaxTemp: number) => {
    setTempRange({
      min: newMinTemp,
      max: newMaxTemp
    });
  };

  const handleSubmit = (data: FormData) => {
    // Prepare control points with proper type information
    const preparedControlPoints = controlPoints.map(point => ({
      x: point.x,
      y: point.y,
      handleX: point.handleX,
      handleY: point.handleY,
      type: point.type || 'linear' // Ensure type is always present
    }));
    
    onSubmit({
      name: data.name,
      description: data.description,
      duration: data.duration,
      controlPoints: preparedControlPoints,
      minTemp: tempRange.min,
      maxTemp: tempRange.max
    });
  };
  
  return (
    <Form {...form}>
      <form onSubmit={form.handleSubmit(handleSubmit)} className="space-y-6">
        <FormField
          control={form.control}
          name="name"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Profile Name</FormLabel>
              <FormControl>
                <Input placeholder="E.g., Quick Ramp Up" {...field} />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />
        
        <FormField
          control={form.control}
          name="description"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Description</FormLabel>
              <FormControl>
                <Textarea 
                  placeholder="Describe the temperature profile"
                  className="resize-none"
                  {...field}
                />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />
        
        <FormField
          control={form.control}
          name="duration"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Duration (minutes)</FormLabel>
              <FormControl>
                <Input type="number" min={1} step={1} {...field} />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />
        
        <div className="space-y-2">
          <label className="text-sm font-medium">
            Temperature Curve
          </label>
          <BezierEditor
            controlPoints={controlPoints}
            onChange={setControlPoints}
            minTemp={tempRange.min}
            maxTemp={tempRange.max}
            duration={form.watch('duration')}
            onTempRangeChange={handleTempRangeChange}
            className="border rounded-md p-2"
          />
        </div>
        
        <Button type="submit" className="w-full">
          {initialData ? 'Update Profile' : 'Create Profile'}
        </Button>
      </form>
    </Form>
  );
};

export default ProfileForm;
