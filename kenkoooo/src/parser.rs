use std::{fs::read_to_string, path::Path};

use crate::{
    ops::{
        color::Color,
        lcut::{LineCut, Orientation},
        merge::Merge,
        pcut::PointCut,
        swap::Swap,
        comment::Comment,
        Move,
    },
    types::{Label, RGBA},
};

pub fn read_solution<P: AsRef<Path>>(path: P) -> anyhow::Result<Vec<Move>> {
    let code = read_to_string(path)?;
    let moves = parse_code(&code);
    Ok(moves)
}

fn parse_code(code: &str) -> Vec<Move> {
    let mut moves = vec![];
    for line in code.split("\n") {
        if let Some(mv) = parse_single_code(line) {
            moves.push(mv);
        }
    }
    moves
}

pub fn parse_single_code(line: &str) -> Option<Move> {
    let line = line.trim().replace(" ", "").to_lowercase();
    if line.starts_with("#") {
        return Some(Move::Comment(Comment {comment: (&line[1..]).to_string()}));
    }
    if line.is_empty() {
        return None;
    }

    let mv = if line.starts_with("cut") {
        parse_cut_move(&line)
    } else if line.starts_with("color") {
        Move::Color(parse_color_move(&line))
    } else if line.starts_with("swap") {
        Move::Swap(parse_swap_move(&line))
    } else if line.starts_with("merge") {
        Move::Merge(parse_merge_move(&line))
    } else {
        unreachable!()
    };
    Some(mv)
}

fn parse_cut_move(mut remain: &str) -> Move {
    remain = &remain[3..];
    let result = parse_label(remain);
    remain = result.1;
    let label = result.0;

    let (next_remain, orientation) = try_parse_orientation(remain);
    remain = next_remain;
    match orientation {
        Some(orientation) => {
            let (line_number, _) = parse_line_number(remain);
            Move::LineCut(LineCut {
                label,
                orientation,
                pos: line_number,
            })
        }
        None => {
            let (point, _) = parse_bracket(remain);
            Move::PointCut(PointCut {
                label,
                x: point[0] as usize,
                y: point[1] as usize,
            })
        }
    }
}
fn parse_color_move(mut remain: &str) -> Color {
    remain = &remain[5..];
    let (label, next_remain) = parse_label(remain);
    remain = next_remain;
    let (color, _) = parse_color(remain);

    Color { label, color }
}
fn parse_swap_move(mut remain: &str) -> Swap {
    remain = &remain[4..];
    let (label1, next_remain) = parse_label(remain);
    remain = next_remain;
    let (label2, _) = parse_label(remain);
    Swap { label1, label2 }
}

fn parse_line_number(remain: &str) -> (usize, &str) {
    let (label, remain) = parse_bracket(remain);
    (label[0] as usize, remain)
}

fn parse_label(line: &str) -> (Label, &str) {
    let (label, remain) = parse_bracket(line);
    (Label(label), remain)
}

fn parse_merge_move(mut remain: &str) -> Merge {
    remain = &remain[5..];
    let (label1, next_remain) = parse_label(remain);
    remain = next_remain;
    let (label2, _) = parse_label(remain);
    Merge { label1, label2 }
}

fn try_parse_orientation(line: &str) -> (&str, Option<Orientation>) {
    if line.starts_with("[x]") {
        (&line[3..], Some(Orientation::X))
    } else if line.starts_with("[y]") {
        (&line[3..], Some(Orientation::Y))
    } else {
        (line, None)
    }
}

fn parse_bracket(line: &str) -> (Vec<u32>, &str) {
    let mut label = vec![];
    let mut buf = String::new();
    for (i, c) in line.chars().enumerate().skip(1) {
        match c {
            '.' | ',' => {
                label.push(buf.parse::<u32>().unwrap());
                buf = String::new();
            }
            ']' => {
                label.push(buf.parse::<u32>().unwrap());

                let remain = &line[(i + 1)..];
                return (label, remain);
            }
            _ => {
                buf.push(c);
            }
        }
    }
    unreachable!()
}

fn parse_color(line: &str) -> (RGBA, &str) {
    let (rgba, remain) = parse_bracket(line);
    (
        RGBA([rgba[0] as u8, rgba[1] as u8, rgba[2] as u8, rgba[3] as u8]),
        remain,
    )
}

#[cfg(test)]
mod tests {
    use super::parse_code;

