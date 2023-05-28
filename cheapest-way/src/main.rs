use buf_read_splitter::BufReadSplitter;

fn read_safe<T, BufRead>(
    words_reader: &mut BufReadSplitter<BufRead>,
    value_name: &str
) -> T where
    T: std::str::FromStr,
    T::Err: std::fmt::Debug,
    BufRead: std::io::BufRead
{
    words_reader
        .next().expect(format!("failed to read {} value", value_name).as_str())
        .parse::<T>().expect(format!("failed to parse {} value", value_name).as_str())
}

fn main() {
    let mut words_reader = BufReadSplitter::new(
        std::io::stdin().lock()
    );

    let height: usize = read_safe(&mut words_reader, "height");
    let width: usize = read_safe(&mut words_reader, "width");
    // we use it to fill extra values to simplify algorithm
    let width = width + 1;

    let mut current_row = Vec::<u64>::new();
    current_row.resize(width, 0u64);
    for i in 1..width {
        let value: u64 = read_safe(&mut words_reader, "cell");
        current_row[i] = value + current_row[i-1];
    }
    // we need this extra value for right step from "nowhere"
    current_row[0] = u64::MAX;

    // we only need previous row for dynamic calculation of price
    let mut prev_row = current_row.clone();
    for _ in 1usize..height {
        prev_row.copy_from_slice(current_row.as_slice());
        for i in 1..width {
            let value: u64 = read_safe(&mut words_reader, "cell");
            current_row[i] = value + std::cmp::min(
                current_row[i-1],   // possible right step
                prev_row[i]         // possible down step
            );
        }
    };
    println!("{}", current_row.last().expect("failed to get cell values"));
}
