importPackage(Packages.com.sk89q.worldedit);
importPackage(Packages.com.sk89q.worldedit.math);
importPackage(Packages.com.sk89q.worldedit.blocks);

var road_type = {
    "小路陡路基": {
        base_material: [
            [
                0,
                "diamond_block",
                "stone",
                "stone",
                "stone",
                "stone",
            ],
            [
                -1,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ],

            [
                -2,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ],

            [
                -3,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ],

            [
                -4,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ],
            [
                -5,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ],
            [
                -6,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ], [
                -7,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
            ], [
                -8,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone", "stone"
            ], [
                -9,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "stone", "stone"
            ],
        ],
        contours: [
        ],
    },
    "小路路基": {
        base_material: [
            [
                0,
                "diamond_block",
                "stone",
                "stone",
                "stone",
                "dirt",
            ],
            [
                -1,
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "grass_block",
            ],

            [
                -2,
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
            ],

            [
                -3,
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "grass_block",
            ],

            [
                -4,
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "dirt",
            ],
            [
                -5,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "grass_block",
            ],
            [
                -6,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "dirt",
            ], [
                -7,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "dirt", "grass_block",
            ], [
                -8,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "dirt", "dirt",
            ], [
                -9,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "dirt",
                "dirt",
                "dirt", "dirt", "grass_block"
            ],
        ],
        contours: [
        ],
    },
    "小路": {
        base_material: [
            [
                0,
                "gray_terracotta#4;white_terracotta#4",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
            ],
            [-1, "yellow_wool"],
            [1, "air", "air", "air", "air", "air"],
            [2, "air", "air", "air", "air", "air"],
            [3, "air", "air", "air", "air", "air", "air"],
            [4, "air", "air", "air", "air", "air", "air"],
            [5, "air", "air", "air", "air", "air", "air"],
            [6, "air", "air", "air", "air", "air", "air", "air"],
            [7, "air", "air", "air", "air", "air", "air", "air"],

        ],
        contours: [
            [3, 0, "white_terracotta"],
            [4, 1, "=red_sandstone_wall#1;void#2"],
        ],
    },

    "虚线": {
        base_material: [
            [
                0,
                "gray_terracotta#4;void#4",
            ]
        ],
        contours: [
        ],
    },
    "双向二车道": {
        base_material: [
            [
                0,
                "yellow_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta"
            ]
        ],
        contours: [
            [4, 0, "white_terracotta#4;void#4"],
            [8, 0, "white_terracotta"],
            [0, 1, "=red_sandstone_wall#1;void#3"],
        ],
    },
    "桥": {
        base_material: [
            [
                0,
                "yellow_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "gray_terracotta",
                "void"
            ],
            [
                -1,
                "stone",
                "stone",
                "stone",
                "stone",
                "stone",
                "smooth_stone_slab[type=top]",
            ],
            [
                -2,
                "void",
                "void",
                "void",
                "stone,2;void,10",
                "stone,2;void,10",
            ],
            [
                -3,
                "void",
                "void",
                "void",
                "stone,2;void,10",
                "stone,2;void,10",
            ],
            [
                -4,
                "void",
                "void",
                "void",
                "stone,2;void,10",
                "stone,2;void,10",
            ]
        ],
        contours: [
            [4, 0, "white_terracotta"],
            [7, 1, "smooth_stone_slab[type=bottom]"],
            [7, 0, "stone"],
            [1, -2, "smooth_stone_slab[type=top]"],
        ],
    },
};

var config = {
    max_distance: 1000,
    max_width: 120,
};

var blocks = context.remember();
var session = context.getSession();
var player = context.getPlayer();

