kbot-d --router2=openai --model=anthropic/claude-3.7-sonnet:th \
        --prompt=./scripts/clean.md \
        --each=./src/modbus/*.h \
        --wrap=meta \
        --mode=tools --preferences=none \
        --disableTools=read_file,read_files,list_files \
        --tools="fs" \
        --filters=code

