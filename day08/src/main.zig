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

const Glyph = u7;
const Patterns = [10]Glyph;
const Outputs = [4]Glyph;
//const Allocator = std.mem.Allocator;
const Display = struct {
    patterns: Patterns = std.mem.zeroes(Patterns),
    outputs: Outputs = std.mem.zeroes(Outputs)
};
const Displays = std.ArrayList(Display);
const GlyphLookup = [128]u8;

fn parseGlyph(s: [] const u8) Glyph {
    var r : Glyph = 0;
    for (s) |c| {
        r |= @as(u7, 1) << @truncate(u3, (c - 'a'));
    }
    return r;
}

fn parseOne(reader: anytype) !?Display
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var parts = std.mem.split(u8, line, " | ");
        var patterns = std.mem.split(u8, parts.next().?, " ");
        var outputs = std.mem.split(u8, parts.next().?, " ");
        var display = Display{};
        var index : u8 = 0;
        while (patterns.next()) |field| {
            display.patterns[index] = parseGlyph(field);
            index += 1;
        }
        index = 0;
        while (outputs.next()) |field| {
            display.outputs[index] = parseGlyph(field);
            index += 1;
        }
        return display;
    }
    return null;
}

const d1 = 2;
const d7 = 3;
const d4 = 4;
const d8 = 7;

fn easyGlyph(g : Glyph) u8 {
    const lut = [_]u8{0, 0,'1', '7', '4', 0, 0, '8'};
    var nbits = @popCount(Glyph, g);
    return lut[nbits];
}

fn partOne(displays : Displays) u32
{
    var count: u32 = 0;
    for (displays.items) |display| {
        var lut = std.mem.zeroes(GlyphLookup);
        for (display.patterns) |g| {
            var c = easyGlyph(g);
            if (c != 0) {
                lut[g] = c;
            }
        }
        for (display.outputs) |output| {
            if (lut[output] != 0) {
                count += 1;
            }
        }
    }
    return count;
}

fn solveDisplay(display : Display) !u32
{
    var lut = std.mem.zeroes(GlyphLookup);
    var rv = std.mem.zeroes([16]Glyph);
    for (display.patterns) |gl| {
        var c = easyGlyph(gl);
        if (c != 0) {
            lut[gl] = c;
            rv[@truncate(u4, c - '0')] = gl;
        }
    }
    // deduce 9
    for (display.patterns) |gl| {
        if (1 == @popCount(Glyph, gl ^ (rv[4] | rv[7]))) {
            if (0 == lut[gl]) {
                lut[gl] = '9';
                rv[9] = gl;
            }
        }
    }
    // deduce 5 and 3
    for (display.patterns) |gl| {
        if (1 == @popCount(Glyph, gl ^ rv[9])) {
            if (0 == lut[gl]) {
                if ((gl & rv[1]) == rv[1]) {
                    lut[gl] = '3';
                    rv[3] = gl;
                }
                else {
                    lut[gl] = '5';
                    rv[5] = gl;
                }
            }
        }
    }
    // 1 ^ 7 determines what bit 'a' is
    // 1 ^ 4 reveals b and d
    // (4 | 7) ^ 8 reveals e and g
    //var a = rv[1] ^ rv[7];
    //var bd = rv[1] ^ rv[4];
    //var eg = (rv[4] | rv[7]) ^ rv[8];
    var e = (rv[8] ^ rv[9]);
    //var g = e ^ eg;
    //var c = (rv[5] ^ rv[9]);

    // 6
    rv[6] = rv[5] | e;
    lut[rv[6]] = '6';
    var b = (rv[3] ^ rv[9]);

    // deduce 0 and 2
    for (display.patterns) |gl| {
        if (lut[gl] == 0) {
            if ((gl & b) == 0) {
                lut[gl] = '2';
                rv[2] = gl;
            }
            else {
                lut[gl] = '0';
                rv[0] = gl;
            }
        }
    }
    

    var text: [4]u8 = undefined;
    text[0] = lut[display.outputs[0]];
    text[1] = lut[display.outputs[1]];
    text[2] = lut[display.outputs[2]];
    text[3] = lut[display.outputs[3]];
    return std.fmt.parseInt(u32, &text, 10);
}

fn partTwo(displays : Displays) !u32
{
    var count: u32 = 0;
    for (displays.items) |display| {
        var dv = try solveDisplay(display);
        count += dv;
    }
    return count;
}

fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    var displays = Displays.init(&arena.allocator);
    while (try parseOne(reader)) |val| {
        try displays.append(val);
    }
    var r = Solution{
        .part1 = partOne(displays),
        .part2 = try partTwo(displays),
    };
    
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
