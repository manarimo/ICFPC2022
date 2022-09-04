use kenkoooo::{fast::FastState, io::read_input, parser::read_solution};

fn main() -> anyhow::Result<()> {
    let args = std::env::args().collect::<Vec<_>>();

    let (target, state) = read_input(&args[1])?;
    let moves = read_solution(&args[2])?;

    let mut state = FastState {
        global_counter: state.global_counter + 1,
        cost: 0,
        blocks: state
            .blocks
            .into_iter()
            .map(|(label, block)| (label.0, block))
            .collect(),
    };

    let map = state.apply_all(&moves)?;

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
    println!("{}", (similarity / 200.0).round() + state.cost as f64);

    Ok(())
}
