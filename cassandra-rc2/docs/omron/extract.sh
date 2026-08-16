kbot-d --router2=openai --model=google/gemini-2.5-pro-preview-03-25 \
        --prompt=./tests/pdf/omron/prompt.md \
        --each=./tests/pdf/omron/*.jpg \
        --mode=completion --preferences=none \
        --dst=./tests/pdf/omron/modbus.md \
        --filters=code --append=concat
