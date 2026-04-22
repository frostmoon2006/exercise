using System;

namespace BlackMythTrip
{
    /// <summary>
    /// 玩家角色
    /// </summary>
    public class Player
    {
        public string Name { get; set; } = "取经人";
        public int Health { get; set; } = 100;  // 生命值
        public int MaxHealth { get; set; } = 100;
        public int Spirit { get; set; } = 50;   // 灵力值
        public int MaxSpirit { get; set; } = 50;
        public int Karma { get; set; } = 0;    // 业力值
        public int Compassion { get; set; } = 0; // 慈悲值
        public int MaxStat { get; set; } = 100;
        
        // 解锁的技能
        public System.Collections.Generic.List<string> Skills { get; set; } = 
            new System.Collections.Generic.List<string>();
        
        /// <summary>
        /// 受伤
        /// </summary>
        public void TakeDamage(int damage)
        {
            Health = Math.Max(0, Health - damage);
        }
        
        /// <summary>
        /// 治疗
        /// </summary>
        public void Heal(int amount)
        {
            Health = Math.Min(MaxHealth, Health + amount);
        }
        
        /// <summary>
        /// 消耗灵力
        /// </summary>
        public bool UseSpirit(int amount)
        {
            if (Spirit >= amount)
            {
                Spirit -= amount;
                return true;
            }
            return false;
        }
        
        /// <summary>
        /// 恢复灵力
        /// </summary>
        public void RecoverSpirit(int amount)
        {
            Spirit = Math.Min(MaxSpirit, Spirit + amount);
        }
    }
}
