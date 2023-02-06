astyle \
--options=Gateway/ubxlib/astyle.cfg \
--suffix=none \
--verbose \
--errors-to-stdout \
--recursive Gateway/src/*.c,*.h \
--recursive sensor_broadcaster/src/*.c,*.h \
$1 $2 $3 # addtional args such as --dry-run etc.