using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading;
using BlackMythTrip;
using BlackMythTrip.Systems;

class Program
{
    static void Main(string[] args)
    {
        Console.OutputEncoding = Encoding.UTF8;
        Console.InputEncoding = Encoding.UTF8;
        
        // 显示游戏标题
        UI.ShowTitle();
        
        // 创建游戏状态
        GameState gameState;
        
        // 检查存档
        if (File.Exists("save.json"))
        {
            UI.ShowMainMenu();
            Console.Write("请输入选项: ");
            string choice = Console.ReadLine();
            
            if (choice == "1")
            {
                // 加载存档
                gameState = SaveSystem.LoadGame();
                Console.WriteLine("正在加载存档...");
                Thread.Sleep(500);
            }
            else if (choice == "2")
            {
                // 检查是否有二周目存档
                if (File.Exists("newgame_plus.json"))
                {
                    Console.WriteLine("\n检测到二周目存档，可以解锁隐藏内容！");
                    Console.Write("是否使用二周目继承数据开始游戏? (Y/N): ");
                    string ngChoice = Console.ReadLine().ToUpper();
                    if (ngChoice == "Y")
                    {
                        gameState = MemoryInheritanceSystem.StartNewGamePlus();
                    }
                    else
                    {
                        gameState = new GameState();
                    }
                }
                else
                {
                    gameState = new GameState();
                }
            }
            else if (choice == "3")
            {
                // 显示成就
                SaveSystem.DisplayAchievements();
                Console.WriteLine("\n按任意键返回...");
                Console.ReadKey();
                Main(args);
                return;
            }
            else if (choice == "4")
            {
                Console.WriteLine("感谢游玩《黑神话·三藏》！");
                return;
            }
            else
            {
                gameState = new GameState();
            }
        }
        else
        {
            // 首次进入游戏
            gameState = new GameState();
        }
        
        // 创建游戏引擎
        var engine = new GameEngine(gameState);
        
        // 开始游戏
        engine.Start();
        
        // 游戏结束后的循环
        Console.WriteLine("\n按任意键返回主菜单...");
        Console.ReadKey();
        Main(args);
    }
}
