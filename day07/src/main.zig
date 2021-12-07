const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;

const AocError = error {
    ParseError
};

const Solution = struct {
    part1: u64 = 0,
    part2: u64 = 0,
};

const Number = i16;
//const Allocator = std.mem.Allocator;
const Positions = std.ArrayList(Number);

fn parseOne(reader: anytype) !?Number
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, ',')) |field| {
        var num = std.mem.trim(u8, field, "\r\n");
        return std.fmt.parseUnsigned(Number, num, 10) catch return null;
    }
    return null;
}

fn lowestCostMove(p: *Positions) !u32
{
    var max = p.items[p.items.len - 1];
    var i : Number = 0;
    var best : i32 = 0x7FFF_FFFF;
    while (i <= max) : (i += 1) {
        var cost: i32 = 0;
        for (p.items) |pos| {
            var move = try std.math.absInt(i - pos);
            cost += move;
            if (cost > best) {
                break;
            }
        }
        if (cost < best) {
            best = cost;
        }
    }
    return try std.math.cast(u32, best);
}
fn lowestCostMove2(p: *Positions) !u64
{
    var max = p.items[p.items.len - 1];
    var i : Number = 0;
    var best : u64 = 0xFFFF_FFFF_FFFF_FFFF;
    while (i <= max) : (i += 1) {
        var cost: u64 = 0;
        for (p.items) |pos| {
            var move = try std.math.cast(u64, try std.math.absInt(i - pos));
            // triangular number
            move = (move * (move + 1)) / 2;
            cost += move;
            if (cost > best) {
                break;
            }
        }
        if (cost < best) {
            best = cost;
        }
    }
    return try std.math.cast(u64, best);
}
fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;
    var positions = Positions.init(allocator);
    
    var r = Solution{};
    while (try parseOne(reader)) |val| {
        try positions.append(val);
    }
    // Sort the positions to allow tail minimum cost prediction
    std.sort.sort(Number, positions.items, {}, comptime std.sort.asc(Number));

    r.part1 = try lowestCostMove(&positions);
    r.part2 = try lowestCostMove2(&positions);
    
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

    //assert(result.part1 == 366057);
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
