const std = @import("std");
const utl = @import("util.zig");
const meta = std.meta;
const assert = std.debug.assert;
const print = std.debug.print;
const Allocator = std.mem.Allocator;
const ArrayList = std.ArrayList;

const Error = utl.Error;
const Solution = utl.Solution;
const Vec2 = utl.Vec2;
const split = utl.split;
const WorldRow = ArrayList(u8);
const World = struct {
    items: ArrayList(WorldRow),
    width: i32 = 0,
    height: i32 = 0,
    fn init(a: *Allocator) !World{
        return World{
            .items = ArrayList(WorldRow).init(a),
        };
    }
};

fn parseOne(reader: anytype, allocator: *Allocator) !?WorldRow
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        if (line.len == 0) {
            return null;
        }
        var row = WorldRow.init(allocator);
        for (line) |h| {
            try row.append(h - '0');
        }
        return row;
    }
    return null;
}

fn getPoint(p: Vec2, b: *World) ?u8 {
    if (p[0] < 0 or p[0] >= b.width or p[1] < 0 or p[1] >= b.height) {
        return null;
    }
    return b.items.items[@intCast(usize, p[1])].items[@intCast(usize, p[0])];
}

fn setPoint(p: Vec2, b: *World, h: u8) void {
    if (p[0] < 0 or p[0] >= b.width or p[1] < 0 or p[1] >= b.height) {
        return;
    }
    b.items.items[@intCast(usize, p[1])].items[@intCast(usize, p[0])] = h;
}
const dirs = [_]Vec2 {
    Vec2{ -1, 0 }, // left
    Vec2{ 0, -1 }, // up
    Vec2{ 1, 0 }, // right
    Vec2{ 0, 1 }, // down
    
    Vec2{ -1, -1 }, // lt
    Vec2{ 1, -1 }, // rt
    Vec2{ 1, 1 }, // rb
    Vec2{ -1, 1 }, // lb
};

fn flashAdjacent(p: Vec2, b: *World) void
{
    for (dirs) |dir| {
        if (getPoint(p + dir, b)) |_adj| {
            var adj = _adj;
            if (adj < 10) {
                adj += 1; 
                setPoint(p + dir, b, adj);
                if (adj > 9) {
                    flashAdjacent(p + dir, b);
                }
            }
        }
    }
}
fn preStep(b: *World) void
{
    var p = Vec2{0, 0};
    while (p[1] < b.height) : (p[1] += 1) {
        p[0] = 0;
        while (p[0] < b.width) : (p[0] += 1) {
            if (getPoint(p, b)) |_v| {
                var v = _v;
                if (v < 10) {
                    v += 1;
                    setPoint(p, b, v);
                    if (v > 9) {
                        flashAdjacent(p, b);
                    }
                }
            }
        }
    }
}
fn flashReset(b: *World) u32
{
    var p = Vec2{0, 0};
    var flash: u32 = 0;
    while (p[1] < b.height) : (p[1] += 1) {
        p[0] = 0;
        while (p[0] < b.width) : (p[0] += 1) {
            if (getPoint(p, b)) |v| {
                if (v > 9) {
                    flash += 1;
                    setPoint(p, b, 0);
                }
            }
        }
    }
    return flash;
}
fn part1(world: *World) u32
{
    var result: u32 = 0;
    var step : u32 = 0;
    while (step < 100) : (step += 1) {
        preStep(world);
        result += flashReset(world);
    }
    return result;
}

fn part2(world: *World) u32
{
    var step : u32 = 0;
    var n = world.height * world.width;
    while (true) : (step += 1) {
        preStep(world);
        if (n == flashReset(world)) {
            return step;
        }
    }
    return 0;
}
fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var world = try World.init(&arena.allocator);
    while (try parseOne(reader, &arena.allocator)) |row| {
        world.width = @intCast(i32, row.items.len);
        try world.items.append(row);
    }
    world.height = @intCast(i32, world.items.items.len);

    var r = Solution{
        .part1 = part1(&world),
        .part2 = part2(&world) + 101, // HACK!
    };
    
    return r;
}

pub fn main() anyerror!void {
    var example = try utl.solveFile("example.txt", solve);
    example.print("example");
    var result = try utl.solveFile("input.txt", solve);
    result.print("part");
    //assert(result.part1 == 366057);
}
