kbot-d --router2=openai --model=anthropic/claude-3.7-sonnet:th \
        --prompt=./scripts/docs.md \
        --each=./src/components/*.h \
        --include=../../src/config.h \
        --wrap=meta \
        --mode=tools --preferences=none \
        --disableTools=read_file,read_files,list_files,file_exists,modify_project_files \
        --tools="fs" \
        --filters=code

