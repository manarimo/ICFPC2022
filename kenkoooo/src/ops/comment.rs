use std::fmt::Display;

#[derive(Clone)]
pub struct Comment {
    pub comment: String,
}
impl Display for Comment {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "#{}", self.comment)
    }
}