var loadConfigFromRegion = function (region, flag, symetry) {
    var x1 = region.getMinimumPoint().getX();
    var y1 = region.getMinimumPoint().getY();
    var z1 = region.getMinimumPoint().getZ();
    var x2 = region.getMaximumPoint().getX();
    var y2 = region.getMaximumPoint().getY();
    var z2 = region.getMaximumPoint().getZ();
    var locator;
    for (var x = x1 - 1; x <= x2 + 1; ++x) {
        for (var y = y1; y <= y2; y++) {
            for (var z = z1 - 1; z <= z2 + 1; ++z) {
                if ((x == x1 - 1 || x == x2 + 1 || z == z1 - 1 || z == z2 + 1) && String(blocks.getBlock(BlockVector3.at(x, y, z))).indexOf("minecraft:structure_block") == 0) {
                    if (!locator) {
                        locator = {
                            x: x, y: y, z: z,
                            pos: BlockVector3.at(x, y, z),
                            dir: x == x1 - 1 ? BlockVector3.at(1, 0, 0) : x == x2 + 1 ? BlockVector3.at(-1, 0, 0) : z == z1 - 1 ? BlockVector3.at(0, 0, 1) : z == z2 + 1 ? BlockVector3.at(0, 0, -1) : null
                        };
                    } else {
                        player.printError("意外的放样配置：选区边界发现多个结构方块定位器");
                        return null;
                    }
                }
            }
        }
    }
    if (!locator) {
        player.printError("意外的放样配置：找不到结构方块定位器");
        return;
    }
    var dealBaseSymetry = function (str, xz, lr) {
        if (!symetry) {
            if (!road_type.base_material[yidx][xz]) {
                road_type.base_material[yidx][xz] = "";
            }
            road_type.base_material[yidx][xz] += "{" + lr + "}" + str;
        } else
            road_type.base_material[yidx][xz] = str;
    }
    var road_type = { base_material: [], contours: [], segments: [], points: [], seg_num: null };
    //x
    if (locator.dir.getX() != 0) {
        road_type.seg_num = x2 - x1 + 1;
        var yidx = 0;

        for (var y = y1; y <= y2; y++, yidx++) {
            if (flag == "l") {
            } else if (flag == "s" || flag == "p") {
            } else {
                road_type.base_material.push([y - locator.y]);
            }
            for (var z = locator.z; z <= z2; ++z) {
                var arr = [];
                for (var x = x1; x <= x2; ++x) {
                    var block = String(blocks.getBlock(BlockVector3.at(x, y, z)));
                    if (block == "minecraft:air") block = "void";
                    if (block == "minecraft:structure_void") block = "air";
                    arr.push(block);
                    var prefix = (!symetry) ? (block == "void" ? "" : locator.dir.getX() > 0 ? "{l}" : "{r}") : "";
                    block = prefix + block;
                    if (flag == "s") {
                        road_type.segments.push([z - locator.z, y - locator.y, x - locator.x, block])
                    }
                    if (flag == "p") {
                        road_type.points.push([z - locator.z, y - locator.y, x - locator.x, block]);
                    }
                }
                var str;
                if (arr.length == 1) {
                    str = arr[0];
                } else {
                    str = arr.join(";");
                }
                if (flag == "l") {
                    road_type.contours.push([z - locator.z, y - locator.y, prefix + str]);
                } else if (flag == "s" || flag == "p") {
                } else {
                    dealBaseSymetry(str, z - locator.z + 1, locator.dir.getX() > 0 ? "l" : "r");
                }
            }
            for (var z = locator.z; z >= z1; z--) {
                var arr = [];
                for (var x = x1; x <= x2; ++x) {
                    var block = String(blocks.getBlock(BlockVector3.at(x, y, z)));
                    if (block == "minecraft:air") block = "void";
                    if (block == "minecraft:structure_void") block = "air";
                    arr.push(block);
                    var prefix = (!symetry) ? (block == "void" ? "" : locator.dir.getX() > 0 ? "{r}" : "{l}") : "";
                    block = prefix + block;
                    if (flag == "s") {
                        road_type.segments.push([locator.z - z, y - locator.y, x - locator.x, block]);
                    }
                    if (flag == "p") {
                        road_type.points.push([locator.z - z, y - locator.y, x - locator.x, block]);
                    }
                }
                var str;
                if (arr.length == 1) {
                    str = arr[0];
                } else {
                    str = arr.join(";");
                }
                if (flag == "l") {
                    road_type.contours.push([locator.z - z, y - locator.y, prefix + str]);
                } else if (flag == "s" || flag == "p") {
                } else {
                    dealBaseSymetry(str, locator.z - z + 1, locator.dir.getX() > 0 ? "r" : "l");
                }
            }
        }
    }
    //z
    if (locator.dir.getZ() != 0) {
        road_type.seg_num = z2 - z1 + 1;
        var yidx = 0;
        for (var y = y1; y <= y2; y++, yidx++) {
            road_type.base_material.push([y - locator.y]);
            for (var x = locator.x; x <= x2; ++x) {
                var arr = [];
                for (var z = z1; z <= z2; ++z) {
                    var block = String(blocks.getBlock(BlockVector3.at(x, y, z)));
                    if (block == "minecraft:air") block = "void";
                    if (block == "minecraft:structure_void") block = "air";
                    arr.push(block);
                    var prefix = (!symetry) ? (block == "void" ? "" : locator.dir.getZ() < 0 ? "{l}" : "{r}") : "";
                    block = prefix + block;
                    // if (!symetry) block = (block == "void" ? "" : locator.dir.getZ() < 0 ? "{l}" : "{r}") + block;
                    if (flag == "s") {
                        road_type.segments.push([x - locator.x, y - locator.y, z - locator.z, block]);
                    }
                    if (flag == "p") {
                        road_type.points.push([x - locator.x, y - locator.y, z - locator.z, block]);
                    }
                }
                var str;
                if (arr.length == 1) {
                    str = arr[0];
                } else {
                    str = arr.join(";");
                }
                if (flag == "l") {
                    road_type.contours.push([x - locator.x, y - locator.y, prefix + str]);
                } else if (flag == "s" || flag == "p") {
                } else {
                    dealBaseSymetry(str, x - locator.x + 1, locator.dir.getZ() < 0 ? "l" : "r");
                    // road_type.base_material[yidx][x - locator.x + 1] = str;
                }
            }
            for (var x = locator.x; x >= x1; x--) {
                var arr = [];
                for (var z = z1; z <= z2; ++z) {
                    var block = String(blocks.getBlock(BlockVector3.at(x, y, z)));
                    if (block == "minecraft:air") block = "void";
                    if (block == "minecraft:structure_void") block = "air";
                    arr.push(block);
                    var prefix = (!symetry) ? (block == "void" ? "" : locator.dir.getZ() < 0 ? "{r}" : "{l}") : "";
                    block = prefix + block;
                    // if (!symetry) block = (block == "void" ? "" : locator.dir.getZ() < 0 ? "{r}" : "{l}") + block;
                    if (flag == "s") {
                        road_type.segments.push([locator.x - x, y - locator.y, z - locator.z, block]);
                    } if (flag == "p") {
                        road_type.points.push([locator.x - x, y - locator.y, z - locator.z, block]);
                    }
                }
                var str;
                if (arr.length == 1) {
                    str = arr[0];
                } else {
                    str = arr.join(";");
                }
                if (flag == "l") {
                    road_type.contours.push([locator.x - x, y - locator.y, prefix + str]);
                } else if (flag == "s" || flag == "p") {
                } else {
                    dealBaseSymetry(str, locator.x - x + 1, locator.dir.getZ() < 0 ? "r" : "l");
                    // road_type.base_material[yidx][locator.x - x + 1] = str;
                }
            }
        }
    }
    // player.print(road_type.base_material.length);
    // player.print(road_type.contours.join(", "));
    return road_type;
};
// 深度优先算法获取路径 lines
var search_line = function (origin) {
    var line_blc_type = String(blocks.getBlock(origin));
    if (line_blc_type == "minecraft:air") {
        player.printError("请勿双脚离地！");
        return;
    }
    var lines = [];
    var lines_string = [];
    var is_online = function (pos) {
        if (lines_string.indexOf(String(pos)) != -1) return false;
        return String(blocks.getBlock(pos)) == line_blc_type;
    }
    var dx = 0, dy = 0, dz = 0;
    var total_length = 0;
    for (var i = 0; i < config.max_distance; ++i) {
        total_length += [0, 1, 1.4142, 1.732][Math.abs(dx) + Math.abs(dy) + Math.abs(dz)];
        lines.push({ pos: origin, distance: total_length });
        lines_string.push(String(origin));
        // up hill 直走
        if (is_online(origin.add(BlockVector3.at(1, 1, 0))) && !(dx == -1 && dy == -1 && dz == 0)) {
            dx = 1; dy = 1; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(-1, 1, 0))) && !(dx == 1 && dy == -1 && dz == 0)) {
            dx = -1; dy = 1; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(0, 1, 1))) && !(dx == 0 && dy == -1 && dz == -1)) {
            dx = 0; dy = 1; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(0, 1, -1))) && !(dx == 0 && dy == -1 && dz == 1)) {
            dx = 0; dy = 1; dz = -1;
            // 先把当前平面直角走完，不然上坡之前的对角会跳过直角方块
        } else if (is_online(origin.add(BlockVector3.at(1, 0, 0))) && !(dx == -1 && dy == 0 && dz == 0)) {
            dx = 1; dy = 0; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(-1, 0, 0))) && !(dx == 1 && dy == 0 && dz == 0)) {
            dx = -1; dy = 0; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(0, 0, 1))) && !(dx == 0 && dy == 0 && dz == -1)) {
            dx = 0; dy = 0; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(0, 0, -1))) && !(dx == 0 && dy == 0 && dz == 1)) {
            dx = 0; dy = 0; dz = -1;
            //up hill again
        } else if (is_online(origin.add(BlockVector3.at(1, 1, 1))) && !(dx == -1 && dy == -1 && dz == -1)) {
            dx = 1; dy = 1; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(-1, 1, -1))) && !(dx == 1 && dy == -1 && dz == 1)) {
            dx = -1; dy = 1; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(1, 1, -1))) && !(dx == -1 && dy == -1 && dz == 1)) {
            dx = 1; dy = 1; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(-1, 1, 1))) && !(dx == 1 && dy == -1 && dz == -1)) {
            dx = -1; dy = 1; dz = 1;
            // planar
        } else if (is_online(origin.add(BlockVector3.at(1, 0, 1))) && !(dx == -1 && dy == 0 && dz == -1)) {
            dx = 1; dy = 0; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(-1, 0, -1))) && !(dx == 1 && dy == 0 && dz == 1)) {
            dx = -1; dy = 0; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(1, 0, -1))) && !(dx == -1 && dy == 0 && dz == 1)) {
            dx = 1; dy = 0; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(-1, 0, 1))) && !(dx == 1 && dy == 0 && dz == -1)) {
            dx = -1; dy = 0; dz = 1;
            // down hill
        } else if (is_online(origin.add(BlockVector3.at(1, -1, 0))) && !(dx == -1 && dy == 1 && dz == 0)) {
            dx = 1; dy = -1; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(-1, -1, 0))) && !(dx == 1 && dy == 1 && dz == 0)) {
            dx = -1; dy = -1; dz = 0;
        } else if (is_online(origin.add(BlockVector3.at(0, -1, 1))) && !(dx == 0 && dy == 1 && dz == -1)) {
            dx = 0; dy = -1; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(0, -1, -1))) && !(dx == 0 && dy == 1 && dz == 1)) {
            dx = 0; dy = -1; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(1, -1, 1))) && !(dx == -1 && dy == 1 && dz == -1)) {
            dx = 1; dy = -1; dz = 1;
        } else if (is_online(origin.add(BlockVector3.at(-1, -1, -1))) && !(dx == 1 && dy == 1 && dz == 1)) {
            dx = -1; dy = -1; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(1, -1, -1))) && !(dx == -1 && dy == 1 && dz == 1)) {
            dx = 1; dy = -1; dz = -1;
        } else if (is_online(origin.add(BlockVector3.at(-1, -1, 1))) && !(dx == 1 && dy == 1 && dz == -1)) {
            dx = -1; dy = -1; dz = 1;
        } else {
            break;
        }
        origin = origin.add(BlockVector3.at(dx, dy, dz));
        if (is_online(origin.add(BlockVector3.at(0, -1, 0)))) {
            lines_string.push(String(origin.add(BlockVector3.at(0, -1, 0))));
        }
    }
    return lines;
}
//计算道路方块距离地图
var compute_roadmap = function (lines, max_width, seg_num, symetry) {
    //为了获取高质量的纵向参数化，需要将道路中线平滑处理
    for (var l = 1; l < lines.length - 1; l++) {
        var temp = lines[l - 1].pos.add(lines[l + 1].pos).add(lines[l].pos).add(lines[l].pos);
        lines[l].smooth_pos = Vector2.at(temp.getX() / 4, temp.getZ() / 4);
    }
    lines[0].smooth_pos = Vector2.at(lines[0].pos.getX(), lines[0].pos.getZ());
    lines[lines.length - 1].smooth_pos = Vector2.at(lines[lines.length - 1].pos.getX(), lines[lines.length - 1].pos.getZ());
    for (var it = 0; it < 50; it++) {
        for (var l = 1; l < lines.length - 1; l++) {
            lines[l].smooth_pos2 = lines[l - 1].smooth_pos.add(lines[l + 1].smooth_pos).add(lines[l].smooth_pos).add(lines[l].smooth_pos).divide(4.0);
        }
        for (var l = 1; l < lines.length - 1; l++) {
            lines[l].smooth_pos = lines[l].smooth_pos2;
        }
    }
    // 计算路径周围路面
    max_width = Math.min(config.max_width, max_width) + 1;
    var sqrt_table = {};
    for (var x = -max_width; x <= max_width; ++x) {
        for (var y = -max_width; y <= max_width; y++) {
            sqrt_table[x + "," + y] = Math.sqrt(x * x + y * y);
        }
    }
    var ranges = {};
    for (var l = 0; l < lines.length; l++) {
        var ox = lines[l].pos.getX();
        var oz = lines[l].pos.getZ();
        var dx = lines[l].smooth_pos ? lines[l].smooth_pos.getX() - ox : 0;
        var dz = lines[l].smooth_pos ? lines[l].smooth_pos.getZ() - oz : 0;
        // player.printError([dx, dz]);
        for (var x = -max_width; x <= max_width; ++x) {
            for (var z = -max_width; z <= max_width; ++z) {
                var index = (ox + x) + "," + (oz + z);
                var distance2 = (x - dx) * (x - dx) + (z - dz) * (z - dz);
                var distance = sqrt_table[x + "," + z];
                if (!ranges[index]) {
                    ranges[index] = {
                        distance2: distance2,
                        distance: distance,
                        pos: [ox + x, oz + z],
                        closest: l
                    }
                } else {
                    if (ranges[index].distance2 > distance2) {
                        ranges[index].distance2 = distance2;
                        ranges[index].closest = l;
                    }
                    if (ranges[index].distance > distance) {
                        ranges[index].distance = distance;
                        // ranges[index].closest = l;
                    }
                }
            }
        }
    }
    for (var index in ranges) {
        var distance = ranges[index].distance;
        if (distance > max_width + 1) {
            delete ranges[index];
        } else if (!symetry) {
            var l_idx = ranges[index].closest;
            var l_o = lines[l_idx].pos;
            var p_o = ranges[index].pos;
            var dl;
            if (l_idx == 0) {
                dl = lines[l_idx + 1].smooth_pos.subtract(lines[l_idx].smooth_pos);
            } else {
                dl = lines[l_idx].smooth_pos.subtract(lines[l_idx - 1].smooth_pos);
            }
            var z_axis = Vector3.at(p_o[0] - l_o.getX(), 0, p_o[1] - l_o.getZ()).cross(Vector3.at(dl.getX(), 0, dl.getZ()));
            if (z_axis.getY() >= 0) {
                ranges[index].l = true;
            }
            if (z_axis.getY() <= 0) {
                ranges[index].r = true;
            }
        }
    }
    var loftStart = (loftRange && loftRange[2] != "") ? (loftRange[2] - 1) || 0 : 0;
    var loftEnd = (loftRange && loftRange[3] != "") ? (loftRange[3] - 1) || (lines.length - 1) : (lines.length - 1);
    // player.print(loftStart)
    // player.print(loftEnd)
    for (var index in ranges) {
        // 计算路径周围的contour方块
        var distance = ranges[index].distance;
        var l_index = ranges[index].closest;
        if (l_index <= loftStart || l_index >= loftEnd) { continue; }
        var pos = ranges[index].pos;
        var neighbors = [
            ranges[(pos[0] + 1) + "," + pos[1]] ? ranges[(pos[0] + 1) + "," + pos[1]].distance : Infinity,
            ranges[(pos[0] - 1) + "," + pos[1]] ? ranges[(pos[0] - 1) + "," + pos[1]].distance : Infinity,
            ranges[pos[0] + "," + (pos[1] + 1)] ? ranges[pos[0] + "," + (pos[1] + 1)].distance : Infinity,
            ranges[pos[0] + "," + (pos[1] - 1)] ? ranges[pos[0] + "," + (pos[1] - 1)].distance : Infinity
        ];
        for (var ic = Math.floor(distance); ic < max_width; ic++) {
            var dic = ic + 0.8;
            if (distance < dic && (neighbors[0] > dic || neighbors[1] > dic || neighbors[2] > dic || neighbors[3] > dic)) {
                ranges[index]["contour-" + ic] = true;
            }
        }
        // 计算路径周围的segments方块
        if (road_type.seg_norm1) {
            neighbors = [
                ranges[(pos[0] + 1) + "," + pos[1]] ? ranges[(pos[0] + 1) + "," + pos[1]].closest : -Infinity,
                ranges[(pos[0] - 1) + "," + pos[1]] ? ranges[(pos[0] - 1) + "," + pos[1]].closest : -Infinity,
                ranges[pos[0] + "," + (pos[1] + 1)] ? ranges[pos[0] + "," + (pos[1] + 1)].closest : -Infinity,
                ranges[pos[0] + "," + (pos[1] - 1)] ? ranges[pos[0] + "," + (pos[1] - 1)].closest : -Infinity
            ];
            for (var ic = l_index; ic <= l_index + 1; ic++) {
                if ((neighbors[0] > ic || neighbors[1] > ic || neighbors[2] > ic || neighbors[3] > ic)) {
                    ranges[index]["segment-" + (ic % seg_num)] = true;
                }
            }
        } else {
            neighbors = [
                ranges[(pos[0] + 1) + "," + pos[1]] ? Math.floor(lines[ranges[(pos[0] + 1) + "," + pos[1]].closest].distance) : -Infinity,
                ranges[(pos[0] - 1) + "," + pos[1]] ? Math.floor(lines[ranges[(pos[0] - 1) + "," + pos[1]].closest].distance) : -Infinity,
                ranges[pos[0] + "," + (pos[1] + 1)] ? Math.floor(lines[ranges[pos[0] + "," + (pos[1] + 1)].closest].distance) : -Infinity,
                ranges[pos[0] + "," + (pos[1] - 1)] ? Math.floor(lines[ranges[pos[0] + "," + (pos[1] - 1)].closest].distance) : -Infinity
            ];
            for (var ic = Math.floor(lines[l_index].distance); ic <= Math.floor(lines[l_index].distance) + 1; ic++) {
                if ((neighbors[0] > ic || neighbors[1] > ic || neighbors[2] > ic || neighbors[3] > ic)) {
                    ranges[index]["segment-" + (ic % seg_num)] = true;
                }
            }
        }
    }
    return ranges;
}
var parse_and_set_block_patern = function (pos, str, l_index, line, symetry, map_blc) {
    var norm1 = false;
    if (segnorm1) norm1 = true;
    if (symetry === false) {
        var sliceR = str.split("{r}").pop().split("{l}")[0];
        var sliceL = str.split("{l}").pop().split("{r}")[0];
        if (map_blc.r == true) parse_and_set_block_patern(pos, sliceR, l_index, line, true);
        if (map_blc.l == true) parse_and_set_block_patern(pos, sliceL, l_index, line, true);
        return;
    } else {
        if (str[0] == "=") {
            norm1 = true;
            str = str.slice(1);
        }
        if (str.split(";").length > 1) {
            var arr = str.split(";");
            var period = 0;
            var pattern = [];
            arr.forEach(function (e) {
                var info = e.split("#");
                var blc_type = info[0];
                var count = Number(info[1]) || 1;
                period += count;
                for (var i = 0; i < count; ++i) {
                    pattern.push(blc_type == "void" ? null : context.getBlock(blc_type));
                }
            });
            if (norm1) {
                var bloc = pattern[l_index % period];
                if (bloc) {
                    blocks.setBlock(pos, bloc);
                    add2needFixTable(pos, bloc);
                }
            } else {
                var bloc = pattern[Math.floor(line[l_index].distance) % period];
                if (bloc) {
                    blocks.setBlock(pos, bloc);
                    add2needFixTable(pos, bloc);
                }
            }
        } else if (str && str != "void") {
            var block = context.getBlock(str);
            blocks.setBlock(pos, block);
            add2needFixTable(pos, block);
        }
    }
}

