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

const ParseResult = struct {
    score: u32 = 0,
    score2: u64 = 0,
};
const Pair = struct {
    a: u8,
    b: u8,
    score: u32,
    score2: u32,
    fn init(a: u8, b:u8, score:u32, score2:u32) Pair {
        return Pair{
            .a = a,
            .b = b,
            .score = score,
            .score2 = score2
        };
    }
};
const pairs = [_]Pair {
    Pair.init('(', ')', 3, 1),
    Pair.init('[', ']', 57, 2),
    Pair.init('{', '}', 1197, 3),
    Pair.init('<', '>', 25137, 4),
};

fn readByte(reader: anytype) ?u8
{
    return reader.readByte() catch null;
}

fn findPairOpen(c: u8) ?Pair {
    for (pairs) |p| {
        if (c == p.a) {
            return p;
        }
    }
    return null;
}

fn findPairClose(c: u8) ?Pair {
    for (pairs) |p| {
        if (c == p.b) {
            return p;
        }
    }
    return null;
}
fn parseOne(reader: anytype, allocator: *Allocator) !?ParseResult
{
    var stack = ArrayList(Pair).init(allocator);
    var result = ParseResult{};
    var failed = false;
    while (readByte(reader)) |c| {
        if (c == '\n') {
            if (!failed) {
                while (stack.popOrNull()) |expected|{
                    result.score2 *= 5;
                    result.score2 += findPairClose(expected.b).?.score2;
                }
            }
            return result;
        }
        if (failed) {
            continue;
        }
        if (findPairOpen(c)) |p| {
            try stack.append(p); // what do we expect?
        }
        else if (stack.popOrNull())|expected| {
            if (c != expected.b) {
                failed = true;
                result.score = findPairClose(c).?.score;
                stack.clearRetainingCapacity();
            }
        }
    }
    return null;
}


fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var r = Solution{};
    var scores2 = ArrayList(u64).init(&arena.allocator);
    while (try parseOne(reader, &arena.allocator)) |line| {
        r.part1 += line.score;
        if (line.score2 != 0) {
            try scores2.append(line.score2);
        }
    }
    std.sort.sort(u64, scores2.items, {}, comptime std.sort.asc(u64));
    r.part2 = scores2.items[(scores2.items.len) / 2];
    return r;
}

pub fn main() anyerror!void {
    var example = try utl.solveFile("example.txt", solve);
    example.print("example");
    var result = try utl.solveFile("input.txt", solve);
    result.print("part");
    //assert(result.part1 == 366057);
}
