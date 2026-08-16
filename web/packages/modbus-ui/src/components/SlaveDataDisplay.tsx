import { useState, useEffect } from 'react';
import { useModbus } from '@/contexts/ModbusContext';
import { Accordion, AccordionContent, AccordionItem, AccordionTrigger } from "@/components/ui/accordion";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { AlertCircle, CheckCircle, Database } from 'lucide-react';
import { Badge } from "@/components/ui/badge";

interface DataFlags {
  isUsed: boolean;
  isSynchronized: boolean;
}

interface CoilInfo {
  index: number;
  address: number;
  value: boolean;
  lastUpdate: number; // Assuming timestamp
  flags: DataFlags;
}

// Assuming RegisterInfo structure is similar, add if needed based on full data
interface RegisterInfo {
 index: number;
 address: number;
 value: number; // Or Uint16Array/Buffer depending on API
 lastUpdate: number;
 flags: DataFlags;
}


interface SlaveData {
  slaveId: number;
  coils: CoilInfo[];
  registers: RegisterInfo[]; // Assuming registers exist, adjust if not
}

const SlaveDataDisplay = () => {
  const { apiUrl } = useModbus();
  const [slaveData, setSlaveData] = useState<SlaveData[] | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      if (!apiUrl) {
        setError("API URL is not configured.");
        setSlaveData(null);
        return;
      }
      
      const fullUrl = `${apiUrl}/api/v1/modbus/rtu/slavedata`;
      try {
        const response = await fetch(fullUrl);
        if (!response.ok) {
           const errorText = await response.text();
           throw new Error(`HTTP error! status: ${response.status} - ${errorText}`);
        }
        const data: SlaveData[] = await response.json();
        setSlaveData(data);
        setError(null);
      } catch (e) {
        console.error("Failed to fetch slave data:", e);
        if (e instanceof Error) {
            setError(`Failed to fetch slave data from ${fullUrl}: ${e.message}`);
        } else {
            setError("An unknown error occurred while fetching slave data.");
        }
        setSlaveData(null);
      }
    };

    fetchData();
    const intervalId = setInterval(fetchData, 5000); // Refresh every 5 seconds

    return () => clearInterval(intervalId);
  }, [apiUrl]);

  if (error) {
    return (
      <Card className="mt-4 border-destructive">
        <CardHeader>
          <CardTitle className="flex items-center text-destructive">
            <AlertCircle className="mr-2"/> Error Loading Slave Data
          </CardTitle>
        </CardHeader>
        <CardContent>
          <p>{error}</p>
          <p>Please check the API endpoint and connection.</p>
        </CardContent>
      </Card>
    );
  }

  if (!slaveData) {
    return <div>Loading slave data...</div>;
  }

  if (slaveData.length === 0) {
    return <div>No slave data available.</div>;
  }

  // Helper to render flags
  const renderFlags = (flags: DataFlags) => (
    <div className="flex space-x-1">
      {flags.isUsed && <Badge variant="secondary" title="Used">U</Badge>}
      {flags.isSynchronized && <Badge variant="outline" title="Synchronized">S</Badge>}
    </div>
  );

  return (
    <Card>
      <CardHeader>
        <CardTitle>Slave Data Overview</CardTitle>
         <p className="text-sm text-muted-foreground">
          Displaying data for {slaveData.length} slaves.
        </p>
      </CardHeader>
      <CardContent>
        <Accordion type="multiple" className="w-full">
          {slaveData.map((slave) => (
            <AccordionItem value={`slave-${slave.slaveId}`} key={slave.slaveId}>
              <AccordionTrigger>
                <div className="flex items-center space-x-2">
                    <Database className="w-4 h-4" />
                    <span>Slave ID: {slave.slaveId}</span>
                    <Badge variant="outline">{slave.coils?.length || 0} Coils</Badge>
                    <Badge variant="outline">{slave.registers?.length || 0} Registers</Badge>
                </div>
                </AccordionTrigger>
              <AccordionContent>
                {/* Coils Table */}
                {slave.coils && slave.coils.length > 0 && (
                  <div className="mb-4">
                    <h4 className="font-semibold mb-2">Coils</h4>
                    <Table>
                      <TableHeader>
                        <TableRow>
                          <TableHead>Index</TableHead>
                          <TableHead>Address</TableHead>
                          <TableHead>Value</TableHead>
                          <TableHead>Last Update</TableHead>
                          <TableHead>Flags</TableHead>
                        </TableRow>
                      </TableHeader>
                      <TableBody>
                        {slave.coils.filter(c => c.flags.isUsed).map((coil) => ( // Only show used coils
                          <TableRow key={`coil-${coil.index}`}>
                            <TableCell>{coil.index}</TableCell>
                            <TableCell>{coil.address}</TableCell>
                            <TableCell>
                                <Badge variant={coil.value ? "default" : "secondary"}>
                                    {coil.value ? 'ON' : 'OFF'}
                                 </Badge>
                            </TableCell>
                            <TableCell>{coil.lastUpdate}</TableCell> {/* Consider formatting timestamp */}
                            <TableCell>{renderFlags(coil.flags)}</TableCell>
                          </TableRow>
                        ))}
                         {slave.coils.filter(c => c.flags.isUsed).length === 0 && (
                           <TableRow><TableCell colSpan={5} className="text-center text-muted-foreground">No used coils</TableCell></TableRow>
                         )}
                      </TableBody>
                    </Table>
                  </div>
                )}

                {/* Registers Table - Assuming structure */}
                 {slave.registers && slave.registers.length > 0 && (
                  <div>
                    <h4 className="font-semibold mb-2">Registers</h4>
                    <Table>
                      <TableHeader>
                        <TableRow>
                          <TableHead>Index</TableHead>
                          <TableHead>Address</TableHead>
                          <TableHead>Value</TableHead>
                          <TableHead>Last Update</TableHead>
                          <TableHead>Flags</TableHead>
                        </TableRow>
                      </TableHeader>
                      <TableBody>
                        {slave.registers.filter(r => r.flags.isUsed).map((register) => ( // Only show used registers
                          <TableRow key={`reg-${register.index}`}>
                            <TableCell>{register.index}</TableCell>
                            <TableCell>{register.address}</TableCell>
                            <TableCell>{register.value}</TableCell>
                            <TableCell>{register.lastUpdate}</TableCell> {/* Consider formatting timestamp */}
                            <TableCell>{renderFlags(register.flags)}</TableCell>
                          </TableRow>
                        ))}
                        {slave.registers.filter(r => r.flags.isUsed).length === 0 && (
                           <TableRow><TableCell colSpan={5} className="text-center text-muted-foreground">No used registers</TableCell></TableRow>
                         )}
                      </TableBody>
                    </Table>
                  </div>
                )}

                 {(!slave.coils || slave.coils.filter(c => c.flags.isUsed).length === 0) &&
                  (!slave.registers || slave.registers.filter(r => r.flags.isUsed).length === 0) && (
                  <p className="text-muted-foreground">No used coils or registers for this slave.</p>
                 )}

              </AccordionContent>
            </AccordionItem>
          ))}
        </Accordion>
      </CardContent>
    </Card>
  );
};

export default SlaveDataDisplay; 