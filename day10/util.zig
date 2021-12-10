const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;
const print = std.debug.print;

const ArrayList = std.ArrayList;

pub const Error = error {
    Parse
};

pub const Solution = struct {
    part1: u64 = 0,
    part2: u64 = 0,
    pub fn print(self: *Solution, prefix: [] const u8) void {

        std.debug.print("{s}1 {d}\n", .{prefix, self.part1});
        std.debug.print("{s}2 {d}\n", .{prefix, self.part2});

    }
};

pub const Vec2 = meta.Vector(2, i32);

pub fn streql(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}
pub fn split(s: []const u8, delim: []const u8) std.mem.SplitIterator(u8) {
    return std.mem.split(u8, s, delim);
}
pub fn parseI32(s: []const u8) std.fmt.ParseIntError!i32 {
    return std.fmt.parseInt(i32, s, 10);
}
pub fn parseU8(s: []const u8) std.fmt.ParseIntError!u8 {
    return std.fmt.parseInt(u8, s, 10);
}
pub fn reduce(
    comptime T: type, 
    range: anytype, 
    comptime f: fn(a: anytype, b: anytype) T
) T
{
    var result: T = 0;
    switch (@typeInfo(@TypeOf(range))) {
        .Array => {
            for (range) |item| {
                result = f(result, item);
            }
        },
        .Struct => {
            if (@hasField(@TypeOf(range), "items")) {
                for (range.items) |item| {
                    result = f(result, item);
                }
            }
            else {
                @compileError("reduce: incompatible type");
            }
        },
        else => @compileError("reduce: incompatible type"),
    }
    return result;
}

fn add (a: anytype, b: anytype) @TypeOf(a + b) {
    return a + b;
}

pub fn sum(comptime T: type, range: anytype) T
{
    return reduce(T, range, add);
}


pub fn solveFile(fname : [] const u8, comptime f: fn(reader : anytype) anyerror!Solution) !Solution
{
    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    const reader = std.io.bufferedReader(file.reader()).reader();
    return f(reader);
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
