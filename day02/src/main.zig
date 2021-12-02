const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;

const Vec2 = meta.Vector(2, i32);

fn streql(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}
fn split(s: []const u8, delim: []const u8) std.mem.SplitIterator(u8) {
    return std.mem.split(u8, s, delim);
}
fn parseI32(s: []const u8) std.fmt.ParseIntError!i32 {
    return std.fmt.parseInt(i32, s, 10);
}

const ParseError = error {
    InvalidCommand
};
fn parseLine(reader: anytype) !?Vec2
{
    const lut = std.ComptimeStringMap(Vec2, .{
        .{ "forward", Vec2{1, 0}},
        .{ "down", Vec2{0, 1}},
        .{ "up", Vec2{0, -1}}
    });
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = split(line, " ");
        var cmd = iter.next() orelse return null;
        var n = try parseI32(iter.next().?);
        if (lut.get(cmd)) |v| {
            return v * @splat(2, n);
        }
        return ParseError.InvalidCommand;
    }
    return null;
}
fn solve(reader : anytype) !void
{
    var pos = Vec2{ 0, 0 };
    var pos2 = Vec2{ 0, 0 };
    var aim: i32 = 0;
    while (try parseLine(reader)) |cmd| {
        //std.debug.print("{d}, {d}\n", .{ cmd[0], cmd[1] });
        pos += cmd;
        aim += cmd[1];
        pos2[0] += cmd[0];
        pos2[1] += aim * cmd[0];
    }
    var part1 = pos[0] * pos[1];
    var part2 = pos2[0] * pos2[1];
    std.debug.print("part1 {d}\n", .{part1});
    std.debug.print("part2 {d}\n", .{part2});
    assert(part1 == 2102357);
    assert(part2 == 2101031224);
}

pub fn main() anyerror!void {
    const fname = "input.txt";
    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    const reader = std.io.bufferedReader(file.reader()).reader();
    try solve(reader);
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
