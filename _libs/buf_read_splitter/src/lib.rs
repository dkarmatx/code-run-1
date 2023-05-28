
pub struct BufReadSplitter<BufRead: std::io::BufRead> {
    buff: String,
    reader: BufRead,
}

impl<'a, BufRead: std::io::BufRead> BufReadSplitter<BufRead> {
    pub fn new(reader: BufRead) -> Self {
        return Self {
            buff: String::from(""),
            reader
        };
    }

    fn populate_buffer(&mut self) -> usize {
        let rbytes = self.reader
            .read_line(&mut self.buff)
            .expect("failed to read stdin");
        return rbytes;
    }
}

impl<BufRead: std::io::BufRead> Iterator for BufReadSplitter<BufRead> {
    type Item = String;

    fn next(&mut self) -> Option<Self::Item> {
        while !(self.buff.is_empty() && self.populate_buffer() == 0usize) {
            let trimmed = self.buff.trim();
            let (found, remainder) = trimmed
                .split_once(char::is_whitespace)
                .unwrap_or((trimmed, ""));
            let found = String::from(found);
            self.buff = String::from(remainder);
            if !found.is_empty() {
                return Some(found);
            }
        }
        return None;
    }
}
