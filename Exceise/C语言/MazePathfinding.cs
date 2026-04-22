/*
 * ============================================================================
 *  Maze Runner Game - C# Console Version
 * ============================================================================
 *
 * Features:
 * 1. Random maze generation using Recursive Backtracking algorithm
 * 2. Control a ball (@) to navigate through the maze
 * 3. Real-time path tracking (shows where you've been)
 * 4. After reaching the goal, display BFS and DFS paths
 * 5. Colorful console UI with smooth rendering
 *
 * Controls:
 *   Arrow Keys / WASD: Move the ball
 *   R: Restart with new maze
 *   Q: Quit game
 *   H: Show hint (BFS path)
 *
 * Author: AI Assistant | Date: 2026-04-22
 * ============================================================================
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

// ============================================================================
// Part 1: Maze Generator (Recursive Backtracking Algorithm)
// ============================================================================

public class MazeGenerator
{
    public int Rows { get; private set; }
    public int Cols { get; private set; }
    private int[,] grid;
    private static readonly (int dr, int dc)[] Directions = new (int, int)[]
    {
        (0, 2), (2, 0), (0, -2), (-2, 0)
    };
    private Random rand;

    public MazeGenerator(int rows, int cols)
    {
        Rows = rows;
        Cols = cols;
        rand = new Random();
        grid = new int[rows, cols];
        for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols; c++)
                grid[r, c] = 1;
    }

    public int[,] Generate()
    {
        CarvePath(1, 1);
        grid[1, 1] = 0;
        grid[Rows - 2, Cols - 2] = 0;
        return grid;
    }

    private void CarvePath(int row, int col)
    {
        grid[row, col] = 0;
        var shuffled = Directions.OrderBy(_ => rand.Next()).ToArray();
        foreach (var dir in shuffled)
        {
            int nr = row + dir.dr;
            int nc = col + dir.dc;
            if (nr > 0 && nr < Rows - 1 && nc > 0 && nc < Cols - 1 && grid[nr, nc] == 1)
            {
                grid[row + dir.dr / 2, col + dir.dc / 2] = 0;
                CarvePath(nr, nc);
            }
        }
    }
}

// ============================================================================
// Part 2: BFS Path Finder (Shortest Path)
// ============================================================================

public class PathFinderBFS
{
    private int[,] grid;
    private int rows, cols;
    public List<(int r, int c)> Path { get; private set; }
    public int NodesExpanded { get; private set; }

    public PathFinderBFS(int[,] grid)
    {
        this.grid = grid;
        rows = grid.GetLength(0);
        cols = grid.GetLength(1);
        Path = new List<(int, int)>();
    }

    public void FindPath(int startR, int startC, int endR, int endC)
    {
        if (grid[startR, startC] == 1 || grid[endR, endC] == 1) return;
        var visited = new bool[rows, cols];
        var parent = new (int r, int c)?[rows, cols];
        var queue = new Queue<(int r, int c)>();
        queue.Enqueue((startR, startC));
        visited[startR, startC] = true;
        NodesExpanded = 0;
        var directions = new (int dr, int dc)[] { (-1, 0), (1, 0), (0, -1), (0, 1) };

        while (queue.Count > 0)
        {
            var current = queue.Dequeue();
            NodesExpanded++;
            if (current.r == endR && current.c == endC)
            {
                ReconstructPath(parent, current);
                return;
            }
            foreach (var dir in directions)
            {
                int nr = current.r + dir.dr;
                int nc = current.c + dir.dc;
                if (nr >= 0 && nr < rows && nc >= 0 && nc < cols &&
                    !visited[nr, nc] && grid[nr, nc] == 0)
                {
                    visited[nr, nc] = true;
                    parent[nr, nc] = current;
                    queue.Enqueue((nr, nc));
                }
            }
        }
    }

    private void ReconstructPath((int r, int c)?[,] parent, (int r, int c) end)
    {
        var stack = new Stack<(int r, int c)>();
        var current = end;
        while (current.r != -1)
        {
            stack.Push(current);
            current = parent[current.r, current.c] ?? (-1, -1);
        }
        while (stack.Count > 0) Path.Add(stack.Pop());
    }
}

// ============================================================================
// Part 3: DFS Path Finder (Exploration)
// ============================================================================

public class PathFinderDFS
{
    private int[,] grid;
    private int rows, cols;
    public List<(int r, int c)> Path { get; private set; }
    public int NodesExpanded { get; private set; }

    public PathFinderDFS(int[,] grid)
    {
        this.grid = grid;
        rows = grid.GetLength(0);
        cols = grid.GetLength(1);
        Path = new List<(int, int)>();
    }

    public void FindPath(int startR, int startC, int endR, int endC)
    {
        if (grid[startR, startC] == 1 || grid[endR, endC] == 1) return;
        var visited = new bool[rows, cols];
        NodesExpanded = 0;
        DFS(startR, startC, endR, endC, visited, new List<(int, int)>());
    }

    private bool DFS(int r, int c, int endR, int endC, bool[,] visited, List<(int, int)> currentPath)
    {
        if (r < 0 || r >= rows || c < 0 || c >= cols || visited[r, c] || grid[r, c] == 1)
            return false;
        visited[r, c] = true;
        currentPath.Add((r, c));
        NodesExpanded++;
        if (r == endR && c == endC)
        {
            Path = new List<(int, int)>(currentPath);
            return true;
        }
        if (DFS(r - 1, c, endR, endC, visited, currentPath)) return true;
        if (DFS(r + 1, c, endR, endC, visited, currentPath)) return true;
        if (DFS(r, c - 1, endR, endC, visited, currentPath)) return true;
        if (DFS(r, c + 1, endR, endC, visited, currentPath)) return true;
        currentPath.RemoveAt(currentPath.Count - 1);
        return false;
    }
}

// ============================================================================
// Part 4: Maze Renderer
// ============================================================================

public static class MazeRenderer
{
    public static void DrawGameFull(int[,] grid, (int r, int c) player,
        HashSet<(int, int)> trail, List<(int, int)> hintPath,
        int hintMode, int moveCount)
    {
        int rows = grid.GetLength(0);
        int cols = grid.GetLength(1);
        // 无闪屏：将光标移到左上角原地重绘
        Console.SetCursorPosition(0, 0);
        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.WriteLine("+==========================================================+");
        Console.Write("|  迷宫逃生 - 控制 @ 从 S 到 E !                            ");
        Console.WriteLine("|");
        Console.WriteLine("+" + new string('=', cols * 2) + "+");
        Console.ResetColor();

        Console.Write("+");
        for (int i = 0; i < cols * 2; i++) Console.Write("-");
        Console.WriteLine("+");

        var hintSet = hintPath != null ? new HashSet<(int, int)>(hintPath) : null;

        for (int r = 0; r < rows; r++)
        {
            Console.Write("|");
            for (int c = 0; c < cols; c++)
            {
                var pos = (r, c);
                if (r == player.r && c == player.c)
                {
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.Write("@ ");
                }
                else if (r == 1 && c == 1)
                {
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.Write("S ");
                }
                else if (r == rows - 2 && c == cols - 2)
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.Write("E ");
                }
                else if (hintSet != null && hintSet.Contains(pos))
                {
                    Console.ForegroundColor = ConsoleColor.DarkGray;
                    Console.Write(". ");
                }
                else if (trail.Contains(pos))
                {
                    Console.ForegroundColor = ConsoleColor.Blue;
                    Console.Write("+ ");
                }
                else if (grid[r, c] == 1)
                {
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.Write("##");
                }
                else
                {
                    Console.ForegroundColor = ConsoleColor.DarkGreen;
                    Console.Write("  ");
                }
                Console.ResetColor();
            }
            Console.WriteLine("|");
        }

        Console.Write("+");
        for (int i = 0; i < cols * 2; i++) Console.Write("-");
        Console.WriteLine("+");

        Console.ForegroundColor = ConsoleColor.Gray;
        Console.Write($"  步数: {moveCount}  |  位置: ({player.Item1},{player.Item2})  |  ");
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine($"终点: ({rows - 2},{cols - 2})");
        Console.ResetColor();

        Console.ForegroundColor = ConsoleColor.DarkGray;
        Console.WriteLine("  方向键/WASD:移动  H:显示提示  R:新迷宫  Q:退出");
        Console.ResetColor();
    }
}

// ============================================================================
// Part 5: Main Program
// ============================================================================

class Program
{
    static int[,] currentMaze;
    static int mazeSize = 21;
    static (int, int) playerPos;
    static HashSet<(int, int)> playerTrail;
    static int moveCount;
    static bool gameWon;
    static bool showHint;
    static List<(int, int)> bfsHintPath;

    static Random rand = new Random();

    static void Main(string[] args)
    {
        Console.Title = "Maze Runner Game";
        Console.OutputEncoding = Encoding.UTF8;
        SetupConsole();
        ShowWelcome();
        NewGame();

        while (true)
        {
            MazeRenderer.DrawGameFull(currentMaze, playerPos, playerTrail,
                showHint ? bfsHintPath : null, 0, moveCount);

            if (gameWon)
            {
                Thread.Sleep(500);
                ShowVictoryScreen();
                break;
            }

            HandleInput();
            Thread.Sleep(16);
        }

        Console.WriteLine("\n  感谢游玩！按任意键退出...");
        Console.ReadKey();
    }

    static void SetupConsole()
    {
        try
        {
            Console.WindowWidth = Math.Min(90, Console.LargestWindowWidth);
            Console.WindowHeight = Math.Min(40, Console.LargestWindowHeight);
            Console.BufferWidth = Console.WindowWidth;
            Console.BufferHeight = 1000;
        }
        catch { }
        Console.CursorVisible = false;
    }

    static void ShowWelcome()
    {
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine("+==========================================================+");
        Console.WriteLine("|                                                          |");
        Console.WriteLine("|          >>>  迷 宫 逃 生 游 戏  <<<                      |");
        Console.WriteLine("|                                                          |");
        Console.WriteLine("|          控制小球(@)穿越随机生成的迷宫                     |");
        Console.WriteLine("|          从起点(S)到达终点(E)                            |");
        Console.WriteLine("|                                                          |");
        Console.WriteLine("|          功能特点:                                        |");
        Console.WriteLine("|          * 随机生成的完美迷宫                             |");
        Console.WriteLine("|          * 实时追踪你的路径                              |");
        Console.WriteLine("|          * 通关后对比BFS与DFS路径                        |");
        Console.WriteLine("|                                                          |");
        Console.WriteLine("+==========================================================+");
        Console.ResetColor();
        Console.WriteLine("\n  按任意键开始游戏...");
        Console.ReadKey();
    }

    static void NewGame()
    {
        if (mazeSize % 2 == 0) mazeSize++;
        var generator = new MazeGenerator(mazeSize, mazeSize);
        currentMaze = generator.Generate();
        playerPos = (1, 1);
        playerTrail = new HashSet<(int, int)>();
        playerTrail.Add(playerPos);
        moveCount = 0;
        gameWon = false;
        showHint = false;
        bfsHintPath = null;

        var bfs = new PathFinderBFS(currentMaze);
        bfs.FindPath(1, 1, mazeSize - 2, mazeSize - 2);
        bfsHintPath = bfs.Path;
    }

    static void HandleInput()
    {
        if (!Console.KeyAvailable) return;
        var key = Console.ReadKey(true);
        int nr = playerPos.Item1, nc = playerPos.Item2;

        switch (key.Key)
        {
            case ConsoleKey.UpArrow:
            case ConsoleKey.W:
                nr--; break;
            case ConsoleKey.DownArrow:
            case ConsoleKey.S:
                nr++; break;
            case ConsoleKey.LeftArrow:
            case ConsoleKey.A:
                nc--; break;
            case ConsoleKey.RightArrow:
            case ConsoleKey.D:
                nc++; break;
            case ConsoleKey.H:
                ToggleHint(); return;
            case ConsoleKey.R:
                NewGame(); return;
            case ConsoleKey.Q:
                Environment.Exit(0); return;
            default:
                return;
        }

        if (nr >= 0 && nr < mazeSize && nc >= 0 && nc < mazeSize &&
            currentMaze[nr, nc] == 0)
        {
            playerPos = (nr, nc);
            playerTrail.Add(playerPos);
            moveCount++;

            if (nr == mazeSize - 2 && nc == mazeSize - 2)
                gameWon = true;
        }
    }

    static void ToggleHint()
    {
        showHint = !showHint;
    }

    static void ShowVictoryScreen()
    {
        // 无闪屏：将光标移到左上角原地重绘
        Console.SetCursorPosition(0, 0);
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("\n");
        Console.WriteLine("    !!! CONGRATULATIONS !!! YOU ESCAPED THE MAZE!\n");
        Console.ResetColor();

        var bfs = new PathFinderBFS(currentMaze);
        bfs.FindPath(1, 1, mazeSize - 2, mazeSize - 2);
        var dfs = new PathFinderDFS(currentMaze);
        dfs.FindPath(1, 1, mazeSize - 2, mazeSize - 2);

        int efficiency = bfs.Path.Count > 0 ?
            (int)((double)bfs.Path.Count / moveCount * 100) : 0;

        Console.WriteLine("  Your Stats:");
        Console.WriteLine($"    Total Moves: {moveCount}");
        Console.WriteLine($"    Cells Visited: {playerTrail.Count}\n");

        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine("  [BFS] Shortest Path:");
        Console.WriteLine($"    Path Length: {bfs.Path.Count} steps");
        Console.WriteLine($"    Nodes Explored: {bfs.NodesExpanded}\n");

        Console.ForegroundColor = ConsoleColor.Magenta;
        Console.WriteLine("  [DFS] Exploration Path:");
        Console.WriteLine($"    Path Length: {dfs.Path.Count} steps");
        Console.WriteLine($"    Nodes Explored: {dfs.NodesExpanded}\n");

        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.WriteLine("  Comparison:");
        Console.WriteLine("  +------------------+----------+----------+");
        Console.WriteLine("  | Path Length     |   BFS    |   DFS    |");
        Console.WriteLine("  +------------------+----------+----------+");
        Console.WriteLine($"  | Steps            | {bfs.Path.Count,-8} | {dfs.Path.Count,-8} |");
        Console.WriteLine($"  | Nodes Explored   | {bfs.NodesExpanded,-8} | {dfs.NodesExpanded,-8} |");
        Console.WriteLine("  +------------------+----------+----------+\n");

        Console.ForegroundColor = ConsoleColor.Gray;
        Console.WriteLine("  Your Performance Analysis:");
        Console.WriteLine($"    Efficiency: {efficiency}%");
        if (moveCount <= bfs.Path.Count * 1.2)
            Console.WriteLine("    Rating: Excellent! Near-optimal path!");
        else if (moveCount <= bfs.Path.Count * 1.5)
            Console.WriteLine("    Rating: Good! Pretty efficient!");
        else if (moveCount <= bfs.Path.Count * 2)
            Console.WriteLine("    Rating: Fair. Try to find shorter paths!");
        else
            Console.WriteLine("    Rating: Completed! Try to find shorter path.");

        Console.WriteLine("\n  Press R to play again, or Q to quit.");
        var key = Console.ReadKey(true);
        if (key.Key == ConsoleKey.R)
            NewGame();
    }
}
