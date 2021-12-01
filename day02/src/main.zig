const std = @import("std");
const fs = std.fs;

// Example test
// const expect = @import("std").testing.expect;
// test "if statement" {
//     const a = true;
//     var x: u16 = 0;
//     if (a) {
//         x += 1;
//     } else {
//         x += 2;
//     }
//     try expect(x == 1);
// }

pub fn main() anyerror!void {
    const stdout = std.io.getStdOut().writer();
    const fname = "input.txt";
    var buf: [1024]u8 = undefined;
    var part1 : i32 = 0;
    var part2 : i32 = 0;
    var index : u32 = 0;
    var last: i32 = 0;
    var last_sum: i32 = 0;
    var window = [3]i32 {0,0,0};

    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    const reader = std.io.bufferedReader(file.reader()).reader();
    while (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var value = try std.fmt.parseInt(i32, line, 10);
        if (value > last) {
            if (index > 0) {
                part1 += 1;
            }
        }
        window[index % window.len] = value;
        last = value;
        index += 1;
        if (index >= window.len) {
            var sum : i32 = 0;
            for (window) |v| {
                sum += v;
            }
            if (index > window.len) {
                if (sum > last_sum) {
                    part2 += 1;
                }
            }
            last_sum = sum;
        }
    }
    try stdout.print("part1 {d}\n", .{part1});
    try stdout.print("part2 {d}\n", .{part2});
}
