use std::{cmp::Reverse, collections::BinaryHeap};

pub struct MinHeap<S, V> {
    inner: BinaryHeap<Reverse<HeapEntry<S, V>>>,
}

struct HeapEntry<S, V> {
    score: S,
    value: V,
}

impl<S, V> MinHeap<S, V>
where
    S: PartialOrd,
{
    pub fn push(&mut self, score: S, value: V) {
        self.inner.push(Reverse(HeapEntry { score, value }));
    }
    pub fn pop(&mut self) -> Option<V> {
        self.inner.pop().map(|e| e.0.value)
    }
}

impl<S, V> Ord for HeapEntry<S, V>
where
    S: PartialOrd,
{
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.partial_cmp(other).unwrap()
    }
}

impl<S, V> PartialOrd for HeapEntry<S, V>
where
    S: PartialOrd,
{
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        self.score.partial_cmp(&other.score)
    }
}

impl<S, V> Eq for HeapEntry<S, V> where S: PartialOrd {}
impl<S, V> PartialEq for HeapEntry<S, V>
where
    S: PartialOrd,
{
    fn eq(&self, other: &Self) -> bool {
        self.score == other.score
    }
}
