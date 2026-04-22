using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace BlackMythTrip
{
    /// <summary>
    /// 游戏引擎核心
    /// </summary>
    public class GameEngine
    {
        private GameState _gameState;
        private bool _isRunning;
        
        public GameEngine(GameState gameState)
        {
            _gameState = gameState;
            _isRunning = true;
        }
        
        /// <summary>
        /// 开始游戏
        /// </summary>
        public void Start()
        {
            // 如果是新游戏，显示序章
            if (_gameState.CurrentChapter == 0)
            {
                ShowPrologue();
                _gameState.CurrentChapter = 1;
            }
            
            // 主循环
            while (_isRunning && _gameState.CurrentChapter <= 5)
            {
                PlayChapter(_gameState.CurrentChapter);
                
                if (_gameState.CurrentChapter >= 5)
                {
                    // 触发结局判定
                    TriggerEnding();
                    _isRunning = false;
                }
                else
                {
                    // 检查是否要继续
                    if (!UI.Confirm("继续下一章节"))
                    {
                        SaveSystem.SaveGame(_gameState);
                        _isRunning = false;
                    }
                    else
                    {
                        _gameState.CurrentChapter++;
                    }
                }
            }
        }
        
        /// <summary>
        /// 显示序章
        /// </summary>
        private void ShowPrologue()
        {
            Console.Clear();
            UI.ShowTitle();
            
            Console.WriteLine();
            UI.ShowStoryText("大唐贞观二十三年，长安城。", true);
            Thread.Sleep(500);
            UI.ShowStoryText("玄奘法师译毕《大般若经》六百卷，于译经阁设坛庆贺。", true);
            Thread.Sleep(500);
            UI.ShowStoryText("然而，就在他提笔写下最后一个字时——", true);
            Thread.Sleep(500);
            UI.ShowStoryText("经卷突然渗出金色的血液，凝成四字偈语：", true);
            
            Console.ForegroundColor = ConsoleColor.DarkRed;
            Console.WriteLine();
            Console.WriteLine("                         【 真 经 铸 轮 回 】");
            Console.ResetColor();
            Thread.Sleep(1000);
            
            UI.ShowStoryText("当夜，译经阁无故失火，四周地涌黑莲。", true);
            UI.ShowStoryText("三藏法师蹊跷圆寂，留下无数未解之谜。", true);
            
            Thread.Sleep(500);
            UI.ShowStoryText("三百载后......", true);
            Thread.Sleep(1000);
            
            UI.ShowStoryText("轮回转世的取经人降生于长安城。", true);
            UI.ShowStoryText("师父说：\"天命取经，此乃你之宿命。\"", true);
            UI.ShowStoryText("然而，在剃度当日——", true);
            
            Console.ForegroundColor = ConsoleColor.DarkRed;
            Console.WriteLine();
            Console.WriteLine("    你目睹佛殿壁画渗血，画中菩萨双目泣泪！");
            Console.WriteLine("    经匣中爬出白骨手臂，将师父拖入虚空！");
            Console.ResetColor();
            
            Thread.Sleep(1000);
            
            UI.ShowStoryText("观音大士现出身形，点化于你：", true);
            
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine();
            Console.WriteLine("    \"取经人，汝之师已入轮回。唯有西行取经，");
            Console.WriteLine("     方能寻得真相，解开这永恒枷锁。\"");
            Console.WriteLine("    \"但需谨记——所见未必为真，所闻未必为实。\"");
            Console.ResetColor();
            
            Thread.Sleep(1000);
            UI.ShowStoryText("于是，你踏上了这条西行之路......", true);
            
            UI.WaitForKey();
        }
        
        /// <summary>
        /// 播放章节
        /// </summary>
        private void PlayChapter(int chapter)
        {
            UI.ShowChapterStart(chapter, "天命取经");
            
            switch (chapter)
            {
                case 1:
                    Chapter1.Play(_gameState);
                    break;
                case 2:
                    Chapter2.Play(_gameState);
                    break;
                case 3:
                    Chapter3.Play(_gameState);
                    break;
                case 4:
                    Chapter4.Play(_gameState);
                    break;
                case 5:
                    Chapter5.Play(_gameState);
                    break;
            }
        }
        
        /// <summary>
        /// 触发结局判定
        /// </summary>
        private void TriggerEnding()
        {
            var karma = _gameState.Player.Karma;
            var compassion = _gameState.Player.Compassion;
            var butterfly = _gameState.ButterflySystem;
            var memory = _gameState.MemorySystem;
            var compass = _gameState.KarmaCompassSystem;
            
            // 检查是否有前世记忆碎片
            bool hasMemoryFragments = compass.CollectedFragments.Count >= 5;
            
            // 检查是否摧毁了所有圣物
            bool destroyedAllHolyItems = butterfly.HasTriggered("burn_ginseng_tree") &&
                                        butterfly.HasTriggered("smash_mirror");
            
            // 检查是否完成地仙阵营任务
            bool completedDiXian = butterfly.HasTriggered("free_zhenyuanzi_soul");
            
            // 判定结局
            string endingId;
            string endingTitle;
            string endingDesc;
            
            if (karma < 30 && hasMemoryFragments)
            {
                // 梵天涅槃
                endingId = "nirvana";
                endingTitle = "梵天涅槃";
                endingDesc = "你将九世积累的功德化作业火，";
                endingDesc += "焚烧真经的同时也焚毁自身存在。\n\n";
                endingDesc += "转轮崩塌瞬间，世间所有寺庙的佛像齐声哀嚎，";
                endingDesc += "额间显现与你相同的\"破戒\"烙印。\n\n";
                endingDesc += "轮回枷锁终于出现裂痕。";
                endingDesc += "而新的取经人，将在百年后再度觉醒......";
                
                memory.UnlockAchievement("梵天涅槃 - 业力低于30并集齐记忆碎片");
            }
            else if (!hasMemoryFragments && !butterfly.HasTriggered("accept_skull_beads"))
            {
                // 永劫轮回
                endingId = "samsara";
                endingTitle = "永劫轮回";
                endingDesc = "你未能觉醒任何前世记忆。\n\n";
                endingDesc += "灵山的诸佛满意地笑了。";
                endingDesc += "又一个完美的\"经炉\"将投入转轮，";
                endingDesc += "为他们的不朽功德添砖加瓦......\n\n";
                endingDesc += "第十一世取经轮回，即将开始。";
                
                memory.UnlockAchievement("永劫轮回 - 未激活任何前世记忆");
            }
            else if (karma >= 70 && compassion >= 70)
            {
                // 三界共主
                endingId = "supreme";
                endingTitle = "三界共主";
                endingDesc = "你同时获得了修罗道的力量与佛光的庇护。\n\n";
                endingDesc += "以自身为媒介，你融合了佛道两家的精髓，";
                endingDesc += "创立了全新的秩序。\n\n";
                endingDesc += "诸天神佛俯首称臣，妖魔万类共尊你为——";
                endingDesc += "三界至尊。\n\n";
                endingDesc += "然而，这真的是解脱吗？";
                
                memory.UnlockAchievement("三界共主 - 业力与慈悲均超过70");
            }
            else if (destroyedAllHolyItems)
            {
                // 无间行者
                endingId = "anralk";
                endingTitle = "无间行者";
                endingDesc = "你摧毁了所有人间的宗教圣物。\n\n";
                endingDesc += "天道紊乱，三界大乱。";
                endingDesc += "妖魔与人族全面战争，";
                endingDesc += "血染大地，尸横遍野......\n\n";
                endingDesc += "你成为了游走在无间地狱的孤魂，";
                endingDesc += "永无止境，永不停歇。";
                
                memory.UnlockAchievement("无间行者 - 摧毁所有宗教圣物");
            }
            else
            {
                // 默认结局：因果循环
                endingId = "karma";
                endingTitle = "因果循环";
                endingDesc = "你既未能打破轮回，也未能成为主宰。\n\n";
                endingDesc += "但你的选择，已经在这片土地上留下了痕迹。";
                endingDesc += "终有一日，";
                endingDesc += "这些痕迹会汇聚成足以改变一切的力量......\n\n";
                endingDesc += "故事并未结束。";
                endingDesc += "只是暂时告一段落。";
            }
            
            // 显示结局
            UI.ShowEnding(endingId, endingTitle, endingDesc);
            
            // 记录完成数据
            var choices = butterfly.TriggeredEvents.Select(e => $"{e.Chapter}_{e.EventId}").ToList();
            memory.RecordPlaythrough(endingId, new Dictionary<string, object>
            {
                { "KarmaPeak", karma },
                { "CompassionPeak", compassion },
                { "ChaptersCompleted", 5 }
            }, choices);
            
            // 显示统计
            ShowEndingStats();
            
            // 显示返回菜单提示
            Console.WriteLine("\n按任意键返回主菜单...");
            Console.ReadKey();
        }
        
        /// <summary>
        /// 显示结局统计
        /// </summary>
        private void ShowEndingStats()
        {
            Console.WriteLine();
            UI.ShowDivider();
            Console.WriteLine("【 本周目统计 】");
            Console.WriteLine($"  业力峰值: {_gameState.Player.Karma}");
            Console.WriteLine($"  慈悲峰值: {_gameState.Player.Compassion}");
            Console.WriteLine($"  蝴蝶效应事件: {_gameState.ButterflySystem.TriggeredEvents.Count}个");
            Console.WriteLine($"  记忆碎片: {_gameState.KarmaCompassSystem.CollectedFragments.Count}片");
            Console.WriteLine($"  收集道具: {_gameState.Inventory.Count}件");
            Console.WriteLine($"  解锁技能: {_gameState.UnlockedSkills.Count}个");
            UI.ShowDivider();
        }
        
        /// <summary>
        /// 处理选择
        /// </summary>
        public void HandleChoice(Choice choice)
        {
            // 应用数值变化
            if (choice.KarmaChange.HasValue)
                _gameState.KarmaSystem.AddKarma(choice.KarmaChange.Value);
            if (choice.CompassionChange.HasValue)
                _gameState.KarmaSystem.AddCompassion(choice.CompassionChange.Value);
            
            // 处理生命值变化
            if (choice.HealthChange.HasValue)
                _gameState.Player.Health = Math.Max(0, 
                    Math.Min(_gameState.Player.MaxHealth, 
                        _gameState.Player.Health + choice.HealthChange.Value));
            
            // 处理灵力变化
            if (choice.SpiritChange.HasValue)
                _gameState.Player.Spirit = Math.Max(0,
                    Math.Min(_gameState.Player.MaxSpirit,
                        _gameState.Player.Spirit + choice.SpiritChange.Value));
            
            // 添加道具
            foreach (var item in choice.GainItems)
            {
                if (!_gameState.Inventory.Contains(item))
                    _gameState.Inventory.Add(item);
            }
            
            // 移除道具
            foreach (var item in choice.LoseItems)
            {
                _gameState.Inventory.Remove(item);
            }
            
            // 解锁技能
            foreach (var skill in choice.UnlockSkills)
            {
                if (!_gameState.UnlockedSkills.Contains(skill))
                    _gameState.UnlockedSkills.Add(skill);
            }
            
            // 触发蝴蝶效应
            if (!string.IsNullOrEmpty(choice.EventId))
            {
                _gameState.ButterflySystem.RecordEvent(
                    _gameState.CurrentChapter,
                    choice.EventId,
                    choice.Text,
                    choice.KarmaChange,
                    choice.CompassionChange,
                    choice.Consequences);
            }
            
            // 显示后果描述
            if (!string.IsNullOrEmpty(choice.ConsequenceText))
            {
                Console.WriteLine();
                Console.ForegroundColor = ConsoleColor.DarkYellow;
                UI.TypeWriter(choice.ConsequenceText, 20);
                Console.ResetColor();
            }
            
            // 检查破戒
            if (choice.TriggerBrokenPrecepts)
            {
                _gameState.KarmaSystem.TriggerBrokenPrecepts(
                    "你做出了违背慈悲的选择......");
            }
            
            // 特殊效果
            if (choice.SpecialEffect != null)
            {
                choice.SpecialEffect(_gameState);
            }
            
            // 消耗道具使用
            if (choice.RequiredItem != null && _gameState.Inventory.Contains(choice.RequiredItem))
            {
                _gameState.Inventory.Remove(choice.RequiredItem);
            }
            
            Console.WriteLine();
        }
        
        /// <summary>
        /// 暂停游戏
        /// </summary>
        public void Pause()
        {
            _isRunning = false;
        }
        
        /// <summary>
        /// 继续游戏
        /// </summary>
        public void Resume()
        {
            _isRunning = true;
        }
    }
}
