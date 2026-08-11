# CONTEXT — RemAbilityUtility 测试引入（已完成）

> 2026-07 批次评审。范围：为 RemAbilityUtility 插件引入测试模块。

## 术语表（Glossary）

| 术语 | 定义 |
|---|---|
| **Tag 事件管理器（FRemScopedAbilityTagEventManager）** | 封装 `RegisterGameplayTagEvent` 的生命周期：注册/覆盖/唯一注册/解除/批量解除/换源；按文档应"离开作用域自动解除"。 |
| **GameplayEvent 管理器（FRemScopedAbilityGameplayEventManager）** | 封装 `GenericGameplayEventCallbacks` 的生命周期，透传 `FGameplayEventData` payload。 |
| **事件类型（EGameplayTagEventType）** | NewOrRemoved（0↔1 显著变化）/ AnyCountChange / AnyCountIncrease / AnyCountDecrease。 |
| **MatchMode（ELogicOperator）** | `HasMatchingGameplayTags` 模板的 All/Any/None 三态匹配参数。 |

## 已确认决策

1. 覆盖层级 = A：Tag 事件管理器 + GameplayEvent 管理器 + statics 模板族实例化（含 `.inl` Query 重载与 `.cpp` `*MatchingQueryTag` 的语义差异锁定）。 *(Q1)*
2. 新建 `RemAbilityUtilityTest` 模块（UncookedOnly，注册进 uplugin）；依赖 Core/CoreUObject/Engine + GameplayTags/GameplayAbilities + RemCommon/RemCommonTest。

## 实施结果（2026-07）

- 3 个 BDD spec（`Rem.AbilityUtility.*`），**27/27 通过**；全量回归：Rem.Camera 95/95、Rem.UniversalData 43/43、Rem.Common 96/96、Rem.Abilities 124/124。
- 夹具：`FRemTestWorld` + 引擎 `UAbilitySystemComponent`（NewObject + RegisterComponent），`AddLooseGameplayTag` / `HandleGameplayEvent` 同步触发。
- **产品修复（测试暴露的真实 bug）**：`FRemScopedAbilityTagEventManager::UnRegisterEvent(Tag, EventType)` 无条件移除本地 handle——即使 ASC 侧解除失败——导致文档承诺的 `UnRegisterEvent(Tag)` 回退（NewOrRemoved 失败 → 重试 AnyCountChange）**永远失效**，且失败后 ASC 绑定悬空。改为仅成功时移除 handle；`FRemScopedAbilityGameplayEventManager` 同模式修复。
- **锁定的引擎行为**（非本模块 bug）：UE 5.8 的 `FGameplayTagCountContainer` 对**非显著移除**（count 2→1）不广播 AnyCountChange（显著移除 1→0 才广播），而**非显著添加**（1→2）会广播——广播不对称；测试锁定该行为。
- 锁定的管理器语义：`SetAbilitySystem` 换源会解除全部旧绑定且**不迁移**绑定（需重新注册）；`UnRegisterEvent` 的 `||` 回退依赖上述修复。
- 锁定的 statics 语义：`.inl` 的 Query 重载走真 `IsTagQueryMatches`（MatchMode 被忽略）；`.cpp` 的 `HasAny/AllMatchingQueryTag` 走 `GetGameplayTagArray` 遍历（空 tag 列表视为匹配）。
- 运行方式：DebugGame Editor 无头 `Automation RunTests Rem.AbilityUtility`（`-DisablePlugins=CrystalNodes`）。
