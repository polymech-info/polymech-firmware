kbot-d --router2=openai --model=google/gemini-2.5-pro-preview-03-25 \
        --prompt=./tests/pdf/omron/convert.md \
        --each=./tests/pdf/omron/*.jpg \
        --include=./tests/pdf/omron/omron.h \
        --mode=completion --preferences=none \
        --dst=./tests/pdf/omron/omron.h \
        --filters=code