    #[test]
    fn test_parser() {
        parse_code(
            r#"color [0] [255, 255, 255, 255]
        cut [0] [x] [8]
        color [0.1] [255, 255, 255, 255]
        merge [0.0] [0.1]
        cut [1] [x] [18]
        color [1.1] [255, 255, 255, 255]
        merge [1.0] [1.1]
        cut [2] [x] [28]
        color [2.1] [255, 255, 255, 255]
        cut [2.1] [y] [324]
        color [2.1.1] [251, 252, 253, 255]
        merge [2.1.0] [2.1.1]
        cut [3] [y] [349]
        color [3.1] [255, 255, 255, 255]
        merge [3.0] [3.1]
        merge [2.0] [4]
        cut [5] [x] [38]
        color [5.1] [255, 255, 255, 255]
        cut [5.1] [y] [232]
        color [5.1.1] [254, 254, 254, 255]
        merge [5.1.0] [5.1.1]
        cut [6] [y] [244]
        color [6.1] [255, 255, 255, 255]
        merge [6.0] [6.1]
        cut [7] [y] [324]
        color [7.1] [253, 254, 254, 255]
        merge [7.0] [7.1]
        cut [8] [y] [349]
        color [8.1] [255, 255, 255, 255]
        merge [8.0] [8.1]
        merge [5.0] [9]
        cut [10] [x] [48]
        color [10.1] [255, 255, 255, 255]
        cut [10.1] [y] [220]
        color [10.1.1] [229, 73, 232, 255]
        merge [10.1.0] [10.1.1]
        cut [11] [y] [244]
        color [11.1] [255, 255, 255, 255]
        merge [11.0] [11.1]
        merge [10.0] [12]
        cut [13] [x] [58]
        color [13.1] [255, 255, 255, 255]
        cut [13.1] [y] [232]
        color [13.1.1] [254, 254, 254, 255]
        merge [13.1.0] [13.1.1]
        cut [14] [y] [257]
        color [14.1] [255, 255, 255, 255]
        merge [14.0] [14.1]
        cut [15] [y] [287]
        color [15.1] [255, 222, 89, 255]
        merge [15.0] [15.1]
        cut [16] [y] [304]
        color [16.1] [255, 255, 255, 255]
        merge [16.0] [16.1]
        merge [13.0] [17]
        cut [18] [x] [68]
        color [18.1] [255, 255, 255, 255]
        cut [18.1] [y] [220]
        color [18.1.1] [229, 73, 232, 255]
        merge [18.1.0] [18.1.1]
        cut [19] [y] [244]
        color [19.1] [255, 255, 255, 255]
        merge [19.0] [19.1]
        cut [20] [y] [287]
        color [20.1] [255, 222, 89, 255]
        merge [20.0] [20.1]
        cut [21] [y] [304]
        color [21.1] [255, 255, 255, 255]
        merge [21.0] [21.1]
        cut [22] [y] [324]
        color [22.1] [92, 225, 230, 255]
        merge [22.0] [22.1]
        cut [23] [y] [349]
        color [23.1] [255, 255, 255, 255]
        merge [23.0] [23.1]
        merge [18.0] [24]
        cut [25] [x] [78]
        color [25.1] [255, 255, 255, 255]
        cut [25.1] [y] [220]
        color [25.1.1] [229, 73, 232, 255]
        merge [25.1.0] [25.1.1]
        cut [26] [y] [244]
        color [26.1] [255, 255, 255, 255]
        merge [26.0] [26.1]
        cut [27] [y] [287]
        color [27.1] [255, 222, 89, 255]
        merge [27.0] [27.1]
        cut [28] [y] [304]
        color [28.1] [255, 255, 255, 255]
        merge [28.0] [28.1]
        merge [25.0] [29]
        cut [30] [x] [88]
        color [30.1] [255, 255, 255, 255]
        cut [30.1] [y] [58]
        color [30.1.1] [16, 139, 228, 255]
        merge [30.1.0] [30.1.1]
        cut [31] [y] [78]
        color [31.1] [253, 254, 254, 255]
        merge [31.0] [31.1]
        cut [32] [y] [88]
        color [32.1] [255, 255, 255, 255]
        merge [32.0] [32.1]
        cut [33] [y] [220]
        color [33.1] [254, 254, 254, 255]
        merge [33.0] [33.1]
        cut [34] [y] [232]
        color [34.1] [217, 217, 217, 255]
        merge [34.0] [34.1]
        cut [35] [y] [244]
        color [35.1] [255, 255, 255, 255]
        merge [35.0] [35.1]
        cut [36] [y] [287]
        color [36.1] [255, 254, 253, 255]
        merge [36.0] [36.1]
        cut [37] [y] [304]
        color [37.1] [255, 255, 255, 255]
        merge [37.0] [37.1]
        merge [30.0] [38]
        cut [39] [x] [98]
        color [39.1] [255, 255, 255, 255]
        cut [39.1] [y] [68]
        color [39.1.1] [16, 139, 228, 255]
        merge [39.1.0] [39.1.1]
        cut [40] [y] [78]
        color [40.1] [255, 255, 255, 255]
        merge [40.0] [40.1]
        cut [41] [y] [220]
        color [41.1] [229, 73, 232, 255]
        merge [41.0] [41.1]
        cut [42] [y] [232]
        color [42.1] [254, 254, 254, 255]
        merge [42.0] [42.1]
        cut [43] [y] [244]
        color [43.1] [255, 255, 255, 255]
        merge [43.0] [43.1]
        cut [44] [y] [271]
        color [44.1] [255, 254, 253, 255]
        merge [44.0] [44.1]
        cut [45] [y] [304]
        color [45.1] [255, 255, 255, 255]
        merge [45.0] [45.1]
        cut [46] [y] [324]
        color [46.1] [254, 254, 254, 255]
        merge [46.0] [46.1]
        cut [47] [y] [349]
        color [47.1] [255, 255, 255, 255]
        merge [47.0] [47.1]
        merge [39.0] [48]
        cut [49] [x] [108]
        color [49.1] [255, 255, 255, 255]
        cut [49.1] [y] [58]
        color [49.1.1] [16, 139, 228, 255]
        merge [49.1.0] [49.1.1]
        cut [50] [y] [78]
        color [50.1] [252, 253, 254, 255]
        merge [50.0] [50.1]
        cut [51] [y] [88]
        color [51.1] [255, 255, 255, 255]
        merge [51.0] [51.1]
        cut [52] [y] [220]
        color [52.1] [229, 73, 232, 255]
        merge [52.0] [52.1]
        cut [53] [y] [244]
        color [53.1] [255, 255, 255, 255]
        merge [53.0] [53.1]
        cut [54] [y] [271]
        color [54.1] [255, 254, 253, 255]
        merge [54.0] [54.1]
        cut [55] [y] [304]
        color [55.1] [255, 255, 255, 255]
        merge [55.0] [55.1]
        merge [49.0] [56]
        cut [57] [x] [118]
        color [57.1] [255, 255, 255, 255]
        cut [57.1] [y] [58]
        color [57.1.1] [16, 139, 228, 255]
        merge [57.1.0] [57.1.1]
        cut [58] [y] [68]
        color [58.1] [254, 254, 254, 255]
        merge [58.0] [58.1]
        cut [59] [y] [78]
        color [59.1] [255, 255, 255, 255]
        merge [59.0] [59.1]
        cut [60] [y] [168]
        color [60.1] [30, 52, 118, 255]
        merge [60.0] [60.1]
        cut [61] [y] [188]
        color [61.1] [254, 254, 254, 255]
        merge [61.0] [61.1]
        cut [62] [y] [198]
        color [62.1] [255, 255, 255, 255]
        merge [62.0] [62.1]
        cut [63] [y] [220]
        color [63.1] [229, 73, 232, 255]
        merge [63.0] [63.1]
        cut [64] [y] [244]
        color [64.1] [255, 255, 255, 255]
        merge [64.0] [64.1]
        cut [65] [y] [271]
        color [65.1] [255, 222, 89, 255]
        merge [65.0] [65.1]
        cut [66] [y] [304]
        color [66.1] [255, 255, 255, 255]
        merge [66.0] [66.1]
        cut [67] [y] [324]
        color [67.1] [92, 225, 230, 255]
        merge [67.0] [67.1]
        cut [68] [y] [349]
        color [68.1] [255, 255, 255, 255]
        merge [68.0] [68.1]
        merge [57.0] [69]
        cut [70] [x] [128]
        color [70.1] [255, 255, 255, 255]
        cut [70.1] [y] [58]
        color [70.1.1] [16, 139, 228, 255]
        merge [70.1.0] [70.1.1]
        cut [71] [y] [78]
        color [71.1] [251, 252, 253, 255]
        merge [71.0] [71.1]
        cut [72] [y] [88]
        color [72.1] [255, 255, 255, 255]
        merge [72.0] [72.1]
        cut [73] [y] [168]
        color [73.1] [30, 52, 118, 255]
        merge [73.0] [73.1]
        cut [74] [y] [178]
        color [74.1] [255, 255, 255, 255]
        merge [74.0] [74.1]
        cut [75] [y] [188]
        color [75.1] [30, 52, 118, 255]
        merge [75.0] [75.1]
        cut [76] [y] [198]
        color [76.1] [255, 255, 255, 255]
        merge [76.0] [76.1]
        cut [77] [y] [232]
        color [77.1] [254, 254, 254, 255]
        merge [77.0] [77.1]
        cut [78] [y] [244]
        color [78.1] [255, 255, 255, 255]
        merge [78.0] [78.1]
        cut [79] [y] [271]
        color [79.1] [255, 254, 253, 255]
        merge [79.0] [79.1]
        cut [80] [y] [304]
        color [80.1] [255, 255, 255, 255]
        merge [80.0] [80.1]
        merge [70.0] [81]
        cut [82] [x] [138]
        color [82.1] [255, 255, 255, 255]
        cut [82.1] [y] [58]
        color [82.1.1] [16, 139, 228, 255]
        merge [82.1.0] [82.1.1]
        cut [83] [y] [78]
        color [83.1] [255, 255, 255, 255]
        merge [83.0] [83.1]
        cut [84] [y] [168]
        color [84.1] [217, 217, 217, 255]
        merge [84.0] [84.1]
        cut [85] [y] [178]
        color [85.1] [254, 254, 254, 255]
        merge [85.0] [85.1]
        cut [86] [y] [188]
        color [86.1] [255, 255, 255, 255]
        merge [86.0] [86.1]
        cut [87] [y] [287]
        color [87.1] [255, 254, 253, 255]
        merge [87.0] [87.1]
        cut [88] [y] [304]
        color [88.1] [255, 255, 255, 255]
        merge [88.0] [88.1]
        merge [82.0] [89]
        cut [90] [x] [148]
        color [90.1] [255, 255, 255, 255]
        cut [90.1] [y] [58]
        color [90.1.1] [254, 254, 254, 255]
        merge [90.1.0] [90.1.1]
        cut [91] [y] [78]
        color [91.1] [253, 254, 254, 255]
        merge [91.0] [91.1]
        cut [92] [y] [88]
        color [92.1] [255, 255, 255, 255]
        merge [92.0] [92.1]
        cut [93] [y] [168]
        color [93.1] [30, 52, 118, 255]
        merge [93.0] [93.1]
        cut [94] [y] [188]
        color [94.1] [255, 255, 255, 255]
        merge [94.0] [94.1]
        cut [95] [y] [220]
        color [95.1] [0, 128, 55, 255]
        merge [95.0] [95.1]
        cut [96] [y] [244]
        color [96.1] [253, 254, 254, 255]
        merge [96.0] [96.1]
        cut [97] [y] [257]
        color [97.1] [255, 255, 255, 255]
        merge [97.0] [97.1]
        cut [98] [y] [271]
        color [98.1] [255, 254, 253, 255]
        merge [98.0] [98.1]
        cut [99] [y] [304]
        color [99.1] [255, 255, 255, 255]
        merge [99.0] [99.1]
        merge [90.0] [100]
        cut [101] [x] [158]
        color [101.1] [255, 255, 255, 255]
        cut [101.1] [y] [58]
        color [101.1.1] [16, 139, 228, 255]
        merge [101.1.0] [101.1.1]
        cut [102] [y] [68]
        color [102.1] [251, 252, 253, 255]
        merge [102.0] [102.1]
        cut [103] [y] [78]
        color [103.1] [255, 255, 255, 255]
        merge [103.0] [103.1]
        cut [104] [y] [168]
        color [104.1] [30, 52, 118, 255]
        merge [104.0] [104.1]
        cut [105] [y] [188]
        color [105.1] [255, 255, 255, 255]
        merge [105.0] [105.1]
        cut [106] [y] [232]
        color [106.1] [0, 128, 55, 255]
        merge [106.0] [106.1]
        cut [107] [y] [244]
        color [107.1] [254, 254, 254, 255]
        merge [107.0] [107.1]
        cut [108] [y] [257]
        color [108.1] [255, 255, 255, 255]
        merge [108.0] [108.1]
        cut [109] [y] [271]
        color [109.1] [255, 254, 253, 255]
        merge [109.0] [109.1]
        cut [110] [y] [287]
        color [110.1] [255, 222, 89, 255]
        merge [110.0] [110.1]
        cut [111] [y] [304]
        color [111.1] [255, 255, 255, 255]
        merge [111.0] [111.1]
        cut [112] [y] [324]
        color [112.1] [252, 253, 254, 255]
        merge [112.0] [112.1]
        cut [113] [y] [349]
        color [113.1] [255, 255, 255, 255]
        merge [113.0] [113.1]
        merge [101.0] [114]
        cut [115] [x] [168]
        color [115.1] [255, 255, 255, 255]
        cut [115.1] [y] [58]
        color [115.1.1] [253, 254, 254, 255]
        merge [115.1.0] [115.1.1]
        cut [116] [y] [68]
        color [116.1] [16, 139, 228, 255]
        merge [116.0] [116.1]
        cut [117] [y] [78]
        color [117.1] [252, 253, 254, 255]
        merge [117.0] [117.1]
        cut [118] [y] [88]
        color [118.1] [255, 255, 255, 255]
        merge [118.0] [118.1]
        cut [119] [y] [168]
        color [119.1] [30, 52, 118, 255]
        merge [119.0] [119.1]
        cut [120] [y] [188]
        color [120.1] [255, 255, 255, 255]
        merge [120.0] [120.1]
        cut [121] [y] [220]
        color [121.1] [217, 217, 217, 255]
        merge [121.0] [121.1]
        cut [122] [y] [232]
        color [122.1] [251, 252, 253, 255]
        merge [122.0] [122.1]
        cut [123] [y] [244]
        color [123.1] [255, 255, 255, 255]
        merge [123.0] [123.1]
        cut [124] [y] [271]
        color [124.1] [255, 254, 253, 255]
        merge [124.0] [124.1]
        cut [125] [y] [304]
        color [125.1] [255, 255, 255, 255]
        merge [125.0] [125.1]
        cut [126] [y] [324]
        color [126.1] [254, 254, 254, 255]
        merge [126.0] [126.1]
        cut [127] [y] [349]
        color [127.1] [255, 255, 255, 255]
        merge [127.0] [127.1]
        merge [115.0] [128]
        cut [129] [x] [178]
        color [129.1] [255, 255, 255, 255]
        cut [129.1] [y] [58]
        color [129.1.1] [16, 139, 228, 255]
        merge [129.1.0] [129.1.1]
        cut [130] [y] [78]
        color [130.1] [251, 252, 253, 255]
        merge [130.0] [130.1]
        cut [131] [y] [88]
        color [131.1] [255, 255, 255, 255]
        merge [131.0] [131.1]
        cut [132] [y] [168]
        color [132.1] [254, 254, 254, 255]
        merge [132.0] [132.1]
        cut [133] [y] [178]
        color [133.1] [30, 52, 118, 255]
        merge [133.0] [133.1]
        cut [134] [y] [188]
        color [134.1] [255, 255, 255, 255]
        merge [134.0] [134.1]
        cut [135] [y] [220]
        color [135.1] [0, 128, 55, 255]
        merge [135.0] [135.1]
        cut [136] [y] [244]
        color [136.1] [255, 255, 255, 255]
        merge [136.0] [136.1]
        cut [137] [y] [324]
        color [137.1] [254, 254, 254, 255]
        merge [137.0] [137.1]
        cut [138] [y] [349]
        color [138.1] [255, 255, 255, 255]
        merge [138.0] [138.1]
        merge [129.0] [139]
        cut [140] [x] [188]
        color [140.1] [255, 255, 255, 255]
        cut [140.1] [y] [58]
        color [140.1.1] [92, 225, 230, 255]
        merge [140.1.0] [140.1.1]
        cut [141] [y] [68]
        color [141.1] [16, 139, 228, 255]
        merge [141.0] [141.1]
        cut [142] [y] [88]
        color [142.1] [255, 255, 255, 255]
        merge [142.0] [142.1]
        cut [143] [y] [168]
        color [143.1] [30, 52, 118, 255]
        merge [143.0] [143.1]
        cut [144] [y] [188]
        color [144.1] [255, 255, 255, 255]
        merge [144.0] [144.1]
        cut [145] [y] [220]
        color [145.1] [251, 252, 253, 255]
        merge [145.0] [145.1]
        cut [146] [y] [244]
        color [146.1] [255, 255, 255, 255]
        merge [146.0] [146.1]
        cut [147] [y] [287]
        color [147.1] [255, 254, 254, 255]
        merge [147.0] [147.1]
        cut [148] [y] [304]
        color [148.1] [255, 255, 255, 255]
        merge [148.0] [148.1]
        cut [149] [y] [324]
        color [149.1] [0, 74, 173, 255]
        merge [149.0] [149.1]
        cut [150] [y] [349]
        color [150.1] [255, 255, 255, 255]
        merge [150.0] [150.1]
        merge [140.0] [151]
        cut [152] [x] [198]
        color [152.1] [255, 255, 255, 255]
        cut [152.1] [y] [58]
        color [152.1.1] [254, 254, 254, 255]
        merge [152.1.0] [152.1.1]
        cut [153] [y] [78]
        color [153.1] [253, 254, 254, 255]
        merge [153.0] [153.1]
        cut [154] [y] [88]
        color [154.1] [255, 255, 255, 255]
        merge [154.0] [154.1]
        cut [155] [y] [168]
        color [155.1] [30, 52, 118, 255]
        merge [155.0] [155.1]
        cut [156] [y] [188]
        color [156.1] [251, 252, 253, 255]
        merge [156.0] [156.1]
        cut [157] [y] [198]
        color [157.1] [255, 255, 255, 255]
        merge [157.0] [157.1]
        cut [158] [y] [220]
        color [158.1] [253, 254, 254, 255]
        merge [158.0] [158.1]
        cut [159] [y] [232]
        color [159.1] [0, 128, 55, 255]
        merge [159.0] [159.1]
        cut [160] [y] [244]
        color [160.1] [255, 255, 255, 255]
        merge [160.0] [160.1]
        cut [161] [y] [271]
        color [161.1] [255, 254, 253, 255]
        merge [161.0] [161.1]
        cut [162] [y] [287]
        color [162.1] [255, 145, 77, 255]
        merge [162.0] [162.1]
        cut [163] [y] [304]
        color [163.1] [255, 255, 255, 255]
        merge [163.0] [163.1]
        cut [164] [y] [324]
        color [164.1] [253, 254, 254, 255]
        merge [164.0] [164.1]
        cut [165] [y] [349]
        color [165.1] [255, 255, 255, 255]
        merge [165.0] [165.1]
        merge [152.0] [166]
        cut [167] [x] [209]
        color [167.1] [255, 255, 255, 255]
        cut [167.1] [y] [168]
        color [167.1.1] [30, 52, 118, 255]
        merge [167.1.0] [167.1.1]
        cut [168] [y] [178]
        color [168.1] [217, 217, 217, 255]
        merge [168.0] [168.1]
        cut [169] [y] [188]
        color [169.1] [255, 255, 255, 255]
        merge [169.0] [169.1]
        cut [170] [y] [220]
        color [170.1] [254, 254, 254, 255]
        merge [170.0] [170.1]
        cut [171] [y] [232]
        color [171.1] [0, 128, 55, 255]
        merge [171.0] [171.1]
        cut [172] [y] [244]
        color [172.1] [255, 255, 255, 255]
        merge [172.0] [172.1]
        cut [173] [y] [287]
        color [173.1] [255, 254, 253, 255]
        merge [173.0] [173.1]
        cut [174] [y] [304]
        color [174.1] [255, 255, 255, 255]
        merge [174.0] [174.1]
        cut [175] [y] [324]
        color [175.1] [0, 74, 173, 255]
        merge [175.0] [175.1]
        cut [176] [y] [349]
        color [176.1] [255, 255, 255, 255]
        merge [176.0] [176.1]
        merge [167.0] [177]
        cut [178] [x] [220]
        color [178.1] [255, 255, 255, 255]
        cut [178.1] [y] [58]
        color [178.1.1] [252, 253, 254, 255]
        merge [178.1.0] [178.1.1]
        cut [179] [y] [68]
        color [179.1] [140, 82, 255, 255]
        merge [179.0] [179.1]
        cut [180] [y] [88]
        color [180.1] [255, 255, 255, 255]
        merge [180.0] [180.1]
        cut [181] [y] [168]
        color [181.1] [30, 52, 118, 255]
        merge [181.0] [181.1]
        cut [182] [y] [188]
        color [182.1] [255, 255, 255, 255]
        merge [182.0] [182.1]
        cut [183] [y] [220]
        color [183.1] [254, 254, 254, 255]
        merge [183.0] [183.1]
        cut [184] [y] [244]
        color [184.1] [255, 255, 255, 255]
        merge [184.0] [184.1]
        cut [185] [y] [271]
        color [185.1] [255, 254, 253, 255]
        merge [185.0] [185.1]
        cut [186] [y] [304]
        color [186.1] [255, 255, 255, 255]
        merge [186.0] [186.1]
        cut [187] [y] [324]
        color [187.1] [251, 252, 253, 255]
        merge [187.0] [187.1]
        cut [188] [y] [349]
        color [188.1] [255, 255, 255, 255]
        merge [188.0] [188.1]
        merge [178.0] [189]
        cut [190] [x] [232]
        color [190.1] [255, 255, 255, 255]
        cut [190.1] [y] [68]
        color [190.1.1] [254, 254, 255, 255]
        merge [190.1.0] [190.1.1]
        cut [191] [y] [78]
        color [191.1] [251, 252, 253, 255]
        merge [191.0] [191.1]
        cut [192] [y] [88]
        color [192.1] [255, 255, 255, 255]
        merge [192.0] [192.1]
        cut [193] [y] [168]
        color [193.1] [251, 252, 253, 255]
        merge [193.0] [193.1]
        cut [194] [y] [178]
        color [194.1] [30, 52, 118, 255]
        merge [194.0] [194.1]
        cut [195] [y] [188]
        color [195.1] [255, 255, 255, 255]
        merge [195.0] [195.1]
        cut [196] [y] [232]
        color [196.1] [254, 254, 254, 255]
        merge [196.0] [196.1]
        cut [197] [y] [244]
        color [197.1] [255, 255, 255, 255]
        merge [197.0] [197.1]
        cut [198] [y] [271]
        color [198.1] [255, 254, 254, 255]
        merge [198.0] [198.1]
        cut [199] [y] [287]
        color [199.1] [255, 254, 253, 255]
        merge [199.0] [199.1]
        cut [200] [y] [304]
        color [200.1] [255, 255, 255, 255]
        merge [200.0] [200.1]
        cut [201] [y] [324]
        color [201.1] [251, 252, 253, 255]
        merge [201.0] [201.1]
        cut [202] [y] [349]
        color [202.1] [255, 255, 255, 255]
        merge [202.0] [202.1]
        merge [190.0] [203]
        cut [204] [x] [244]
        color [204.1] [255, 255, 255, 255]
        cut [204.1] [y] [58]
        color [204.1.1] [140, 82, 255, 255]
        merge [204.1.0] [204.1.1]
        cut [205] [y] [68]
        color [205.1] [254, 254, 255, 255]
        merge [205.0] [205.1]
        cut [206] [y] [78]
        color [206.1] [255, 255, 255, 255]
        merge [206.0] [206.1]
        cut [207] [y] [168]
        color [207.1] [30, 52, 118, 255]
        merge [207.0] [207.1]
        cut [208] [y] [188]
        color [208.1] [255, 255, 255, 255]
        merge [208.0] [208.1]
        cut [209] [y] [220]
        color [209.1] [251, 252, 253, 255]
        merge [209.0] [209.1]
        cut [210] [y] [232]
        color [210.1] [217, 217, 217, 255]
        merge [210.0] [210.1]
        cut [211] [y] [244]
        color [211.1] [255, 255, 255, 255]
        merge [211.0] [211.1]
        cut [212] [y] [271]
        color [212.1] [255, 254, 253, 255]
        merge [212.0] [212.1]
        cut [213] [y] [304]
        color [213.1] [255, 255, 255, 255]
        merge [213.0] [213.1]
        cut [214] [y] [324]
        color [214.1] [251, 252, 253, 255]
        merge [214.0] [214.1]
        cut [215] [y] [349]
        color [215.1] [255, 255, 255, 255]
        merge [215.0] [215.1]
        merge [204.0] [216]
        cut [217] [x] [257]
        color [217.1] [255, 255, 255, 255]
        cut [217.1] [y] [58]
        color [217.1.1] [140, 82, 255, 255]
        merge [217.1.0] [217.1.1]
        cut [218] [y] [78]
        color [218.1] [254, 254, 255, 255]
        merge [218.0] [218.1]
        cut [219] [y] [88]
        color [219.1] [255, 255, 255, 255]
        merge [219.0] [219.1]
        cut [220] [y] [168]
        color [220.1] [30, 52, 118, 255]
        merge [220.0] [220.1]
        cut [221] [y] [188]
        color [221.1] [254, 254, 254, 255]
        merge [221.0] [221.1]
        cut [222] [y] [198]
        color [222.1] [255, 255, 255, 255]
        merge [222.0] [222.1]
        cut [223] [y] [220]
        color [223.1] [254, 254, 254, 255]
        merge [223.0] [223.1]
        cut [224] [y] [232]
        color [224.1] [217, 217, 217, 255]
        merge [224.0] [224.1]
        cut [225] [y] [244]
        color [225.1] [255, 255, 255, 255]
        merge [225.0] [225.1]
        cut [226] [y] [271]
        color [226.1] [255, 254, 254, 255]
        merge [226.0] [226.1]
        cut [227] [y] [287]
        color [227.1] [255, 254, 253, 255]
        merge [227.0] [227.1]
        cut [228] [y] [304]
        color [228.1] [255, 255, 255, 255]
        merge [228.0] [228.1]
        cut [229] [y] [324]
        color [229.1] [253, 254, 254, 255]
        merge [229.0] [229.1]
        cut [230] [y] [349]
        color [230.1] [255, 255, 255, 255]
        merge [230.0] [230.1]
        merge [217.0] [231]
        cut [232] [x] [271]
        color [232.1] [255, 255, 255, 255]
        cut [232.1] [y] [58]
        color [232.1.1] [252, 253, 254, 255]
        merge [232.1.0] [232.1.1]
        cut [233] [y] [68]
        color [233.1] [140, 82, 255, 255]
        merge [233.0] [233.1]
        cut [234] [y] [78]
        color [234.1] [254, 254, 255, 255]
        merge [234.0] [234.1]
        cut [235] [y] [88]
        color [235.1] [255, 255, 255, 255]
        merge [235.0] [235.1]
        cut [236] [y] [168]
        color [236.1] [254, 254, 254, 255]
        merge [236.0] [236.1]
        cut [237] [y] [178]
        color [237.1] [255, 255, 255, 255]
        merge [237.0] [237.1]
        cut [238] [y] [324]
        color [238.1] [251, 252, 253, 255]
        merge [238.0] [238.1]
        cut [239] [y] [349]
        color [239.1] [255, 255, 255, 255]
        merge [239.0] [239.1]
        merge [232.0] [240]
        cut [241] [x] [287]
        color [241.1] [255, 255, 255, 255]
        cut [241.1] [y] [58]
        color [241.1.1] [140, 82, 255, 255]
        merge [241.1.0] [241.1.1]
        cut [242] [y] [78]
        color [242.1] [254, 254, 255, 255]
        merge [242.0] [242.1]
        cut [243] [y] [88]
        color [243.1] [255, 255, 255, 255]
        merge [243.0] [243.1]
        cut [244] [y] [220]
        color [244.1] [255, 254, 254, 255]
        merge [244.0] [244.1]
        cut [245] [y] [232]
        color [245.1] [255, 254, 253, 255]
        merge [245.0] [245.1]
        cut [246] [y] [244]
        color [246.1] [255, 254, 254, 255]
        merge [246.0] [246.1]
        cut [247] [y] [257]
        color [247.1] [255, 255, 255, 255]
        merge [247.0] [247.1]
        cut [248] [y] [287]
        color [248.1] [16, 139, 228, 255]
        merge [248.0] [248.1]
        cut [249] [y] [304]
        color [249.1] [255, 255, 255, 255]
        merge [249.0] [249.1]
        cut [250] [y] [324]
        color [250.1] [252, 253, 254, 255]
        merge [250.0] [250.1]
        cut [251] [y] [349]
        color [251.1] [255, 255, 255, 255]
        merge [251.0] [251.1]
        merge [241.0] [252]
        cut [253] [x] [304]
        color [253.1] [255, 255, 255, 255]
        cut [253.1] [y] [232]
        color [253.1.1] [255, 254, 254, 255]
        merge [253.1.0] [253.1.1]
        cut [254] [y] [257]
        color [254.1] [255, 255, 255, 255]
        merge [254.0] [254.1]
        cut [255] [y] [271]
        color [255.1] [251, 252, 253, 255]
        merge [255.0] [255.1]
        cut [256] [y] [304]
        color [256.1] [255, 255, 255, 255]
        merge [256.0] [256.1]
        cut [257] [y] [324]
        color [257.1] [251, 252, 253, 255]
        merge [257.0] [257.1]
        cut [258] [y] [349]
        color [258.1] [255, 255, 255, 255]
        merge [258.0] [258.1]
        merge [253.0] [259]
        cut [260] [x] [324]
        color [260.1] [255, 255, 255, 255]
        cut [260.1] [y] [220]
        color [260.1.1] [255, 254, 253, 255]
        merge [260.1.0] [260.1.1]
        cut [261] [y] [232]
        color [261.1] [255, 87, 87, 255]
        merge [261.0] [261.1]
        cut [262] [y] [244]
        color [262.1] [255, 255, 255, 255]
        merge [262.0] [262.1]
        cut [263] [y] [287]
        color [263.1] [254, 254, 254, 255]
        merge [263.0] [263.1]
        cut [264] [y] [304]
        color [264.1] [255, 255, 255, 255]
        merge [264.0] [264.1]
        cut [265] [y] [324]
        color [265.1] [251, 252, 253, 255]
        merge [265.0] [265.1]
        cut [266] [y] [349]
        color [266.1] [255, 255, 255, 255]
        merge [266.0] [266.1]
        merge [260.0] [267]
        cut [268] [x] [349]
        color [268.1] [255, 255, 255, 255]
        merge [268.0] [268.1]
        "#,
        );
    }
}
