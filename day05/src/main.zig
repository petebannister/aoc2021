const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;

const ParseError = error {
    Fail
};

const FindError = error {
    Missing
};

const Solution = struct {
    part1: u32,
    part2: u32,
    pub fn init() Solution {
        return Solution{
            .part1 = 0,
            .part2 = 0
        };
    }
};

const Allocator = std.mem.Allocator;
const Point = meta.Vector(2, i32);
const Line = [2]Point;
const World = std.AutoHashMap(Point, u32);

fn parsePoint(s: [] const u8) !Point {
    var iter = std.mem.split(u8, s, ",");
    var p : Point = undefined;
    var a = iter.next() orelse return ParseError.Fail;
    var b = iter.next() orelse return ParseError.Fail;

    p[0] = try std.fmt.parseUnsigned(i32, a, 10);
    p[1] = try std.fmt.parseUnsigned(i32, b, 10);
    return p;
}
fn parseLine(reader: anytype) !?Line
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = std.mem.split(u8, line, " -> ");
        var a = iter.next() orelse return null;
        var b = iter.next() orelse return null;
        var r : Line = undefined;
        r[0] = try parsePoint(a);
        r[1] = try parsePoint(b);
        return r;
    }
    return null;
}

fn addPoint(world: *World, x: i32, y: i32) !void
{
    var r = try world.getOrPut(Point{x, y});
    if (r.found_existing) {
        r.value_ptr.* += 1;
    }
    else {
        r.value_ptr.* = 1;
    }
}

fn addLine(world: *World, line: Line) !void
{
    if (line[0][0] == line[1][0]) { // vertical
        var x = line[0][0];
        var y1 = line[0][1];
        var y2 = line[1][1];
        if (y1 > y2) {
            std.mem.swap(i32, &y1, &y2);
        }
        var y = y1;
        while (y <= y2) : (y += 1) {
            try addPoint(world, x, y);
        } 
    }
    else if (line[0][1] == line[1][1]) { // horizontal
        var y = line[0][1];
        var x1 = line[0][0];
        var x2 = line[1][0];
        if (x1 > x2) {
            std.mem.swap(i32, &x1, &x2);
        }
        var x = x1;
        while (x <= x2) : (x += 1) {
            try addPoint(world, x, y);
        } 

    }
    // else ignore diagonals
}



fn addLineDiagonals(world: *World, line: Line) !void
{
    var dx = line[1][0] - line[0][0];
    var dy = line[1][1] - line[0][1];
    if (dx > 0) {
        dx = 1;
    }
    if (dx < 0) {
        dx = -1;
    }

    if (dy > 0) {
        dy = 1;
    }
    if (dy < 0) {
        dy = -1;
    }

    var dp = Point{dx, dy};

    var p1 = line[0];
    var p2 = line[1];
    // extend the endpoint for the termination condition
    p2 += dp;
    
    while (p1[0] != p2[0] or p1[1] != p2[1]) : (p1 += dp) {

        try addPoint(world, p1[0], p1[1]);
    }
}

fn countOverlaps(world: *World) u32
{
    var count : u32 = 0;
    var iter = world.iterator();
    while (iter.next()) |item| {
//std.debug.print("[{d},{d}]: {d}\n", .{item.key_ptr.*[0], item.key_ptr.*[1], item.value_ptr.*});

        if (item.value_ptr.* > 1) {
            count += 1;
        }
    }
    return count;
}

fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    var world = World.init(allocator);
    var world2 = World.init(allocator);
    var r = Solution.init();
    while (try parseLine(reader)) |line| {
        try addLine(&world, line);
        try addLineDiagonals(&world2, line);
    }

    r.part1 = countOverlaps(&world);
    r.part2 = countOverlaps(&world2);
    
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
    _ = example;

    var result = try solveFile("input.txt");
    
    std.debug.print("part1 {d}\n", .{result.part1});
    std.debug.print("part2 {d}\n", .{result.part2});
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
