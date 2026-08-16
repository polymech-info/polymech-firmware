
import React, { useState, useEffect } from 'react';
import MainNav from '@/components/MainNav';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardDescription, CardFooter, CardHeader, CardTitle } from '@/components/ui/card';
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from '@/components/ui/table';
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs';
import { Trash2, Edit, Plus } from 'lucide-react';
import { useToast } from '@/hooks/use-toast';
import { api, HeatZone } from '@/lib/api';

import { 
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog';

import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";

import {
  Form,
  FormControl,
  FormDescription,
  FormField,
  FormItem,
  FormLabel,
  FormMessage,
} from "@/components/ui/form";

import { Input } from "@/components/ui/input";
import { Textarea } from "@/components/ui/textarea";
import { useForm } from "react-hook-form";
import { z } from "zod";
import { zodResolver } from "@hookform/resolvers/zod";

// Form schema for creating a zone
const zoneFormSchema = z.object({
  name: z.string().min(1, "Zone name is required"),
  description: z.string().optional(),
});

type ZoneFormValues = z.infer<typeof zoneFormSchema>;

const ZoneSettings = () => {
  const { toast } = useToast();
  const [zones, setZones] = useState<HeatZone[]>([]);
  const [deleteZoneId, setDeleteZoneId] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [isCreateDialogOpen, setIsCreateDialogOpen] = useState(false);

  // Form for creating a new zone
  const form = useForm<ZoneFormValues>({
    resolver: zodResolver(zoneFormSchema),
    defaultValues: {
      name: "",
      description: "",
    },
  });

  useEffect(() => {
    loadZones();
  }, []);

  const loadZones = async () => {
    try {
      setLoading(true);
      const zoneData = await api.getZones();
      setZones(zoneData);
      setError(null);
    } catch (err) {
      setError('Failed to load zones');
      console.error('Error loading zones:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleDeleteZone = async () => {
    if (!deleteZoneId) return;
    
    try {
      await api.deleteZone(deleteZoneId);
      
      toast({
        title: "Zone Deleted",
        description: "The zone has been successfully removed",
        variant: "default"
      });
      
      // Refresh the zones list
      loadZones();
    } catch (error: any) {
      toast({
        title: "Error",
        description: error.message || "Failed to delete zone",
        variant: "destructive"
      });
    } finally {
      setDeleteZoneId(null);
    }
  };

  const handleCreateZone = async (data: ZoneFormValues) => {
    try {
      await api.createZone({
        name: data.name,
        description: data.description,
      });
      
      toast({
        title: "Zone Created",
        description: "The new zone has been successfully created",
        variant: "default"
      });
      
      // Reset form
      form.reset();
      
      // Close dialog
      setIsCreateDialogOpen(false);
      
      // Refresh zones list
      loadZones();
    } catch (error: any) {
      toast({
        title: "Error",
        description: error.message || "Failed to create zone",
        variant: "destructive"
      });
    }
  };

  return (
    <div className="container py-6 space-y-6">
      <MainNav />
      
      <div>
        <h1 className="text-2xl font-bold">Zone Settings</h1>
        <p className="text-muted-foreground">Manage your heat zones</p>
      </div>
      
      <Tabs defaultValue="zones">
        <TabsList>
          <TabsTrigger value="zones">Zones</TabsTrigger>
        </TabsList>
        
        <TabsContent value="zones" className="space-y-4">
          <Card>
            <CardHeader>
              <CardTitle>Heat Zones</CardTitle>
              <CardDescription>View and manage your heating zones</CardDescription>
            </CardHeader>
            <CardContent>
              {loading ? (
                <div className="py-4 text-center">Loading zones...</div>
              ) : error ? (
                <div className="py-4 text-center text-destructive">{error}</div>
              ) : zones.length === 0 ? (
                <div className="py-4 text-center">No zones found. Create a zone to get started.</div>
              ) : (
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead>Name</TableHead>
                      <TableHead>Description</TableHead>
                      <TableHead className="w-[100px]">Actions</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    {zones.map((zone) => (
                      <TableRow key={zone.id}>
                        <TableCell className="font-medium">{zone.name}</TableCell>
                        <TableCell>{zone.description || '-'}</TableCell>
                        <TableCell>
                          <div className="flex gap-2">
                            <Button 
                              variant="outline" 
                              size="icon"
                              onClick={() => {
                                // For future implementation of edit functionality
                                toast({
                                  title: "Edit Feature",
                                  description: "Edit functionality will be implemented in a future update",
                                });
                              }}
                            >
                              <Edit className="h-4 w-4" />
                            </Button>
                            <Button 
                              variant="outline" 
                              size="icon"
                              className="text-destructive hover:bg-destructive hover:text-destructive-foreground"
                              onClick={() => setDeleteZoneId(zone.id)}
                            >
                              <Trash2 className="h-4 w-4" />
                            </Button>
                          </div>
                        </TableCell>
                      </TableRow>
                    ))}
                  </TableBody>
                </Table>
              )}
            </CardContent>
            <CardFooter>
              <Button
                onClick={() => setIsCreateDialogOpen(true)}
                className="ml-auto"
              >
                <Plus className="mr-2 h-4 w-4" /> Create New Zone
              </Button>
            </CardFooter>
          </Card>
        </TabsContent>
      </Tabs>
      
      {/* Create Zone Dialog */}
      <Dialog open={isCreateDialogOpen} onOpenChange={setIsCreateDialogOpen}>
        <DialogContent>
          <DialogHeader>
            <DialogTitle>Create New Zone</DialogTitle>
            <DialogDescription>
              Add a new heating zone to your system.
            </DialogDescription>
          </DialogHeader>
          
          <Form {...form}>
            <form onSubmit={form.handleSubmit(handleCreateZone)} className="space-y-4">
              <FormField
                control={form.control}
                name="name"
                render={({ field }) => (
                  <FormItem>
                    <FormLabel>Zone Name</FormLabel>
                    <FormControl>
                      <Input placeholder="Living Room" {...field} />
                    </FormControl>
                    <FormDescription>
                      A descriptive name for your heating zone
                    </FormDescription>
                    <FormMessage />
                  </FormItem>
                )}
              />
              
              <FormField
                control={form.control}
                name="description"
                render={({ field }) => (
                  <FormItem>
                    <FormLabel>Description (Optional)</FormLabel>
                    <FormControl>
                      <Textarea 
                        placeholder="North side of the house, includes kitchen and dining area" 
                        {...field} 
                      />
                    </FormControl>
                    <FormDescription>
                      Provide additional details about this zone
                    </FormDescription>
                    <FormMessage />
                  </FormItem>
                )}
              />
              
              <DialogFooter>
                <Button type="button" variant="outline" onClick={() => setIsCreateDialogOpen(false)}>
                  Cancel
                </Button>
                <Button type="submit">Create Zone</Button>
              </DialogFooter>
            </form>
          </Form>
        </DialogContent>
      </Dialog>
      
      {/* Delete Zone Alert Dialog */}
      <AlertDialog open={!!deleteZoneId} onOpenChange={(open) => !open && setDeleteZoneId(null)}>
        <AlertDialogContent>
          <AlertDialogHeader>
            <AlertDialogTitle>Delete Zone?</AlertDialogTitle>
            <AlertDialogDescription>
              This will permanently delete this zone. This action cannot be undone.
            </AlertDialogDescription>
          </AlertDialogHeader>
          <AlertDialogFooter>
            <AlertDialogCancel>Cancel</AlertDialogCancel>
            <AlertDialogAction 
              onClick={handleDeleteZone}
              className="bg-destructive text-destructive-foreground"
            >
              Delete Zone
            </AlertDialogAction>
          </AlertDialogFooter>
        </AlertDialogContent>
      </AlertDialog>
    </div>
  );
};

export default ZoneSettings;
