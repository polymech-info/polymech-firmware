Import("env")
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    "xtensa-esp32s3-elf-strip -g -S -x $TARGET"
)
