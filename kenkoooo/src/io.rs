use std::{fs::read_to_string, path::Path};

use crate::{scanner::IO, types::Picture};

pub fn read_input<P: AsRef<Path>>(path: P) -> Picture {
    let input = read_to_string(path).unwrap();
    let mut sc = IO::new(input.as_bytes(), Vec::<u8>::new());

    let w: usize = sc.read();
    let h: usize = sc.read();

    let mut rgba = vec![vec![crate::types::RGBA([255; 4]); w]; h];
    for color in 0..4 {
        for i in 0..h {
            for j in 0..w {
                rgba[i][j].0[color] = sc.read();
            }
        }
    }

    Picture(rgba)
}
