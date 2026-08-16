kbot-d --router2=openai --model=anthropic/claude-3.7-sonnet:th \
        --prompt=./scripts/stats.md \
        --each=./src/modbus/*.h \
        --include=../../src/config.h \
        --include=./src/NetworkComponent.h \
        --include2=./src/profiles/PlotBase.h \
        --wrap=meta \
        --mode=tools \
        --preferences=none \
        --disableTools=read_file,read_files,list_files,file_exists,modify_project_files \
        --tools="fs" \
        --filters=code \
        --globExtension=match-cpp

