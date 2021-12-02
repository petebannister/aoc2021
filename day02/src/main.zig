const std = @import("std");
const fs = std.fs;
const meta = std.meta;

const Vec2 = meta.Vector(2, i32);

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
const CommandError = error{
    InvalidCommand
};
fn readCommand(reader: anytype) !?Vec2
{
    const streql = std.mem.eql;
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = std.mem.split(u8, line, " ");
        var cmd = iter.next() orelse return null;
        var n = try std.fmt.parseInt(i32, iter.next().?, 10);
        
        std.debug.print("command: {s}", .{cmd});
        if (streql(u8, cmd, "forward")) {
            return Vec2{n, 0};
        }
        if (streql(u8, cmd, "down")) {
            return Vec2{0, n};
        }
        if (streql(u8, cmd, "up")) {
            return Vec2{0, -n};
        }
        return CommandError.InvalidCommand;
    }
    return null;
}


pub fn main() anyerror!void {
    const stdout = std.io.getStdOut().writer();
    const fname = "input.txt";
    var part1 : i32 = 0;
    var part2 : i32 = 0;
    var pos = Vec2{ 0, 0 };
    var pos2 = Vec2{ 0, 0 };
    var aim: i32 = 0;
    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    const reader = std.io.bufferedReader(file.reader()).reader();
    while (try readCommand(reader)) |cmd| {
        try stdout.print("{d}, {d}\n", .{ cmd[0], cmd[1] });
        pos += cmd;
        aim += cmd[1];
        pos2[0] += cmd[0];
        pos2[1] += aim * cmd[0];
    }
    part1 = pos[0] * pos[1];
    part2 = pos2[0] * pos2[1];
    try stdout.print("part1 {d}\n", .{part1});
    try stdout.print("part2 {d}\n", .{part2});
}
