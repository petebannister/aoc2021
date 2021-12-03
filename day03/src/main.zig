const std = @import("std");
const fs = std.fs;
const meta = std.meta;
const assert = std.debug.assert;


const example = [_]u32{
    0b00100,
    0b11110,
    0b10110,
    0b10111,
    0b10101,
    0b01111,
    0b00111,
    0b11100,
    0b10000,
    0b11001,
    0b00010,
    0b01010,
};

fn parseLine(reader: anytype) !?u32
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        if (line.len > 0) {
            var value =  try std.fmt.parseUnsigned(u32, line, 2);
            return value;
        }
    }
    return null;
}
const one:u32 = 1;
fn accumulateBits(freq: []u32, value: u32) void {
    for (freq) |*digit, index| {
        var bit = (freq.len - 1) - index;
        var mask:u32  = (one << @truncate(u5,bit));
        if (0 != (value & mask)) {
            digit.* += 1;
        }
    }
}

fn mostCommonBits(freq: [] const u32, total: usize) u32 {
    var result:u32 = 0;
    for (freq) |value| {
        result <<= one;
        if ((value * 2) >= total) {
            result |= one;
        }
    }
    return result; 
}

fn leastCommonBits(freq: [] const u32, total: usize) u32 {
    var result:u32 = 0;
    for (freq) |value| {
        result <<= one;
        if ((value * 2) < total) {
            result |= one;
        }
    }
    return result;
}
fn getRatingValue(numbers: []const u32, alloc: *std.mem.Allocator, comptime nbits: u32, comptime most: bool) !u32 
{
    var set = try std.DynamicBitSet.initFull(numbers.len, alloc);
    defer set.deinit();
    var bit:u5 = nbits;
    var rating: u32 = 0;
    outer: while (bit > 0) {
        bit -= 1;
        var mask:u32 = one << (bit);
        var iter = set.iterator(.{});
        var freq = std.mem.zeroes([nbits]u32);
        while (iter.next()) |index| {
            var val = numbers[index];
            accumulateBits(&freq, val);
        }
        var criteria : u32 = undefined;
        if (most) {
            criteria = mostCommonBits(&freq, set.count());
        }
        else {
            criteria = leastCommonBits(&freq, set.count());
        }
        iter = set.iterator(.{});
        while (iter.next()) |index| {
            var v = numbers[index];
            if ((v & mask) != (criteria & mask)) {
                set.unset(index);
                if (1 == set.count()) {
                    if (set.iterator(.{}).next()) |index2| {
                        rating = numbers[index2];
                        set.unset(index2);
                        break :outer;
                    }
                }
            }
        }
    }
    return rating;
}
fn getO2RatingValue(numbers: []const u32, alloc: *std.mem.Allocator, comptime nbits: u32) !u32
{
    return getRatingValue(numbers, alloc, nbits, true);
} 
fn getCO2RatingValue(numbers: []const u32, alloc: *std.mem.Allocator, comptime nbits: u32) !u32
{
    return getRatingValue(numbers, alloc, nbits, false);
} 
fn solve(reader : anytype) !void
{
    var gpalloc = std.heap.GeneralPurposeAllocator(.{}){};
    var alloc = &gpalloc.allocator;
    var freq = std.mem.zeroes([12]u32);
    var numbers = std.ArrayList(u32).init(alloc);
    defer numbers.deinit();

    while (try parseLine(reader)) |value| {
        //std.debug.print("{d}, {d}\n", .{ cmd[0], cmd[1] });
        accumulateBits(&freq, value);
        try numbers.append(value);
    }

    var gamma = mostCommonBits(&freq, numbers.items.len);
    var epsilon = ~gamma & 0xFFF; 

    var o2_rating = try getO2RatingValue(numbers.items, alloc, 12);
    var co2_rating = try getCO2RatingValue(numbers.items, alloc, 12);

    assert(23 == try getO2RatingValue(&example, alloc, 5));
    assert(10 == try getCO2RatingValue(&example, alloc, 5));

    var part1: u32 = gamma * epsilon;
    var part2: u32 = o2_rating * co2_rating;
    std.debug.print("part1 {d}\n", .{part1});
    std.debug.print("part2 {d}\n", .{part2});
    assert(part1 == 2954600);
    assert(part2 == 1662846);
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

const expect = @import("std").testing.expect;
test "example" {
    const alloc = std.testing.allocator;
    try expect(23 == try getO2RatingValue(&example, alloc));
    try expect(10 == try getCO2RatingValue(&example, alloc));
}