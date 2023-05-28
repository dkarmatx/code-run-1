use buf_read_splitter::BufReadSplitter;

fn read_safe<T, BufRead>(
    words_reader: &mut BufReadSplitter<BufRead>,
    value_name: &str
) -> T
where
    T: std::str::FromStr,
    <T as std::str::FromStr>::Err: std::fmt::Debug,
    BufRead: std::io::BufRead
{
    words_reader
        .next().expect(format!("failed to read {} value", value_name).as_str())
        .parse::<T>().expect(format!("failed to parse {} value", value_name).as_str())
}

fn main() {
    let mut words_reader: BufReadSplitter<_> = BufReadSplitter::new(
        std::io::stdin().lock()
    );

    let height: usize = read_safe(&mut words_reader, "height");
    let width: usize = read_safe(&mut words_reader, "width");
    // we use it to fill extra values to simplify algorithm
    let width = width + 1;
    let height = height + 1;

    // read map with zero filled borders for simpler algorithm
    let mut map = Vec::<Vec<u64>>::new();
    for j in 0..height {
        let mut row = Vec::<u64>::new();
        for i in 0..width {
            if i == 0 || j == 0 {
                row.push(0);
            } else {
                row.push(read_safe(&mut words_reader, "cell"));
            }
        }
        map.push(row);
    }

    // calculate max cost way
    for j in 1..height {
        for i in 1..width {
            map[j][i] = std::cmp::max(
                map[j-1][i],
                map[j][i-1]
            ) + map[j][i];
        }
    }

    // calculate max cost path
    let mut j = height - 1;
    let mut i = width - 1;
    let mut path = Vec::<char>::new();
    while !(j == 1 && i == 1) {
        let down_step = map[j-1][i];
        let right_step = map[j][i-1];
        path.push(if down_step > right_step {
            j -= 1;
            'D'
        } else {
            i -= 1;
            'R'
        });
    }

    // print the result
    println!("{}", map.last().unwrap().last().unwrap());
    path.iter().rev().for_each(|x| {
        print!("{}", x);
    });
    println!("");
}
