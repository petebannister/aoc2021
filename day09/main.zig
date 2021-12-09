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
const BathymetryRow = ArrayList(u8);
const Bathymetry = struct {
    items: ArrayList(BathymetryRow),
    width: i32 = 0,
    height: i32 = 0,
    fn init(a: *Allocator) !Bathymetry{
        return Bathymetry{
            .items = ArrayList(BathymetryRow).init(a),
        };
    }
};

fn parseOne(reader: anytype, allocator: *Allocator) !?BathymetryRow
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        if (line.len == 0) {
            return null;
        }
        var row = BathymetryRow.init(allocator);
        for (line) |h| {
            try row.append(h - '0');
        }
        return row;
    }
    return null;
}

fn getPoint(p: Vec2, b: *Bathymetry) ?u8 {
    if (p[0] < 0 or p[0] >= b.width or p[1] < 0 or p[1] >= b.height) {
        return null;
    }
    return b.items.items[@intCast(usize, p[1])].items[@intCast(usize, p[0])];
}

fn setPoint(p: Vec2, b: *Bathymetry, h: u8) void {
    if (p[0] < 0 or p[0] >= b.width or p[1] < 0 or p[1] >= b.height) {
        return;
    }
    b.items.items[@intCast(usize, p[1])].items[@intCast(usize, p[0])] = h;
}
const dirs = [4]Vec2 {
    Vec2{ -1, 0 }, // left
    Vec2{ 0, -1 }, // up
    Vec2{ 1, 0 }, // right
    Vec2{ 0, 1 } // down
};

fn lowPoints(b: *Bathymetry) u32
{
    var result: u32 = 0;
    var p = Vec2{0, 0};
    while (p[1] < b.height) : (p[1] += 1) {
        p[0] = 0;
        while (p[0] < b.width) : (p[0] += 1) {
            var h = getPoint(p, b);
            for (dirs) |dir| {
                if (getPoint(p + dir, b)) |adj| {
                    if (adj <= h.?) {
                        h = null;
                        break;
                    }
                }
            }
            if (h) |v| {
                var risk = 1 + v;
                result += risk;
            }
        }
    }
    return result;
}

fn lowPointsVec(b: *Bathymetry, a: *Allocator) !ArrayList(Vec2)
{
    var result = ArrayList(Vec2).init(a);
    var p = Vec2{0, 0};
    while (p[1] < b.height) : (p[1] += 1) {
        p[0] = 0;
        while (p[0] < b.width) : (p[0] += 1) {
            var h = getPoint(p, b);
            for (dirs) |dir| {
                if (getPoint(p + dir, b)) |adj| {
                    if (adj <= h.?) {
                        h = null;
                        break;
                    }
                }
            }
            if (h) |_| {
                try result.append(p);
            }
        }
    }
    return result;
}

fn basins(b: *Bathymetry) !u32
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var result: u32 = 0;
    var low = try lowPointsVec(b, &arena.allocator);
    var stack = std.ArrayList(Vec2).init(&arena.allocator);
    var basin_sizes = std.ArrayList(u32).init(&arena.allocator);
    for (low.items) |lp| {
        var basin_size : u32 = 0;
        try stack.append(lp);
        while (stack.items.len > 0) {
            var p = stack.pop();
            if (getPoint(p, b)) |h| {
                if (h < 9) {
                    basin_size += 1;
                    // mark done - avoids visited set
                    setPoint(p, b, 10);
                    
                    for (dirs) |dir| {
                        var p2 = p + dir;
                        if (getPoint(p2, b)) |adj| {
                            if (adj < 9) {
                                try stack.append(p2);
                            }
                        }
                    }
                }
            }
        }
        try basin_sizes.append(basin_size);
    }
    std.sort.sort(u32, basin_sizes.items, {}, comptime std.sort.desc(u32));
    result = basin_sizes.items[0]*basin_sizes.items[1]*basin_sizes.items[2];
    return result;
}

fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var bathy = try Bathymetry.init(&arena.allocator);
    while (try parseOne(reader, &arena.allocator)) |row| {
        bathy.width = @intCast(i32, row.items.len);
        try bathy.items.append(row);
    }
    bathy.height = @intCast(i32, bathy.items.items.len);

    var r = Solution{
        .part1 = lowPoints(&bathy),
        .part2 = try basins(&bathy),
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
