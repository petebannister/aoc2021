const std = @import("std");
const fs = std.fs;

pub fn main() anyerror!void {
    const stdout = std.io.getStdOut().writer();
    const fname = "input.txt";
    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    var buf: [1024]u8 = undefined;
    const reader = std.io.bufferedReader(file.reader()).reader();
    var count : i32 = 0;
    var last: i32 = 0x7fffffff; // to ignore first
    while (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        // Parse the line, copy any needed bytes due to shared buffer
        var value = try std.fmt.parseInt(i32, line, 10);
        if (value > last) {
            count += 1;
        }
        last = value;
    }
    try stdout.print("{d}\n", .{count});
}
