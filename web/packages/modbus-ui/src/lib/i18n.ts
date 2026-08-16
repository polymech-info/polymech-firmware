import { ESignalType } from "@/types";

export const getControlPointTypeNames = (translate: (key: string) => string): Record<number, string> => ({
    [ESignalType.NONE]: translate('No Operation'),
    [ESignalType.MB_WRITE_COIL]: translate('Write Coil'),
    [ESignalType.MB_WRITE_HOLDING_REGISTER]: translate('Write Holding Register'),
    [ESignalType.CALL_METHOD]: translate('Call Method'),
    [ESignalType.CALL_FUNCTION]: translate('Call Function'),
    [ESignalType.CALL_REST]: translate('Call REST API'),
    [ESignalType.GPIO_WRITE]: translate('Write GPIO'),
    [ESignalType.DISPLAY_MESSAGE]: translate('Display Message'),
    [ESignalType.USER_DEFINED]: translate('User Defined'),
    [ESignalType.PAUSE_PROFILE]: translate('Pause Profile'),
    [ESignalType.START_PIDS]: translate('Start PID Controllers'),
    [ESignalType.STOP_PIDS]: translate('Stop PID Controllers'),
    [ESignalType.BUZZER_OFF]: translate('Buzzer: Off'),
    [ESignalType.BUZZER_SOLID]: translate('Buzzer: Solid On'),
    [ESignalType.BUZZER_SLOW_BLINK]: translate('Buzzer: Slow Blink'),
    [ESignalType.BUZZER_FAST_BLINK]: translate('Buzzer: Fast Blink'),
    [ESignalType.BUZZER_LONG_BEEP_SHORT_PAUSE]: translate('Buzzer: Long Beep/Short Pause'),
    [ESignalType.IFTTT_WEBHOOK]: translate('Send IFTTT Notification'),
}); 