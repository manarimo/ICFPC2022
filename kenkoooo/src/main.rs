use std::{cmp::Reverse, collections::BinaryHeap};

use kenkoooo::{io::read_input, types::State};
use rand::{rngs::StdRng, Rng, SeedableRng};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();

    let (picture, state) = read_input(&args[1]);
    let mut heap = BinaryHeap::new();
    heap.push(Reverse(Sortable {
        score: state.calc_score(&picture),
        value: state,
    }));
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
