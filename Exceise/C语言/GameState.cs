using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.Json;
using BlackMythTrip.Systems;

namespace BlackMythTrip
{
    /// <summary>
    /// 游戏状态
    /// </summary>
    public class GameState
    {
        /// <summary>
        /// 当前章节
        /// </summary>
        public int CurrentChapter { get; set; } = 0;
        
        /// <summary>
        /// 当前周目
        /// </summary>
        public int Playthrough { get; set; } = 1;
        
        /// <summary>
        /// 玩家角色
        /// </summary>
        public Player Player { get; set; } = new Player();
        
        /// <summary>
        /// 背包道具
        /// </summary>
        public List<string> Inventory { get; set; } = new List<string>();
        
        /// <summary>
        /// 已解锁技能
        /// </summary>
        public List<string> UnlockedSkills { get; set; } = new List<string>();
        
        /// <summary>
        /// 业力-慈悲系统
        /// </summary>
        public KarmaCompassionSystem KarmaSystem { get; set; } = new KarmaCompassionSystem();
        
        /// <summary>
        /// 蝴蝶效应系统
        /// </summary>
        public ButterflyEffectSystem ButterflySystem { get; set; } = new ButterflyEffectSystem();
        
        /// <summary>
        /// 因果罗盘系统
        /// </summary>
        public KarmaCompassSystem KarmaCompassSystem { get; set; } = new KarmaCompassSystem();
        
        /// <summary>
        /// 多周目记忆继承系统
        /// </summary>
        public MemoryInheritanceSystem MemorySystem { get; set; } = new MemoryInheritanceSystem();
        
        /// <summary>
        /// 解锁的隐藏内容
        /// </summary>
        public HashSet<string> UnlockedSecrets { get; set; } = new HashSet<string>();
        
        /// <summary>
        /// 游戏时间
        /// </summary>
        public DateTime GameStartTime { get; set; } = DateTime.Now;
        
        /// <summary>
        /// 存档时间
        /// </summary>
        public DateTime LastSaveTime { get; set; } = DateTime.Now;
        
        /// <summary>
        /// 获取当前章节名称
        /// </summary>
        public string GetCurrentChapterName()
        {
            string[] names = { "", "轮回之始", "西行歧路", "佛魔试炼", "情镜劫波", "灵山真相" };
            return CurrentChapter >= 1 && CurrentChapter <= 5 ? names[CurrentChapter] : "序章";
        }
        
        /// <summary>
        /// 检查是否拥有道具
        /// </summary>
        public bool HasItem(string item)
        {
            return Inventory.Contains(item);
        }
        
        /// <summary>
        /// 检查是否拥有技能
        /// </summary>
        public bool HasSkill(string skill)
        {
            return UnlockedSkills.Contains(skill);
        }
        
        /// <summary>
        /// 添加道具
        /// </summary>
        public void AddItem(string item)
        {
            if (!Inventory.Contains(item))
                Inventory.Add(item);
        }
        
        /// <summary>
        /// 移除道具
        /// </summary>
        public void RemoveItem(string item)
        {
            Inventory.Remove(item);
        }
        
        /// <summary>
        /// 解锁技能
        /// </summary>
        public void UnlockSkill(string skill)
        {
            if (!UnlockedSkills.Contains(skill))
                UnlockedSkills.Add(skill);
        }
        
        /// <summary>
        /// 序列化存档数据
        /// </summary>
        public string Serialize()
        {
            var options = new JsonSerializerOptions 
            { 
                WriteIndented = true 
            };
            return JsonSerializer.Serialize(this, options);
        }
        
        /// <summary>
        /// 反序列化存档数据
        /// </summary>
        public static GameState Deserialize(string json)
        {
            try
            {
                var state = JsonSerializer.Deserialize<GameState>(json);
                return state ?? new GameState();
            }
            catch
            {
                return new GameState();
            }
        }
    }
}
