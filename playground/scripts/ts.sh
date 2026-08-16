kbot-d --model=google/gemini-2.5-flash-preview:thinking \
    --prompt=./scripts/ts.md \
    --include=./src/modbus/*.h \
    --mode=completion --preferences=none \
    --dst=../web/client/src/modbus-types.ts \
    --filters=code
