use buf_read_splitter::BufReadSplitter;

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
    let mut reader = BufReadSplitter::new(
        std::io::stdin().lock()
    );

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
