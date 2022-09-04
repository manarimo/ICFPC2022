use kenkoooo::{fast::FastState, io::read_input, ops::Move, parser::read_solution, types::Picture};

fn main() -> anyhow::Result<()> {
    let args = std::env::args().collect::<Vec<_>>();

    let problem_id = &args[1];
    let solution = &args[2];

    let (target, state) = read_input(problem_id)?;
    let mut moves = read_solution(solution)?;

    let initial_state = FastState {
        global_counter: state.global_counter + 1,
        cost: 0,
        blocks: state
            .blocks
            .into_iter()
            .map(|(label, block)| (label.0, block))
            .collect(),
    };

    let initial_state=

    Ok(())
}

fn calc_score(initial_state: &FastState, moves: &[Move], target: &Picture) -> Option<f64> {
    let mut state = initial_state.clone();
    let map = match state.apply_all(&moves) {
        Ok(map) => map,
        Err(_) => return None,
    };

    let mut similarity = 0.0;
    for (y, row) in map.into_iter().enumerate() {
        for (x, color) in row.into_iter().enumerate() {
            let mut sum = 0.0;
            for i in 0..4 {
                let a = target.0[y][x].0[i] as f64;
                let b = color[i] as f64;
                let d = a - b;
                sum += d * d;
            }
            similarity += sum.sqrt();
        }
    }
    Some(similarity)
}
