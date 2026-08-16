import { useState, useEffect } from 'react';
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { AlertCircle, CheckCircle } from 'lucide-react';
import { useModbus } from '@/contexts/ModbusContext';

interface QueueItemFlag {
  isUsed: boolean;
  isHighPriority: boolean;
  isInProgress: boolean;
  isBroadcast: boolean;
  isSynchronized: boolean;
}

interface QueueItem {
  index: number;
  token: number;
  slaveId: number;
  address: number;
  type: number;
  value: number;
  quantity: number;
  status: number;
  retries: number;
  timestamp: number;
  flags: QueueItemFlag;
}

interface QueueData {
  count: number;
  maxSize: number;
  queue: QueueItem[];
}

const QueueDisplay = () => {
  const { apiUrl } = useModbus();
  const [queueData, setQueueData] = useState<QueueData | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      if (!apiUrl) {
        setError("API URL is not configured.");
        return;
      }
      const fullUrl = `${apiUrl}/api/v1/modbus/rtu/queue`;
      try {
        const response = await fetch(fullUrl);
        if (!response.ok) {
          throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data: QueueData = await response.json();
        setQueueData(data);
        setError(null);
      } catch (e) {
        console.error("Failed to fetch queue data:", e);
        if (e instanceof Error) {
            setError(`Failed to fetch queue data from ${fullUrl}: ${e.message}`);
        } else {
            setError("An unknown error occurred while fetching queue data.");
        }
        setQueueData(null);
      }
    };

    fetchData();
    const intervalId = setInterval(fetchData, 5000);

    return () => clearInterval(intervalId);
  }, [apiUrl]);

  if (error) {
    return (
        <Card className="mt-4 border-destructive">
            <CardHeader>
                <CardTitle className="flex items-center text-destructive">
                    <AlertCircle className="mr-2"/> Error Loading Queue Data
                </CardTitle>
            </CardHeader>
            <CardContent>
                <p>{error}</p>
                <p>Please check if the backend service is running and accessible at the configured URL.</p>
            </CardContent>
        </Card>
    );
  }

  if (!queueData) {
    return <div>Loading queue data...</div>;
  }

  const getStatusIcon = (status: number) => {
    return status === 0 ? <CheckCircle className="text-green-500" /> : <AlertCircle className="text-yellow-500" />;
  };

  const getFlagTooltip = (flags: QueueItemFlag) => {
    return `Used: ${flags.isUsed}, HighPrio: ${flags.isHighPriority}, InProgress: ${flags.isInProgress}, Broadcast: ${flags.isBroadcast}, Synced: ${flags.isSynchronized}`;
  };


  return (
    <Card>
      <CardHeader>
        <CardTitle>Modbus RTU Queue</CardTitle>
        <p className="text-sm text-muted-foreground">
          Showing {queueData.count} items (Max Size: {queueData.maxSize})
        </p>
      </CardHeader>
      <CardContent>
        <Table>
          <TableHeader>
            <TableRow>
              <TableHead>Index</TableHead>
              <TableHead>Token</TableHead>
              <TableHead>Slave ID</TableHead>
              <TableHead>Type</TableHead>
              <TableHead>Address</TableHead>
              <TableHead>Quantity/Value</TableHead>
              <TableHead>Status</TableHead>
              <TableHead>Retries</TableHead>
              <TableHead>Timestamp</TableHead>
              <TableHead>Flags</TableHead>
            </TableRow>
          </TableHeader>
          <TableBody>
            {queueData.queue.map((item) => (
              <TableRow key={item.index}>
                <TableCell>{item.index}</TableCell>
                <TableCell>{item.token}</TableCell>
                <TableCell>{item.slaveId}</TableCell>
                <TableCell>{item.type}</TableCell>
                <TableCell>{item.address}</TableCell>
                <TableCell>{item.quantity > 0 ? item.quantity : item.value}</TableCell>
                <TableCell className="flex justify-center">{getStatusIcon(item.status)}</TableCell>
                <TableCell>{item.retries}</TableCell>
                <TableCell>{item.timestamp}</TableCell>
                <TableCell title={getFlagTooltip(item.flags)}>
                    {item.flags.isUsed ? 'U' : ''}
                    {item.flags.isHighPriority ? 'H' : ''}
                    {item.flags.isInProgress ? 'P' : ''}
                    {item.flags.isBroadcast ? 'B' : ''}
                    {item.flags.isSynchronized ? 'S' : ''}
                </TableCell>
              </TableRow>
            ))}
            {queueData.count === 0 && (
              <TableRow>
                <TableCell colSpan={10} className="text-center">Queue is empty</TableCell>
              </TableRow>
            )}
          </TableBody>
        </Table>
      </CardContent>
    </Card>
  );
};

export default QueueDisplay; 