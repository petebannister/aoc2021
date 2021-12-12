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

const Cave = struct {
    //name: [] const u8,
    small: bool,
    next: ArrayList(*Cave),
    fn init(name: [] const u8, a: *Allocator) Cave {
        var r: Cave = undefined;
        //r.name = name;
        r.small = (name[0] >= 'a' and name[0] <= 'z');
        r.next = @TypeOf(r.next).init(a);
        return r;
    }
};
const CaveMap = std.StringHashMap(*Cave);

fn parseOne(map: *CaveMap, reader: anytype, allocator: *Allocator) !bool
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = split(line, "-");
        var a = try allocator.dupeZ(u8, iter.next().?);
        var b = try allocator.dupeZ(u8, iter.next().?);
        var ar = try map.getOrPut(a);
        if (!ar.found_existing) {
            var c = try allocator.create(Cave);
            c.* = Cave.init(a, allocator);
            //ar.key_ptr.*
            ar.value_ptr.* = c;
        }
        var br = try map.getOrPut(b);
        if (!br.found_existing) {
            var c = try allocator.create(Cave);
            c.* = Cave.init(b, allocator);
            br.value_ptr.* = c;
        }
        try ar.value_ptr.*.next.append(br.value_ptr.*);
        try br.value_ptr.*.next.append(ar.value_ptr.*);
        return true;
    }
    return false;
}

fn back(list: anytype) @TypeOf(list.items[0]) {
    return list.items[list.items.len - 1];
}
fn backPtr(list: anytype) @TypeOf(&list.items[0]) {
    return &list.items[list.items.len - 1];
}


fn canVisit(cave:* Cave, path: []* Cave) bool
{
    if (cave.small) {
        // O(N) search here could be improved?
        for (path) |visited| {
            if (visited == cave) {
                return false;
            }
        }
    }
    return true;
}

fn part1(caves: *CaveMap, allocator: *Allocator) !u32
{
    var start = caves.getPtr("start").?.*;
    var end = caves.getPtr("end").?.*;
    var path = ArrayList(*Cave).init(allocator);
    var index = ArrayList(i32).init(allocator);
    try path.append(start);
    try index.append(-1);
    var count: u32 = 0;
    while (path.items.len > 0) {
        var i : *i32 = backPtr(index);
        var c : *Cave = back(path);
        i.* += 1;
        if (i.* >= @intCast(i32, c.next.items.len)) {
            _ = index.pop();
            _ = path.pop();
        }
        else {
            var next = c.next.items[@intCast(usize, i.*)];
            if (next == end) {
                count += 1;
            }
            else if (canVisit(next, path.items)) {
                try path.append(next);
                try index.append(-1);
            }
        }
    }
    return count;
}

fn part2(caves: *CaveMap, allocator: *Allocator) !u32
{
    _ = caves;
    _ = allocator;
    return 0;
}
fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var caves = CaveMap.init(&arena.allocator);
    while (try parseOne(&caves, reader, &arena.allocator)) {
        
    }

    var r = Solution{
        .part1 = try part1(&caves, &arena.allocator),
        .part2 = try part2(&caves, &arena.allocator)
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
