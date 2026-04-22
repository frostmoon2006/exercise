using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace BlackMythTrip
{
    /// <summary>
    /// UI界面系统
    /// </summary>
    public static class UI
    {
        // 控制台颜色
        public static ConsoleColor TitleColor = ConsoleColor.Cyan;
        public static ConsoleColor StoryColor = ConsoleColor.White;
        public static ConsoleColor WarningColor = ConsoleColor.Yellow;
        public static ConsoleColor DangerColor = ConsoleColor.Red;
        public static ConsoleColor SuccessColor = ConsoleColor.Green;
        public static ConsoleColor ChoiceColor = ConsoleColor.Magenta;
        public static ConsoleColor KarmaColor = ConsoleColor.DarkRed;
        public static ConsoleColor CompassionColor = ConsoleColor.DarkBlue;
        
        /// <summary>
        /// 显示游戏标题
        /// </summary>
        public static void ShowTitle()
        {
            Console.Clear();
            
            string[] title = {
                "╔═══════════════════════════════════════════════════════════╗",
                "║                                                           ║",
                "║            《 黑 神 话 · 三 藏 》                           ║",
                "║                                                           ║",
                "║              ～ 轮 回 之 缚 与 真 经 谎 言 ～               ║",
                "║                                                           ║",
                "║                     WILD TRIP TO WEST                      ║",
                "║                                                           ║",
                "╚═══════════════════════════════════════════════════════════╝"
            };
            
            Console.ForegroundColor = TitleColor;
            foreach (var line in title)
            {
                Console.WriteLine(line);
            }
            Console.ResetColor();
            
            Console.WriteLine();
            TypeWriter("  轮回之始，真经铸劫。取经之路，实为囚笼。", 30);
            Console.WriteLine();
            Thread.Sleep(500);
        }
        
        /// <summary>
        /// 显示主菜单
        /// </summary>
        public static void ShowMainMenu()
        {
            Console.WriteLine("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Console.WriteLine("           【主菜单】           ");
            Console.WriteLine("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            Console.WriteLine("  1. 继续游戏");
            Console.WriteLine("  2. 新游戏");
            Console.WriteLine("  3. 已解锁成就");
            Console.WriteLine("  4. 退出游戏");
            Console.WriteLine("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        }
        
        /// <summary>
        /// 显示玩家状态栏
        /// </summary>
        public static void ShowStatusBar(Player player, GameState gameState)
        {
            Console.WriteLine();
            Console.WriteLine("┌─────────────────────────────────────────────────────────────┐");
            
            string chapterName = gameState.GetCurrentChapterName();
            string formName = gameState.KarmaSystem.GetFormName();
            
            // 第一行：章节和形态
            Console.Write($"│ 章节: {chapterName}");
            int spaces1 = 60 - Encoding.GetEncoding("GB18030").GetByteCount(chapterName) - 
                          Encoding.GetEncoding("GB18030").GetByteCount(formName);
            Console.Write(new string(' ', Math.Max(1, spaces1)));
            Console.WriteLine($"形态: {formName} │");
            
            // 第二行：属性值
            string karmaStr = $"业力: {player.Karma}/{player.MaxStat}";
            string compassionStr = $"慈悲: {player.Compassion}/{player.MaxStat}";
            string healthStr = $"生命: {player.Health}/{player.MaxHealth}";
            string spiritStr = $"灵力: {player.Spirit}/{player.MaxSpirit}";
            
            Console.Write($"│ {karmaStr}");
            int spaces2 = 30 - karmaStr.Length;
            Console.Write(new string(' ', Math.Max(1, spaces2)));
            Console.Write($"{compassionStr}");
            int spaces3 = 30 - compassionStr.Length;
            Console.Write(new string(' ', Math.Max(1, spaces3)));
            Console.WriteLine($"│");
            
            Console.Write($"│ {healthStr}");
            int spaces4 = 30 - healthStr.Length;
            Console.Write(new string(' ', Math.Max(1, spaces4)));
            Console.Write($"{spiritStr}");
            int spaces5 = 30 - spiritStr.Length;
            Console.Write(new string(' ', Math.Max(1, spaces5)));
            Console.WriteLine($"│");
            
            // 第三行：特殊状态
            if (gameState.KarmaSystem.ActiveEffects.Count > 0)
            {
                string effects = "状态: " + string.Join(", ", gameState.KarmaSystem.ActiveEffects);
                Console.Write($"│ {effects}");
                int spaces6 = 71 - Encoding.GetEncoding("GB18030").GetByteCount(effects);
                Console.WriteLine(new string(' ', Math.Max(1, spaces6)) + "│");
            }
            
            Console.WriteLine("└─────────────────────────────────────────────────────────────┘");
        }
        
        /// <summary>
        /// 显示进度条
        /// </summary>
        public static void ShowProgressBar(int current, int max, string label, int barLength = 30)
        {
            double progress = (double)current / max;
            int filled = (int)(progress * barLength);
            int empty = barLength - filled;
            
            Console.Write($"{label}: [");
            Console.Write(new string('█', filled));
            Console.Write(new string('░', empty));
            Console.Write($"] {current}/{max}");
        }
        
        /// <summary>
        /// 绘制边框
        /// </summary>
        public static void DrawBox(string title, int padding = 2)
        {
            string border = "═";
            int width = 70;
            
            Console.WriteLine($"╔{new string(border[0], width - 2)}╗");
            
            // 标题
            Console.WriteLine($"║{new string(' ', (width - 2 - Encoding.GetEncoding("GB18030").GetByteCount(title)) / 2)}{title}{new string(' ', (width - 2 - Encoding.GetEncoding("GB18030").GetByteCount(title)) / 2)}║");
            
            Console.WriteLine($"╚{new string(border[0], width - 2)}╝");
        }
        
        /// <summary>
        /// 显示分隔线
        /// </summary>
        public static void ShowDivider()
        {
            Console.WriteLine("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        }
        
        /// <summary>
        /// 逐字显示文本（打字机效果）
        /// </summary>
        public static void TypeWriter(string text, int delay = 20)
        {
            Console.ForegroundColor = StoryColor;
            foreach (char c in text)
            {
                Console.Write(c);
                Thread.Sleep(delay);
            }
            Console.WriteLine();
            Console.ResetColor();
        }
        
        /// <summary>
        /// 显示故事文本
        /// </summary>
        public static void ShowStoryText(string text, bool typewriter = false)
        {
            Console.WriteLine();
            Console.ForegroundColor = StoryColor;
            
            if (typewriter)
            {
                TypeWriter("    " + text, 15);
            }
            else
            {
                // 自动换行处理
                string[] words = text.Split(' ');
                string line = "    ";
                int lineLength = 60;
                
                foreach (string word in words)
                {
                    if (Encoding.GetEncoding("GB18030").GetByteCount(line) + 
                        Encoding.GetEncoding("GB18030").GetByteCount(word) > lineLength)
                    {
                        Console.WriteLine(line);
                        line = "    " + word + " ";
                    }
                    else
                    {
                        line += word + " ";
                    }
                }
                Console.WriteLine(line);
            }
            
            Console.ResetColor();
            Console.WriteLine();
        }
        
        /// <summary>
        /// 显示危险/警告文本
        /// </summary>
        public static void ShowDangerText(string text)
        {
            Console.ForegroundColor = DangerColor;
            Console.WriteLine($"  ⚠ {text}");
            Console.ResetColor();
        }
        
        /// <summary>
        /// 显示成功文本
        /// </summary>
        public static void ShowSuccessText(string text)
        {
            Console.ForegroundColor = SuccessColor;
            Console.WriteLine($"  ✓ {text}");
            Console.ResetColor();
        }
        
        /// <summary>
        /// 显示选项
        /// </summary>
        public static void ShowChoices(List<Choice> choices)
        {
            Console.WriteLine();
            for (int i = 0; i < choices.Count; i++)
            {
                Console.ForegroundColor = ChoiceColor;
                Console.WriteLine($"  [{i + 1}] {choices[i].Text}");
                Console.ResetColor();
                
                if (!string.IsNullOrEmpty(choices[i].Hint))
                {
                    Console.ForegroundColor = ConsoleColor.DarkGray;
                    Console.WriteLine($"      提示: {choices[i].Hint}");
                    Console.ResetColor();
                }
            }
            Console.WriteLine();
        }
        
        /// <summary>
        /// 显示章节开始
        /// </summary>
        public static void ShowChapterStart(int chapter, string title)
        {
            Console.Clear();
            
            string[] chapterTitles = {
                "", "第一章", "第二章", "第三章", "第四章", "第五章"
            };
            
            string[] subtitles = {
                "", "轮回之始", "西行歧路", "佛魔试炼", "情镜劫波", "灵山真相"
            };
            
            Console.ForegroundColor = TitleColor;
            Console.WriteLine();
            Console.WriteLine("╔═══════════════════════════════════════════════════════════════╗");
            Console.WriteLine($"║              {chapterTitles[chapter],-12}  {title,-38}║");
            Console.WriteLine($"║                         ～ {subtitles[chapter]} ～                           ║");
            Console.WriteLine("╚═══════════════════════════════════════════════════════════════╝");
            Console.ResetColor();
            
            Console.WriteLine();
            Thread.Sleep(1000);
        }
        
        /// <summary>
        /// 显示结局
        /// </summary>
        public static void ShowEnding(string endingId, string title, string description)
        {
            Console.Clear();
            
            Console.ForegroundColor = DangerColor;
            Console.WriteLine();
            Console.WriteLine("╔═══════════════════════════════════════════════════════════════╗");
            Console.WriteLine("║                                                               ║");
            Console.WriteLine($"║                    【 结 局 ： {title} 】                      ║");
            Console.WriteLine("║                                                               ║");
            Console.WriteLine("╚═══════════════════════════════════════════════════════════════╝");
            Console.ResetColor();
            
            Console.WriteLine();
            Thread.Sleep(500);
            TypeWriter(description, 40);
            
            Console.WriteLine();
            Thread.Sleep(1000);
        }
        
        /// <summary>
        /// 等待按键
        /// </summary>
        public static void WaitForKey()
        {
            Console.WriteLine("\n按任意键继续...");
            Console.ReadKey();
        }
        
        /// <summary>
        /// 确认选择
        /// </summary>
        public static bool Confirm(string message)
        {
            Console.Write($"\n{message} (Y/N): ");
            string input = Console.ReadLine()?.ToUpper() ?? "";
            return input == "Y";
        }
        
        /// <summary>
        /// 显示加载动画
        /// </summary>
        public static void ShowLoading(string message)
        {
            Console.Write($"\n{message}");
            for (int i = 0; i < 3; i++)
            {
                Thread.Sleep(300);
                Console.Write(".");
            }
            Console.WriteLine();
        }
    }
}
