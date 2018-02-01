inkscape flow.svg --export-id=id_commands --export-png=../bin/imgs/flow/24/commands.png -w 25
inkscape flow.svg --export-id=id_commands --export-png=../bin/imgs/flow/32/commands.png -w 34

inkscape flow.svg --export-id=id_trash --export-png=../bin/imgs/flow/24/trash.png -w 100
inkscape flow.svg --export-id=id_trash --export-png=../bin/imgs/flow/32/trash.png -w 140

inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/24/shapes.png -h 1024
inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/32/shapes.png -h 1024
inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/24/shapes_ns.png -h 1024
inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/32/shapes_ns.png -h 1024
inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/24/shapes_alt.png -h 1024
inkscape flow.svg --export-id=id_shapes --export-png=../bin/imgs/flow/32/shapes_alt.png -h 1024

./export.sh "flow" "24 32" "tb_colors tb_crop tb_fullscreen tb_run tb_style tb_zoom tb_close tb_comments tb_debug tb_help tb_save tb_sub"
