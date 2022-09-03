use std::{cmp::Reverse, collections::BinaryHeap};

use kenkoooo::{io::read_input, types::State};
use rand::{rngs::StdRng, Rng, SeedableRng};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();

    let picture = read_input(&args[1]);
    let state = State::new(picture.width(), picture.height());
    let mut heap = BinaryHeap::new();
    heap.push(Reverse(Sortable {
        score: state.calc_score(&picture),
        value: state,
    }));

    let mut rng = StdRng::seed_from_u64(181);
    loop {
        let mut next = BinaryHeap::new();
        for _ in 0..30 {
            let state = match heap.pop() {
                Some(Reverse(Sortable {
                    score: _,
                    value: state,
                })) => state,
                _ => {
                    break;
                }
            };

            for _ in 0..1000 {
                match rng.gen::<u32>() % 3 {
                    0 => {
                        // lcut
                    }
                    1 => {
                        //
                    }
                    2 => {}
                    _ => unimplemented!(),
                }
            }
        }
    }
}

struct Sortable<S, V> {
    score: S,
    value: V,
}

impl<S, V> PartialOrd for Sortable<S, V>
where
    S: PartialOrd,
{
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        self.score.partial_cmp(&other.score)
    }
}

impl<S, V> PartialEq for Sortable<S, V>
where
    S: PartialEq,
{
    fn eq(&self, other: &Self) -> bool {
        self.score == other.score
    }
}

impl<S, V> Ord for Sortable<S, V>
where
    S: PartialOrd,
{
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.partial_cmp(other).unwrap()
    }
}

impl<S, V> Eq for Sortable<S, V> where S: PartialEq {}
