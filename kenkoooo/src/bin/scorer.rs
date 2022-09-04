use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    score::{range_raw_similarity, RawSimilarity},
    types::Block,
};

fn main() -> anyhow::Result<()> {
    let args = std::env::args().collect::<Vec<_>>();

    let (target, mut state) = read_input(&args[1])?;
    let moves = read_solution(&args[2])?;

    let mut raw_similarity = range_raw_similarity(
        &state.picture,
        &target,
        &Block {
            x1: 0,
            x2: target.width(),
            y1: 0,
            y2: target.height(),
        },
    );
    for mv in moves {
        if let Move::Color(c) = &mv {
            let block = state.get_block(&c.label);
            raw_similarity -= range_raw_similarity(&state.picture, &target, block);
        }
        state = state.apply(&mv)?;
        if let Move::Color(c) = &mv {
            let block = state.get_block(&c.label);
            raw_similarity += range_raw_similarity(&state.picture, &target, block);
        }
    }
    let score = raw_similarity.to_normalized_similarity() + state.cost;
    println!("{}", score);

    Ok(())
}
