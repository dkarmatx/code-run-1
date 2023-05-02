struct SplitWhitespacesReader {
    buff: String,
    reader: std::io::Stdin,
}

impl SplitWhitespacesReader {
    pub fn new(reader: std::io::Stdin) -> Self {
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

impl Iterator for SplitWhitespacesReader {
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

fn calculate_shortest_pin_path(pins: &std::collections::BTreeSet<u64>) -> Option<u64> {
    // creating distances list
    let mut distances = pins
        .iter()
        .zip(pins.iter().skip(1))
        .map(|(p1, p2)| {
                p2 - p1
            }
        );

    let (mut prev, mut current) = (
        u64::MAX,
        distances.next()?
    );

    for d in distances {
        (prev, current) = (
            current,
            std::cmp::min(prev, current) + d
        );
    }

    return Some(current);
}

fn main() {
    let mut reader = SplitWhitespacesReader::new(std::io::stdin());

    let size: usize = reader
        .next()
        .expect("nothing to read")
        .parse()
        .expect("failed to parse number of pins");

    let pins: std::collections::BTreeSet<u64> = reader
        .take(size)
        .map(|w| {
            w.parse().expect("failed to parse pin coordinates")})
        .collect();

    let result = calculate_shortest_pin_path(&pins)
        .expect("failed to solve");

    println!("{}", result);
}
