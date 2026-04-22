using System;
using System.Collections.Generic;

namespace BlackMythTrip
{
    /// <summary>
    /// 选择项
    /// </summary>
    public class Choice
    {
        /// <summary>
        /// 选项文本
        /// </summary>
        public string Text { get; set; }
        
        /// <summary>
        /// 提示信息
        /// </summary>
        public string Hint { get; set; }
        
        /// <summary>
        /// 事件ID（用于蝴蝶效应追踪）
        /// </summary>
        public string EventId { get; set; }
        
        /// <summary>
        /// 业力变化
        /// </summary>
        public int? KarmaChange { get; set; }
        
        /// <summary>
        /// 慈悲变化
        /// </summary>
        public int? CompassionChange { get; set; }
        
        /// <summary>
        /// 生命值变化
        /// </summary>
        public int? HealthChange { get; set; }
        
        /// <summary>
        /// 灵力值变化
        /// </summary>
        public int? SpiritChange { get; set; }
        
        /// <summary>
        /// 获得道具
        /// </summary>
        public List<string> GainItems { get; set; } = new List<string>();
        
        /// <summary>
        /// 失去道具
        /// </summary>
        public List<string> LoseItems { get; set; } = new List<string>();
        
        /// <summary>
        /// 需求道具
        /// </summary>
        public string RequiredItem { get; set; }
        
        /// <summary>
        /// 解锁技能
        /// </summary>
        public List<string> UnlockSkills { get; set; } = new List<string>();
        
        /// <summary>
        /// 触发破戒状态
        /// </summary>
        public bool TriggerBrokenPrecepts { get; set; }
        
        /// <summary>
        /// 后果描述
        /// </summary>
        public string ConsequenceText { get; set; }
        
        /// <summary>
        /// 后果列表
        /// </summary>
        public List<string> Consequences { get; set; } = new List<string>();
        
        /// <summary>
        /// 特殊效果（回调函数）
        /// </summary>
        public Action<GameState> SpecialEffect { get; set; }
        
        /// <summary>
        /// 需要的最小业力值
        /// </summary>
        public int? MinKarma { get; set; }
        
        /// <summary>
        /// 需要的最小慈悲值
        /// </summary>
        public int? MinCompassion { get; set; }
        
        /// <summary>
        /// 需要的道具列表
        /// </summary>
        public List<string> RequiredItems { get; set; } = new List<string>();
        
        /// <summary>
        /// 需要的技能列表
        /// </summary>
        public List<string> RequiredSkills { get; set; } = new List<string>();
        
        /// <summary>
        /// 检查是否可以选择
        /// </summary>
        public bool CanChoose(GameState gameState)
        {
            if (MinKarma.HasValue && gameState.Player.Karma < MinKarma.Value)
                return false;
            if (MinCompassion.HasValue && gameState.Player.Compassion < MinCompassion.Value)
                return false;
            if (!string.IsNullOrEmpty(RequiredItem) && !gameState.Inventory.Contains(RequiredItem))
                return false;
            foreach (var item in RequiredItems)
                if (!gameState.Inventory.Contains(item))
                    return false;
            foreach (var skill in RequiredSkills)
                if (!gameState.UnlockedSkills.Contains(skill))
                    return false;
            return true;
        }
    }
}
