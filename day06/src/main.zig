const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;

const AocError = error {
    ParseError
};

const Solution = struct {
    part1: u32 = 0,
    part2: u32 = 0,
};

//const Allocator = std.mem.Allocator;
const Population = meta.Vector(9, u32);

fn parseOne(reader: anytype) !?u8
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, ',')) |field| {
        var num = std.mem.trim(u8, field, "\r\n");
        return std.fmt.parseUnsigned(u8, num, 10) catch return null;
    }
    return null;
}

fn iterate(p: *Population, n: u32) u32
{
    var i = n;
    while (i > 0) : (i -= 1) {
        var last = p.*;
        var next = @shuffle(u32, last, last, meta.Vector(9, i32){1,2,3,4,5,6,7,8,0});
        next[6] += last[0];
        next[8] = last[0];
        p.* = next;
    }
    return @reduce(.Add, p.*);
}

fn solve(reader : anytype) !Solution
{
    //var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    //defer arena.deinit();
    //const allocator = &arena.allocator;

    var population = std.mem.zeroes(Population);
    var r = Solution{};
    while (try parseOne(reader)) |life| {
        population[life] += 1;
    }

    r.part1 = iterate(&population, 80);
    //r.part2 = countOverlaps(&world2);
    
    return r;
}

fn solveFile(fname : [] const u8) !Solution
{
    var file = try fs.cwd().openFile(fname, fs.File.OpenFlags{ .read = true });
    defer file.close();
    const reader = std.io.bufferedReader(file.reader()).reader();
    return try solve(reader);
}

pub fn main() anyerror!void {
    var example = try solveFile("example.txt");

    std.debug.print("example1 {d}\n", .{example.part1});
    std.debug.print("example2 {d}\n", .{example.part2});

    var result = try solveFile("input.txt");
    
    std.debug.print("part1 {d}\n", .{result.part1});
    std.debug.print("part2 {d}\n", .{result.part2});

    assert(result.part1 == 366057);
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
