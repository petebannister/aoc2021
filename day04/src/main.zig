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
const Number = u8;
const Numbers = std.ArrayList(Number);
const BoardLine = [5]Number;
const BoardNumbers = [5]BoardLine;
const BoardFlags = [5]std.bit_set.StaticBitSet(5);
const Board = struct {
    rows: BoardNumbers,
    marked_rows: BoardFlags,
    marked_cols: BoardFlags,
    win_order: u32,
    winning_draw: u32,
    pub fn init(numbers: BoardNumbers) Board {
        return Board{
            .rows = numbers,
            .marked_rows = std.mem.zeroes(BoardFlags),
            .marked_cols = std.mem.zeroes(BoardFlags), 
            .win_order = 0,
            .winning_draw = 0
        };
    }
};
const Boards = std.ArrayList(Board);

const Game = struct {
    numbers: Numbers,
    boards: Boards,
    index: u32,
    won: u32
};


fn parseNumbers(reader: anytype, allocator: *Allocator) !Numbers
{
    var buf: [1024]u8 = undefined;
    var numbers = Numbers.init(allocator);
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = std.mem.split(u8, line, ",");
        while (iter.next()) |field| {
            var num = try std.fmt.parseUnsigned(Number, field, 10);
            try numbers.append(num);
        }
        return numbers;
    }
    return ParseError.Fail;
}
fn skipLine(reader: anytype) !void {
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line|{
        _ = line;
        return;
    }
    return ParseError.Fail;
}
fn parseBoardLine(reader: anytype) !BoardLine
{
    var buf: [1024]u8 = undefined;
    if (try reader.readUntilDelimiterOrEof(&buf, '\n')) |line| {
        var iter = std.mem.split(u8, line, " ");
        var index : u8 = 0;
        var board_line = std.mem.zeroes(BoardLine);
        while (iter.next()) |field| {
            var num = std.fmt.parseUnsigned(Number, field, 10) catch continue;
            board_line[index] = num;
            index += 1;
        }
        return board_line;
    }
    return ParseError.Fail;
}
fn parseBoard(reader: anytype) !Board
{
    var board : BoardNumbers = undefined;
    board[0] = try parseBoardLine(reader);
    board[1] = try parseBoardLine(reader);
    board[2] = try parseBoardLine(reader);
    board[3] = try parseBoardLine(reader);
    board[4] = try parseBoardLine(reader);
    return Board.init(board);
}

fn parseGame(reader: anytype, allocator: *Allocator) !Game
{
    var game : Game = undefined;
    game.numbers = try parseNumbers(reader, allocator);
    game.boards = Boards.init(allocator);
    game.index = 0;
    game.won = 0;
    while (true) {
        skipLine(reader) catch return game;
        var board = parseBoard(reader) catch return game;
        try game.boards.append(board);
    }
    return ParseError.Fail;
}

fn step(game: *Game) void {
    var draw = game.numbers.items[game.index];
    game.index += 1;
    for (game.boards.items) |*board| {
        if (board.win_order == 0) { // Only process board if it is not completed
            for (board.rows) |*row, irow| {
                for (row) |*col, icol| {
                    if (col.* == draw) {
                        board.marked_rows[irow].set(icol);
                        board.marked_cols[icol].set(irow);

                        // Winning Board?
                        var won = false;
                        if (board.marked_rows[irow].count() == 5) {
                            won = true;
                        }
                        if (board.marked_cols[icol].count() == 5) {
                            won = true;
                        }
                        if (won) {
                            game.won += 1;
                            board.win_order = game.won;
                            board.winning_draw = draw;
                        }
                    }
                }
            }
        }
    }
}

fn winningBoard(game: *Game) !*Board {
    for (game.boards.items) |*board| {
        if (board.win_order == 1) {
            return board;
        }
    }
    return FindError.Missing;
}
fn losingBoard(game: *Game) !*Board {
    for (game.boards.items) |*board| {
        if (board.win_order == game.won) {
            return board;
        }
    }
    return FindError.Missing;
}
fn score(board: *Board) u32 
{
    var result: u32 = 0;

    for (board.rows) |row, irow| {
        for (row) |col, icol| {
            if (!board.marked_rows[irow].isSet(icol)) {
                result += col;
            }
        }
    }

    return result * board.winning_draw;
}

fn solve(reader : anytype) !Solution
{
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = &arena.allocator;

    var game = try parseGame(reader, allocator);
    var r = Solution.init();
    while (game.won < game.boards.items.len) {
        step(&game);
    }

    var winning = try winningBoard(&game);
    r.part1 = score(winning);
    var losing = try losingBoard(&game);
    r.part2 = score(losing);
    
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
    assert(result.part1 == 14093);
    assert(result.part2 == 17388);
}

// Example test
// const expect = @import("std").testing.expect;
// test "example" {
//     try expect(1 == 1);
// }
