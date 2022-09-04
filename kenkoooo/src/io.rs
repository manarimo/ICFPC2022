use std::fs::File;
use std::path::Path;

use anyhow::{anyhow, Result};
use image::GenericImageView;
use serde::Deserialize;

use crate::types::Picture;
use crate::types::{Block, Label, State, RGBA};

pub fn read_input(problem_id: &str) -> Result<(Picture, State)> {
    let (w, h, target) = read_picture(format!("../problem/original/{problem_id}.png"))?;
    let mut state = State::new(w, h);
    if let Ok(initial) = File::open(format!("../problem/original/{problem_id}.initial.json")) {
        let initial: InitialState = serde_json::from_reader(initial)?;
        for block in initial.blocks {
            let label = block
                .block_id
                .split(".")
                .map(|id| id.parse::<u32>().unwrap())
                .collect::<Vec<_>>();
            state.push_block(
                Label(label),
                Block {
                    x1: block.bottom_left[0],
                    x2: block.top_right[0],
                    y1: block.bottom_left[1],
                    y2: block.top_right[1],
                },
            );

            let initial_picture = read_picture(format!(
                "../problem/original_initial/{problem_id}.initial.png"
            ));
            for x in block.bottom_left[0]..block.top_right[0] {
                for y in block.bottom_left[1]..block.top_right[1] {
                    match (&block.png_bottom_left_point, &block.color) {
                        (Some([xx, yy]), None) => {
                            let (_, _, pic) = initial_picture
                                .as_ref()
                                .map_err(|e| anyhow!("initial picture: {:?}", e))?;
                            let dx = x - block.bottom_left[0];
                            let dy = y - block.bottom_left[1];

                            state.picture.0[y][x] = RGBA(pic.0[dy + yy][dx + xx].0);
                        }
                        (None, Some(color)) => {
                            state.picture.0[y][x] = RGBA(color.clone());
                        }
                        _ => return Err(anyhow!("invalid initial state")),
                    }
                }
            }

            state.global_counter += 1;
        }
        state.global_counter = state.global_counter.max(1) - 1;
    }

    Ok((target, state))
}

fn read_picture<P: AsRef<Path>>(path: P) -> Result<(usize, usize, Picture)> {
    let img = image::open(path)?;
    let w = img.width() as usize;
    let h = img.height() as usize;

    let mut rgba = vec![vec![RGBA([255; 4]); w]; h];
    for pixel in img.pixels() {
        let (x, y, p) = pixel;
        // 使いやすさ重視で上下反転させる
        rgba[h - 1 - y as usize][x as usize] = RGBA(p.0);
    }
    Ok((w, h, Picture(rgba)))
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct InitialState {
    width: usize,
    height: usize,
    blocks: Vec<InitialStateBlock>,
}
#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct InitialStateBlock {
    block_id: String,
    bottom_left: [usize; 2],
    top_right: [usize; 2],
    color: Option<[u8; 4]>,
    png_bottom_left_point: Option<[usize; 2]>,
}
