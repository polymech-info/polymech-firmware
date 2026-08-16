kbot-d --model=google/gemini-2.5-pro-preview-03-25 \
        --prompt=./prompt.md \
        --each=./*.jpg \
        --mode=completion --preferences=none \
        --include=./registers.h \
        --dst=./registers.h \
        --filters=code
