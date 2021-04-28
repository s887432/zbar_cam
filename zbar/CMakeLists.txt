add_library(zbarLib decoder/code39.c decoder/code93.c decoder/code128.c decoder/ean.c decoder/i25.c decoder/sq_finder.c decoder/qr_finder.c decoder/codabar.c decoder/databar.c qrcode/bch15_5.c qrcode/binarize.c qrcode/isaac.c qrcode/qrdec.c  qrcode/qrdectxt.c qrcode/rs.c qrcode/util.c config.c decoder.c error.c image.c img_scanner.c refcnt.c scanner.c symbol.c sqcode.c)

include_directories("${PROJECT_SOURCE_DIR}/zbar_lib/qrcode")
include_directories("${PROJECT_SOURCE_DIR}/zbar_lib/decode")