//填充方块
var build_road = function (line, roadmap, roadtype, symetry) {
    var loftStart = (loftRange && loftRange[2] != "") ? (loftRange[2] - 1) || 0 : 0;
    var loftEnd = (loftRange && loftRange[3] != "") ? (loftRange[3] - 1) || (line.length - 1) : (line.length - 1);
    for (var index in roadmap) {
        var map_blc = roadmap[index];
        var distance = map_blc.distance;
        var l_index = map_blc.closest;
        var pos = map_blc.pos;
        var roadY = line[l_index].pos.getY();
        roadtype.base_material.forEach(function (layer) {
            if (layer[Math.floor(distance) + 1] && l_index > loftStart && l_index < loftEnd) {
                parse_and_set_block_patern(
                    BlockVector3.at(pos[0], roadY + layer[0], pos[1]),
                    layer[Math.floor(distance) + 1],
                    l_index,
                    line,
                    symetry,
                    map_blc
                );
            }
        });
        roadtype.contours.forEach(function (contour) {
            if (map_blc["contour-" + contour[0]] === true) {
                parse_and_set_block_patern(
                    BlockVector3.at(pos[0], roadY + contour[1], pos[1]),
                    contour[2],
                    l_index,
                    line,
                    symetry,
                    map_blc
                );
            }
        });
        roadtype.segments.forEach(function (seg) {
            if (map_blc["segment-" + seg[2]] === true && Math.floor(distance) == seg[0]) {
                parse_and_set_block_patern(
                    BlockVector3.at(pos[0], roadY + seg[1], pos[1]),
                    seg[3],
                    l_index,
                    line,
                    symetry,
                    map_blc
                );
            }
        });
        roadtype.points.forEach(function (p) {
            var sflag = "segment-" + p[2];
            if (map_blc[sflag] === true && (p[0] - distance <= 0 && p[0] - distance >= -3)) {
                var neighbors = [
                    roadmap[(pos[0] + 1) + "," + pos[1]] && roadmap[(pos[0] + 1) + "," + pos[1]][sflag] ? roadmap[(pos[0] + 1) + "," + pos[1]].distance : Infinity,
                    roadmap[(pos[0] - 1) + "," + pos[1]] && roadmap[(pos[0] - 1) + "," + pos[1]][sflag] ? roadmap[(pos[0] - 1) + "," + pos[1]].distance : Infinity,
                    roadmap[pos[0] + "," + (pos[1] + 1)] && roadmap[pos[0] + "," + (pos[1] + 1)][sflag] ? roadmap[pos[0] + "," + (pos[1] + 1)].distance : Infinity,
                    roadmap[pos[0] + "," + (pos[1] - 1)] && roadmap[pos[0] + "," + (pos[1] - 1)][sflag] ? roadmap[pos[0] + "," + (pos[1] - 1)].distance : Infinity,
                    roadmap[(pos[0] + 1) + "," + (pos[1] - 1)] && roadmap[(pos[0] + 1) + "," + (pos[1] - 1)][sflag] ? roadmap[(pos[0] + 1) + "," + (pos[1] - 1)].distance : Infinity,
                    roadmap[(pos[0] - 1) + "," + (pos[1] + 1)] && roadmap[(pos[0] - 1) + "," + (pos[1] + 1)][sflag] ? roadmap[(pos[0] - 1) + "," + (pos[1] + 1)].distance : Infinity,
                    roadmap[(pos[0] + 1) + "," + (pos[1] + 1)] && roadmap[(pos[0] + 1) + "," + (pos[1] + 1)][sflag] ? roadmap[(pos[0] + 1) + "," + (pos[1] + 1)].distance : Infinity,
                    roadmap[(pos[0] - 1) + "," + (pos[1] - 1)] && roadmap[(pos[0] - 1) + "," + (pos[1] - 1)][sflag] ? roadmap[(pos[0] - 1) + "," + (pos[1] - 1)].distance : Infinity
                ];
                if ((distance == 0 && p[0] == 0) || p[0] > neighbors[0] || p[0] > neighbors[1] || p[0] > neighbors[2] || p[0] > neighbors[3] || p[0] > neighbors[4] || p[0] > neighbors[5] || p[0] > neighbors[6] || p[0] > neighbors[7]) {
                    parse_and_set_block_patern(
                        BlockVector3.at(pos[0], roadY + p[1], pos[1]),
                        p[3],
                        l_index,
                        line,
                        symetry,
                        map_blc
                    );
                }
            }
        });
    }
}
var needFixTable = {};
var add2needFixTable = function (pos, block) {
    if (!autofix) return;

    var err = false;
    try {
        block.getState(block.getBlockType().getProperty("west"));
    } catch (e) {
        err = true;
        return;
    }
    if (!err) needFixTable[String(pos)] = { block: block, pos: pos };
}
var east = BlockVector3.at(1, 0, 0);
var west = BlockVector3.at(-1, 0, 0);
var north = BlockVector3.at(0, 0, -1);
var south = BlockVector3.at(0, 0, 1);
var fix = function () {
    for (var blc in needFixTable) {
        var block = needFixTable[blc];
        var nb = block.block;
        if (needFixTable[String(block.pos.add(east))]) {
            nb = nb["with"](nb.getBlockType().getProperty("east"), true);
        } else {
            nb = nb["with"](nb.getBlockType().getProperty("east"), false);
        }

        if (needFixTable[String(block.pos.add(west))]) {
            nb = nb["with"](nb.getBlockType().getProperty("west"), true);
        } else {
            nb = nb["with"](nb.getBlockType().getProperty("west"), false);
        }

        if (needFixTable[String(block.pos.add(north))]) {
            nb = nb["with"](nb.getBlockType().getProperty("north"), true);
        } else {
            nb = nb["with"](nb.getBlockType().getProperty("north"), false);
        }

        if (needFixTable[String(block.pos.add(south))]) {
            nb = nb["with"](nb.getBlockType().getProperty("south"), true);
        } else {
            nb = nb["with"](nb.getBlockType().getProperty("south"), false);
        }
        blocks.setBlock(block.pos, nb);

    }
}
context.checkArgs(1, 5, "<road_type> [start:end] [-ns(nosymetry)] [-nf(nofix)] [-n1(norm1)]");
var road_type;
var symetry = !(argv[2] == "-ns" || argv[3] == "-ns" || argv[4] == "-ns" || argv[5] == "-ns");
var autofix = !(argv[2] == "-nf" || argv[3] == "-nf" || argv[4] == "-nf" || argv[5] == "-nf");
var segnorm1 = (argv[2] == "-n1" || argv[3] == "-n1" || argv[4] == "-n1" || argv[5] == "-n1");
var loftRange = /^(d?)([0-9]*)\:([0-9]*)$/.exec(argv[2]);
if (!loftRange) loftRange = /^(d?)([0-9]*)\:([0-9]*)$/.exec(argv[3]);
if (!loftRange) loftRange = /^(d?)([0-9]*)\:([0-9]*)$/.exec(argv[4]);
if (!loftRange) loftRange = /^(d?)([0-9]*)\:([0-9]*)$/.exec(argv[5]);
if (loftRange) player.print(loftRange);
// player.print(symetry)
if (argv[1] == "sel" || argv[1] == "s") {
    var region = session.getRegionSelector(player.getWorld()).getRegion();
    road_type = loadConfigFromRegion(region, null, symetry);
} else if (argv[1] == "linesel" || argv[1] == "ls") {
    var region = session.getRegionSelector(player.getWorld()).getRegion();
    road_type = loadConfigFromRegion(region, "l", symetry);
} else if (argv[1] == "segsel" || argv[1] == "ss") {
    var region = session.getRegionSelector(player.getWorld()).getRegion();
    road_type = loadConfigFromRegion(region, "s", symetry);
} else if (argv[1] == "pointsel" || argv[1] == "ps") {
    var region = session.getRegionSelector(player.getWorld()).getRegion();
    road_type = loadConfigFromRegion(region, "p", symetry);
} else {
    road_type = road_type[argv[1]];
}
if (!road_type) {
    player.printError("未知的道路种类！");
} else {
    if (!road_type.base_material) {
        road_type.base_material = [];
    }
    if (!road_type.contours) {
        road_type.contours = [];
    }
    if (!road_type.segments) {
        road_type.segments = [];
    }
    if (!road_type.seg_num) {
        road_type.seg_num = 4;
    }
    if (!road_type.points) {
        road_type.points = [];
    }
    var max_width = 0;
    if (road_type.seg_num[0] == "=") {
        road_type.seg_norm1 = true;
        road_type.seg_num = road_type.seg_num.slice(1);
    } else {
        road_type.seg_norm1 = segnorm1;
    }
    road_type.base_material.forEach(function (e) {
        max_width = Math.max(max_width, e.length - 2);
    });
    road_type.contours.forEach(function (e) {
        max_width = Math.max(max_width, e[0]);
    });
    road_type.segments.forEach(function (e) {
        e[2] = Math.abs(e[2]) % road_type.seg_num;
        max_width = Math.max(max_width, e[0]);
    });
    road_type.points.forEach(function (e) {
        e[2] = Math.abs(e[2]) % road_type.seg_num;
        max_width = Math.max(max_width, e[0]);
    });
    road_type.contours = road_type.contours.filter(function (e) {
        return e[2] != "void";
    });
    road_type.segments = road_type.segments.filter(function (e) {
        return e[3] != "void";
    });
    road_type.points = road_type.points.filter(function (e) {
        return e[3] != "void";
    });

    road_type.seg_num = Number(road_type.seg_num);
    var start = player.getBlockOn().toVector().toBlockPoint();
    if (loftRange && loftRange[1] == "d") start = start.add(BlockVector3(0, -1, 0));
    var central_line = search_line(start);
    player.print("路径总长" + central_line.length + "个方块");
    if (central_line) {
        var roadmap = compute_roadmap(central_line, max_width, road_type.seg_num, symetry);
        build_road(central_line, roadmap, road_type, symetry);
    }
    if (autofix) {
        fix();
    }
}
player.print(road_type.points.join("\n"));