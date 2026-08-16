kbot-d --model=google/gemini-2.5-flash-preview:thinking \
    --prompt=./scripts/ts-commons.md \
    --include=./src/modbus/ModbusTypes.cpp \
    --mode=completion --preferences=none \
    --dst=../web/client/src/modbus-commons.ts \
    --filters=code
