using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace BlackMythTrip
{
    /// <summary>
    /// 存档系统
    /// </summary>
    public static class SaveSystem
    {
        private const string SaveFileName = "save.json";
        private const string AchievementFileName = "achievements.json";
        
        /// <summary>
        /// 保存游戏
        /// </summary>
        public static bool SaveGame(GameState gameState)
        {
            try
            {
                gameState.LastSaveTime = DateTime.Now;
                
                var json = gameState.Serialize();
                File.WriteAllText(SaveFileName, json);
                
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"\n游戏已保存到: {Path.GetFullPath(SaveFileName)}");
                Console.ResetColor();
                
                return true;
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"\n保存失败: {ex.Message}");
                Console.ResetColor();
                return false;
            }
        }
        
        /// <summary>
        /// 加载游戏
        /// </summary>
        public static GameState LoadGame()
        {
            try
            {
                if (!File.Exists(SaveFileName))
                {
                    Console.WriteLine("存档文件不存在，创建新游戏...");
                    return new GameState();
                }
                
                var json = File.ReadAllText(SaveFileName);
                var gameState = GameState.Deserialize(json);
                
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine($"\n游戏已加载!");
                Console.WriteLine($"存档时间: {gameState.LastSaveTime}");
                Console.WriteLine($"当前章节: {gameState.GetCurrentChapterName()}");
                Console.ResetColor();
                
                return gameState;
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"\n加载失败: {ex.Message}");
                Console.ResetColor();
                return new GameState();
            }
        }
        
        /// <summary>
        /// 检查存档是否存在
        /// </summary>
        public static bool SaveExists()
        {
            return File.Exists(SaveFileName);
        }
        
        /// <summary>
        /// 删除存档
        /// </summary>
        public static bool DeleteSave()
        {
            try
            {
                if (File.Exists(SaveFileName))
                {
                    File.Delete(SaveFileName);
                    return true;
                }
                return false;
            }
            catch
            {
                return false;
            }
        }
        
        /// <summary>
        /// 保存成就（独立文件）
        /// </summary>
        public static void SaveAchievements(List<string> achievements)
        {
            try
            {
                var json = JsonSerializer.Serialize(achievements, new JsonSerializerOptions { WriteIndented = true });
                File.WriteAllText(AchievementFileName, json);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"成就保存失败: {ex.Message}");
            }
        }
        
        /// <summary>
        /// 加载成就
        /// </summary>
        public static List<string> LoadAchievements()
        {
            try
            {
                if (File.Exists(AchievementFileName))
                {
                    var json = File.ReadAllText(AchievementFileName);
                    return JsonSerializer.Deserialize<List<string>>(json) ?? new List<string>();
                }
            }
            catch
            {
                // 忽略错误
            }
            return new List<string>();
        }
        
        /// <summary>
        /// 显示已解锁成就
        /// </summary>
        public static void DisplayAchievements()
        {
            Console.Clear();
            Console.WriteLine();
            Console.WriteLine("╔═══════════════════════════════════════════════════════════════╗");
            Console.WriteLine("║                    【 已解锁成就 】                        ║");
            Console.WriteLine("╚═══════════════════════════════════════════════════════════════╝");
            
            var achievements = LoadAchievements();
            
            if (achievements.Count == 0)
            {
                Console.WriteLine("\n  尚未解锁任何成就......");
                Console.WriteLine("  完成不同的结局可解锁成就！");
            }
            else
            {
                foreach (var achievement in achievements)
                {
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.WriteLine($"\n  ★ {achievement}");
                    Console.ResetColor();
                }
                
                Console.WriteLine($"\n  共解锁 {achievements.Count} 个成就");
            }
            
            // 检查二周目数据
            if (File.Exists("newgame_plus.json"))
            {
                Console.WriteLine();
                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.WriteLine("  ★ 二周目数据已解锁！");
                Console.WriteLine("  下次开始新游戏时可继承部分数据。");
                Console.ResetColor();
            }
        }
        
        /// <summary>
        /// 获取存档信息
        /// </summary>
        public static void ShowSaveInfo()
        {
            if (File.Exists(SaveFileName))
            {
                var fileInfo = new FileInfo(SaveFileName);
                Console.WriteLine($"\n存档信息:");
                Console.WriteLine($"  文件大小: {fileInfo.Length} 字节");
                Console.WriteLine($"  修改时间: {fileInfo.LastWriteTime}");
            }
            else
            {
                Console.WriteLine("\n暂无存档");
            }
        }
        
        /// <summary>
        /// 自动保存
        /// </summary>
        public static void AutoSave(GameState gameState)
        {
            // 自动保存到备份文件
            try
            {
                gameState.LastSaveTime = DateTime.Now;
                var json = gameState.Serialize();
                File.WriteAllText("autosave.json", json);
            }
            catch
            {
                // 忽略自动保存错误
            }
        }
    }
}
