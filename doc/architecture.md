# AlphaDict 架构说明

本文面向想读代码或改词典格式的人，说明 AlphaDict 的整体结构：数据怎么存、
索引树怎么长、一次查找走过哪些层、屏幕取词与生词练习如何接进这套结构，
以及 QML 前端如何把结果画出来。

**验证状态**：文中格式与流程结论绝大部分来自阅读当前工作区源码（`src/`）与
既有文档（`doc/front-end_format.txt`、`doc/make_dictionary.txt`、`changelog`）。
二进制格式与工具链已用仓库里的 `debian/alphadict/usr/bin/aldctl` 对
`system/dicts/langdao-ec-gb.aldict` 做过真实往返验证（见 §11.1）；其余章节
（屏幕取词、生词练习、QML 交互）仍属静态阅读，未经运行验证。

源码重新编译当前无法完成：`src/Util.cpp` 依赖 boost（`boost/lexical_cast.hpp`），
当前环境未安装 `/usr/include/boost`。上面那次往返用的是仓库自带二进制。

版本基线：AlphaDict 1.5，`aldctl` 内部版本串为 `2.0`。

---

## 1. 系统概览

一句话：**自定义二进制词典 + 字符索引树 + 双前端（QML / QtWidgets）+ 消息队列驱动的后台查找。**

```
        ┌──────────────────────────────────────────┐
        │  前端                                    │
        │  QmlMainWindow (QML, --enable-qml)       │
        │  MainWindow    (QtWidgets, 兼容路径)      │
        └───────────────┬──────────────────────────┘
                        │ QmlBridge / 直接调用
        ┌───────────────▼──────────────────────────┐
        │  SysMessager（主线程 processMessage）      │
        │  MessageQueue ×2：sysMessageQ / uiMessageQ │
        └───────────────▲──────────────────────────┘
                        │ 任务结果、待办查询
        ┌───────────────┴──────────────────────────┐
        │  DictManager        TaskManager (2 线程)  │
        │  词典槽位/缓存         LookupTask 队列      │
        └──────┬───────────────┬───────────────────┘
               │               │
      ┌────────▼──────┐  ┌─────▼──────────┐  ┌──────────────┐
      │ Aldict        │  │ VocabularyBook │  │ Configure    │
      │ 词典对象+缓存  │  │ + SqliteDB     │  │ + tinyxml2   │
      └────────┬──────┘  └────────────────┘  └──────────────┘
               │
      ┌────────▼──────────────────────────────────┐
      │ AldictDocument：块缓存 + 索引遍历 + 解析    │
      └────────┬──────────────────────────────────┘
               │
      ┌────────▼──────────────────────────────────┐
      │ .aldict 文件                                │
      │ header │ char index │ string index │ data  │
      └───────────────────────────────────────────┘
```

设计取向很明确：**磁盘格式自己定，查找在数据侧完成，进程内只保留少量块缓存**。
词典是只读的，生词本才是可写的（SQLite）。

---

## 2. 代码地图

核心（与界面无关）：

- `src/main.cpp`：入口。`CONFIG_QML` 时用 `QmlMainWindow`，否则 `MainWindow`；
  单实例、高 DPI、语言与配置初始化。
- `src/Application.{h,cpp}`：全局单例 `g_application`，持有两个消息队列、
  `DictManager`、`Configure`、`SysMessager`，管理生命周期。
- `src/DictManager.{h,cpp}`：词典槽位（最多同时打开 8 本）、并行分发查找、
  结果按词典顺序回传、大小写/英文变形回退、索引列表生成。
- `src/TaskManager.{h,cpp}`：固定工作线程池（`MAX_WORK_THREAD = 2`），
  `LookupTask` 在此执行。
- `src/MessageQueue.{h,cpp}` + `src/SysMessager.{h,cpp}`：无锁/加锁队列与
  主线程分发器。所有跨线程结果都走这里，不用 Qt 信号跨线程直连。
- `src/VocabularyBook.{h,cpp}`：生词本业务逻辑（加入、删除、复习计分）。
- `src/db/`：`iDB` 抽象 + `SqliteDB` 实现 + `DBFactory` 选择器。
- `src/dict/aldict/`：词典格式实现。`Aldict` 是词典对象与缓存策略，
  `AldictDocument` 负责块读取、索引遍历与词条解析。
- `src/lib/kary_tree/`：通用 K 叉树容器（`kari_tree` / `kari_tree2`），
  `aldctl` 用它建索引。
- `src/Configure.{h,cpp}`：`configure.xml`（tinyxml2）读写，字号、主题、
  取词开关、词典列表。
- `src/Util.{h,cpp}`、`src/Strings.{h,cpp}`、`src/CharUtil.*`：路径、编码、
  UTF-8 与宽字符转换、界面字符串表。

前端：

- `src/gui/qml/`：`Main.qml` 与页面（词典、生词本、设置、帮助）、
  `Theme.qml` 单例、若干自绘控件（滚动条、开关、按钮、下拉框）。
- `src/gui/qt/`：`QmlMainWindow`（QQuickWidget 宿主）、`MainWindow`（旧
  QtWidgets 界面）、`QmlBridge`（QML 与 C++ 的唯一接口）、各 Model。

工具链：`src/tools/aldctl.cpp`（build/export/info）、`src/tools/aldconv.py`
（第三方数据转换脚本）、`src/win32/`（取词钩子、Windows 实现）。

---

## 3. 运行模型：线程、消息与任务

三条执行线：

1. **主线程**：Qt 事件循环。QML 或 Widgets 在这里渲染；`SysMessager::processMessage()`
   也在这里轮询 `sysMessageQ`，把后台结果转成界面更新。
2. **工作线程（2 个）**：`TaskManager` 从队列取 `Task`。一次查找中，
   每本已打开的词典各自派一个 `LookupTask`，因此两本词典可以真并行，
   第三本排队。
3. **词典 IO**：没有独立线程，读盘发生在工作线程里，由 `AldictDocument`
   的块缓存兜住重复访问。

消息是跨线程的唯一通道。`MessageQueue.h` 中的消息类型：

- 查询类：`MSG_DICT_QUERY`、`MSG_DICT_PENDING_QUERY`、`MSG_DICT_INDEX_QUERY`、
  `MSG_CAPWORD_QUERY`
- 结果类：`MSG_SET_DICTITEMS`、`MSG_SET_CAPWORD_DICTITEM`、`MSG_SET_DICTEN`、
  `MSG_MOVE_DICTITEM`、`MSG_RESET_INDEXLIST`、`MSG_LOOKUP_DONE`
- 控制类：`MSG_RELOAD_DICT`、`MSG_QUIT`

两条队列用途不同：`sysMessageQ` 进 `SysMessager`（后台 → 主线程的通用事件），
`uiMessageQ` 专供界面更新（例如 `MSG_SET_DICTITEMS`）。消息里的堆对象有明确
归属约定：带 `void*` 的结果由接收方释放，注释写在 `DictManager::onAddLookupResult`
与 `MessageQueue` 的 `push` 声明处。

**去重与抢占**：每个词典槽位有 `pending` / `pendingFlag`。如果该槽已有任务在跑，
新的查询不另起线程，只记成 pending；任务结束时发现 pending 非空，就把 pending
重新投递成 `MSG_DICT_PENDING_QUERY`。效果是“输入很快时只保留最后一次查询”，
且不会泄漏任务对象（`onAddLookupResult` 会清零 `task`，失败时置 NULL）。

**结果合并**：结果消息带词典下标（`which`），`onAddLookupResult` 把下标塞进
`DictItemList` 首项，界面按词典顺序拼接，而不是后到的覆盖先到的。

---

## 4. 词典二进制格式 `.aldict`

全部小端（`ald_write_u32` 即 `endian_write_u32_le`）。文件按 **256 字节块**
（`ALD_BLOCK`）组织，块内偏移 0 基，块号 1 基：

```
块 1   ┌────────────────────────────────────┐
       │ header（有效 173 字节）+ 填充        │
块 2   ├────────────────────────────────────┤
       │ 字符索引区：每节点 10 字节          │
       │  ├ wchr[4]        UTF-32 字符       │
       │  ├ location[4]    子表位置/数据位置  │
       │  └ len_content[2] 子节点个数        │
块 N   ├────────────────────────────────────┤
       │ 字符串索引区：变长条目               │
       │  ├ location[4]    data 区字节偏移    │
       │  ├ len_str[1]     字符串字节数       │
       │  └ str[]          不含 NUL           │
块 M   ├────────────────────────────────────┤
       │ 词条数据区：变长                     │
       │  ├ len_word[1] + word                │
       │  ├ len_phon[1] + phonetic           │
       │  └ len_expl[2] + explanation        │
       └────────────────────────────────────┘
```

### 4.1 头部（`struct aldict_header`，173 字节）

- `magic[2]`：`0x77 0x88`
- `h_version[1]`：前端格式版本字符，默认 `'1'`
- `p_date[4]`：出版日期，`day, month, year_lo, year_hi`
- `p_identi[60]`：出版者，NUL 结尾
- `d_version[2]`：词典数据版本，存为 `minor, major`（文本里写 `major.minor`）
- `d_identi[60]`：词典名，NUL 结尾
- `d_entries[4]`：条目数（u32）
- `loc_chrindex[1]`：字符索引起始块号（1 字节，因此字符索引必须落在前 255 块内；
  实际固定为第 2 块，`INDEX_BLOCK_NR`)
- `loc_strindex[4]`、`loc_data[4]`：字符串索引区、数据区起始块号
- `src_lan[15]`、`det_lan[15]`：源/目标语言名，缺省 `any`
- `flags[1]`：目前只定义 `F_DUPLICATEINX (0x01)`

### 4.2 字符索引：树而不是列表

`aldict_charindex` = `wchr[4] + location[4] + len_content[2]`，共 10 字节。
一个节点表示“某前缀路径上的下一个字符”，`len_content` 是它的直接子节点数。

- 普通情况：`location` 指向**下一个字符节点所在下标**（数组下标，不是字节偏移）。
- 子树被“压平”成一个字符串时：`location` 置高位 `F_LOCSTRINX (0x80000000)`，
  低 31 位给出字符串索引**区内**字节偏移；此时 `len_content` 为 0。
- `F_DUPLICATEINX` 用于标记重复词（`wchr = 0` 的哑节点）。

`ALD_ADDR_MAX (0x7FFFFFFF)` 是数据地址上限；`ALD_INVALID_ADDR (0xFFFFFFFF)`
表示空地址。

### 4.3 字符串索引：变长词表

条目 = `location(4) + len_str(1) + str(len_str)`，没有 NUL，读侧按长度比较。
因为一条条目必须完整落在一个块里，`aldctl` 限制 `STRINX_LEN_MAX = 60` 个字符：
`5 + 60×4 ≤ 256`，即按最坏情况（全 4 字节 UTF-8）仍不超块。

跨块处理是格式里最巧的地方：条目放不下时，写侧 `fseek` 到下一块边界；
该块首字节写 0，读侧见 `len_str[0] == 0` 就整块跳过。因此块内可能留空洞，
但读取永远不需要跨块拼接（未实测；由 `check_block_bound` 与 `AldictDocument`
的读取代码对读确认）。

### 4.4 数据区

`u8 len_word + word + u8 len_phon + phonetic + u16 len_expl + explanation`。
上限 `WORD_LEN_MAX = 255`、`PHON_LEN_MAX = 255`、`EXPL_LEN_MAX = 65535`。
**同一词的多个义项在数据区连续存放**，查找命中第一个后按顺序解析，
所以“一词多义”不需要额外链表。

`alias` 不单独存储：别名作为额外索引项，`location` 直接指向原词条数据，
导出时按地址分组还原（`aldctl.cpp` 中 `comment: group by address`）。

### 4.5 `location` 的三种语义

这是读代码最容易混淆的地方，务必分清：

1. **字符索引节点里**：0 基**数组下标**（指向 `aldict_charindex` 数组中另一条）。
2. **字符索引 + `F_LOCSTRINX`**：字符串索引**区内字节偏移**。
3. **字符串索引条目里**：数据**区内字节偏移**。

而 `aldict_header` 里的三个 `loc_*` 都是 **1 基块号**。换算：

```c
字符串索引所在块号 = off / 256 + m_strIndexLoc;   // off 为区内偏移
字符索引位置       = (m_chrIndexLoc - 1) * 256;    // 区内 0 基偏移
数据条目位置       = (m_dataLoc   - 1) * 256 + loc;
```

### 4.6 与文本前端的对应

`aldctl` 的文本格式（`doc/front-end_format.txt`）是这套结构的可编辑视图：
`[header]` 的 `name / publisher / date / version / dictversion / srclang / dstlang`
分别落到 `d_identi / p_identi / p_date / h_version / d_version / src_lan / det_lan`；
每个 `[entry]` 生成一条数据，`word` 与全部 `alias` 各插入一个索引项。
`explanation:` 块原样保存，包含缩进与空行。

---

## 5. 索引树：从词条到可查找结构

### 5.1 为什么是字符树

逐字符下钻让“精确查找”和“前缀查找”共用同一套遍历：前者走到字符耗尽为止，
后者走到前缀末尾再取一个候选。每层只要比较一个 UTF-32 字符，不需要比较整串，
也不需要 B 树那种页分裂。

代价是节点多：一个英文词典每个词都贡献一条路径，公共前缀共享节点。
`aldctl` 因此做两件事压体积：**压平**（把长链合并成字符串索引条目）和
**重复词合并**。

### 5.2 构建流程（`aldctl build`）

1. 逐词条把 `word / phonetic / explanation` 写进临时文件，记下 `pos`，
   该 `pos` 就是将来字符串索引条目里的 `location`。
2. 把 `word`（以及每个 `alias`）插入内存中的 K 叉树，节点值 = `pos`。
   同一 `word` 再次出现（第二个义项）时插 `wchr = 0` 的哑子节点，
   并把全局 `flags` 置 `F_DUPLICATEINX`。
3. `trim_indextree` 自底向上决定哪些子树压平，压平结果写进另一个临时文件
   （字符串索引），返回区内偏移。
4. 把字符索引数组从第 2 块开始写出（`INDEX_BLOCK_NR = 2`）。
5. `fseek` 到块边界，依次附上字符串索引区与数据区。
6. 回填头部，写入条目数与三个区起始块号。

### 5.3 压平判据：`is_in_stringindex`

对每个节点递归统计 `words`（有效 `location` 的节点数）与 `depth`（子树高度），
决定“整棵子树能否用一条字符串索引代替”。任一条件成立就不压：

- 子树为空；
- 直接子节点数 `> STRINX_WORDS_MAX (10)`；
- `depth > (words > 1 ? STRINX_DEPTH_MAX (5) : STRINX_LEN_MAX (60))`。

`words` 与 `depth` 是递归累计值，所以深层子树会先在上层被判定“不压”，
再逐层往下试。注意 60 这个上限：单条路径（`words == 1`）允许长到 60 字符，
正是字符串条目“5 字节头 + 60×4”的上限；一旦有分叉（`words > 1`），深度
压到 5 层，避免一条字符串索引“吞掉”太多分支。这是空间与比较次数的折中：
压平越狠，命中的词要整串比较；不压，则每个字符都要读一个 10 字节节点。

### 5.4 写盘顺序的约束

字符索引必须先写、字符串索引与数据区后写，因为前者的 `location` 依赖后两者
的位置；而头部最后回填，因为要记录后两者的块号。三个区都从块边界开始，
所以任何区之间都有 0~255 字节填充，读取时不需要关心。

---

## 6. 查找

### 6.1 精确查找（`AldictDocument::lookup`）

1. 从索引树根节点开始，每个字符在父节点的连续子节点区间
   `[location, location + len_content)` 上做**二分查找**（`bsearch`，子节点按
   `wchr` 升序排列），命中后下钻。
2. 命中节点带 `F_LOCSTRINX` → 进入字符串索引区读一条（或顺块跳到下一条），
   长度相等时**从尾部逐字节比较**（尾部区分度更高，常见词尾不同）。
3. 字符串索引条目指向数据区，解析出 `word/phon/expl`。
4. 若命中项之后紧邻的是同词的另一义项，继续取；一旦 `word` 不同即停止。

“同词多义连续存放”让 `lookup` 不需要额外遍历，但意味着 `aldctl` 写入时必须
把相同 `word` 的条目相邻输出。

### 6.2 前缀与候选（`lookupCandidate`）

候选在精确查找失败后生成：`lookupCandidate(currentNode, advance, candidate)`
拿到已匹配前缀与失配节点，再用 `IndexStat{start = 0, end = 5}` 限定条数，
沿子树拼出候选词。`stat` 计的是**叶子序号**：`loadIndex` 每访问一个叶子
`++stat->number`，只有 `start <= number < end` 的叶子才收进列表。所以
`end = 5` 是开区间，上限是 5 条候选，与字符数无关。索引面板另走
`getIndexList(start, end, startwith)`，按字符区间批量取词，用于“以某字母开头”
这类列表。

### 6.3 大小写与英文变形回退

`DictManager::lookupIgnoreGrammarCase` 在精确查找失败后依次尝试：

1. 首字母翻转大小写（`A lot of` ↔ `a lot of`，这也是“首字母大写句子”取词
   能命中的原因）；
2. 全小写；
3. 全大写。

再失败则 `lookupIgnoreEnglishGrammar` 做形态学退让，按后缀逐级尝试：

- 1 字符：`s` / `d` / `r` → 去尾
- 2 字符：`'s` `'t` `n't` → 去尾；`es` `ed` `er` `en` `ly` → 去尾
  （`ies → y`、`er → e` 额外再试）
- 3 字符：`'re` `'ll` `'ve` → 去尾；`est` `ing` → 去尾，并试 `+e`、
  `iest → y`
- 双写辅音：`stopped / written / hottest` → 去一个重复字母

这套规则只对英文有效，命中即返回，不做词性判断——简单、可预测，误命中由
用户自行判断。

### 6.4 多词典并行与结果顺序

`DictManager::lookup` 对每本已打开的词典各投一个 `LookupTask`（`which == -1`
时遍历全部）。结果带 `which` 回到主线程，界面按词典在设置里的顺序合并，
而不是按返回先后。`m_loading` 或 `m_dictTotal == 0` 时直接返回，避免在重载
中途读到半初始化的槽位。

### 6.5 自动语言判定

主界面没有“源语言 → 目标语言”下拉框：用户只输入词，方向由输入本身决定。

- `Util::detectLanguage(text, scripts)` 逐码点扫描 UTF-8：CJK 统一表意文字
  （含扩展 A/B、兼容区）→ `chinese`；平假名/片假名 → `japanese`；谚文 →
  `korean`；西里尔 → `russian`；拉丁字母、数字、标点、截断或非法 UTF-8 →
  `english`。畸形字节只前进一个字节，保证不越界、不死循环；ASCII 字节在
  解码前就跳过，英文输入只是一层字节比较。
- 枚举范围收敛到已装词典：`loadDict()` 末尾把已打开词典的源语言并成脚本
  掩码 `m_lanMask`（`english → LAN_LATIN`、`chinese → LAN_CJK`、
  `japanese → LAN_KANA`、`korean → LAN_HANGUL`、`russian → LAN_CYRILLIC`；
  `any` 与无法用字符集区分的语言（`french`、`german` …）直接给
  `LAN_ALL`），`detectLanguage()` 只在这个掩码内作答。只装英汉 + 汉英两本
  时，`こんにちは` 不再返回 `japanese`，而是返回空串，日语/韩语/俄语的区间
  连比较都省掉。
- 空串的语义是“没有词典认领这段文字”，不是错误：`beginQuery()` 见空串就
  不过滤（全量搜索），也不改 `m_curLan`，索引列表与状态栏停在原处。
- 加载与查询解耦：`loadDict()` 打开**所有**启用的词典（上限
  `DICT_MAX_OPEN`），不按配置的方向过滤；过滤发生在 `lookup()` 里，由
  `beginQuery()` 决定本次查询允许用哪些词典（`dictMatchesLan()`：源语言相同，
  比较前把词典声明的源语言折叠成小写，或词典标为 `any`）。这样切换语言不需要重载词典，也就没有重载竞态。
- 若没有任何词典能回答该语言，本次查询不做过滤（退化为全量搜索），而不是
  直接给空结果。
- 主词典（索引列表、状态栏词典名、取词窗）随语言变化：`pickMainDict()` 选
  该语言下的第一本词典，`getIndexList()`/`indexListSize()`/`onClick()`/
  `openDictId()` 都走它。语言变化时 `beginQuery()` 推一条
  `MSG_RESET_INDEXLIST`，界面据此重建索引并刷新状态栏，随后才轮到本次查询
  的结果入队。
- 取词（`QUERY_CAPWORD_FLAG`）不改动主词典，也不过滤：取词窗把各词典的命中
  都列出来。生词本里的词可能是任一语言，所以 `VocabularyBook::add()` 用
  `mainDictIndexFor(word)` 按词本身选词典。
- 两个前端（QML 与旧的 QtWidgets）都没有语言选择框，也没有 `m_autoLang`
  之类的模式开关；旧配置里的 `<srclan>` / `<detlan>` 已不再读写。

---

## 7. 缓存与并发

三层缓存，目的都是减少重复工作：

- **查询结果缓存**：`DCACHE_SIZE_MAX = 10`，`map<string, DictItemList>`；
  满 10 条时 `erase(begin())`，即按键序丢弃第一条——简单，但**不是 LRU**。
- **索引缓存**：`ICACHE_SIZE_MAX = 20`，缓存 `iIndexItem`，同样按键序淘汰。
  词典带 `F_DUPLICATEINX` 时不启用索引缓存：同一个词可能对应多条索引，
  必须回查找确认。
- **块缓存**：`CACHE_SLICE_MAX = 400` 块（约 100 KB），`map<int,void*>`
  以块号为键；插入新块时若已超 400，先释放并删除 `begin()` 那块（最低块号），
  同样不是 LRU。

并发要点：

- 同时打开词典上限 `DICT_MAX_OPEN = 8`；重载时用 `DictManager` 的互斥锁
  锁住整个槽位表，`getIndexList` 也在锁内读，注释明确写了“UI 宁愿等 20 ms
  也不要看到空索引”。
- 任务结果带 `owner` 指针；`onAddLookupResult` 校验 `m_dictOpen[which].task == owner`，
  拒绝已经过期的结果（词典被重载后槽位可能换了主人）。
- 块缓存与 `AldictDocument` 的读操作在多线程下由词典级互斥保护；同一本词典
  的两个查询不会并发进入同一个 `AldictDocument`（单线程语义，靠队列串行化，
  未逐行验证）。

---

## 8. 屏幕取词

取词请求最终都归到 `MSG_CAPWORD_QUERY` → `lookup`（带 `QUERY_CAPWORD_FLAG`）→
`MSG_SET_CAPWORD_DICTITEM` → 取词窗口。文本来源按平台不同：

1. **剪贴板**：`cwsClipboard` 打开时监视剪贴板，复制/划词复制的文本进入查找。
   Linux 下这是主要路径。
2. **Windows 文本钩子**：`src/win32/TextOutHook*.cpp` 用 IAT Hook 拦截
   `TextOut` 一类绘制调用，把被绘制的文本回传（仅 Windows 构建）；
   `cwsSelection` 控制“选中即取词”这类开关。
3. **全局热键**：`X11Util` 用 Xlib 的 `XGrabKey` 抓 `Ctrl+Alt+<字母>`
   （`m_cws.shortcutKey`），Windows 用 `RegisterHotKey`。它的作用是暂停/恢复
   取词，不是取词来源。`forwardHotKey` 想把热键转发给焦点窗口，源码注释
   标着 “Fixed: not working”，当前调用已被注释掉。

命中的结果按 `which` 分条保留，取词窗口只显示第一条词典的释义并允许切换。
`CapWordDialog(autoclose, inv, parent)` 接收配置里的 `AutoCloseEnable`
（是否自动关闭）与 `AutoCloseInterval`（延迟毫秒，默认 10000）。窗口独立于
主窗口，定位在鼠标附近并做屏幕边界收敛。

---

## 9. 生词本与练习

存储：`~/.AlphaDict/vbook.sqlite`。`Configure::getVBPath()` 只返回
`~/.AlphaDict/vbook`，后缀由 `SqliteDB::suffix()` 补。表 `tbl_word`，8 列
`word / expl / count / complexity / date / arg1..arg3`。`VocabularyBook`
的上限是 `VB_CAPACITY = 5000`；`VBookModel` 另有一道 1000 行闸门，
超出后界面不再接受添加。

- `VocabularyBook::add(word)` 先调 `DictManager::lookup(word, 0, ...)` 取
  **第一本词典**的命中释义（`bfind` 为真的项拼接）再落库，所以**离线也能复习**。
  `expl` 是快照，词典更新后不会自动刷新——这是刻意取舍。
- 加载时若 `vbook.sqlite` 不存在，会先建表，再回退读同目录的 `vbook.xml`
  （旧版本遗留格式）。

练习算法（`VBookModel`）：

- `exam`（考试模式）：答对 `complexity -= 1` 并计分，答错 `complexity += 1`
  并留在原词；`complexity` 越高出现越频繁。
- `study`（背诵模式）：同一词连续答对 `repeat` 次（界面可选 1/2/3/5）才通过。
- 界面用星号表示难度：`(complexity + count) / 3`，钳制在 1..10。
- 拼写检查不区分大小写；判定后显示 `✔ Correct` / `✘ Try again` 并着色
  （`Theme.success` / `Theme.danger`）。

`VBookModel` 的 1000 行闸门卡在界面侧，`VocabularyBook`/数据库侧仍是 5000：
两者不一致时以更严的 1000 为准。

---

## 10. 前端：QML 与 QtWidgets

### 10.1 双前端与构建开关

`main.cpp` 用 `CONFIG_QML` 选择实现：

- `CONFIG_QML` 定义 → `QmlMainWindow`（`QQuickWidget` 载 `Main.qml`）。
- 未定义 → `MainWindow`（`.ui` 文件 + `MainWindow.cpp`）。

两套前端共享 `DictManager`、`VocabularyBook`、`Configure` 与同一套消息，
所以功能一致，QML 版是 1.5 的主推界面，Widgets 版是兼容/回退路径。

### 10.2 `QmlBridge`：唯一接口

`QmlBridge` 暴露属性（`Q_PROPERTY`）与槽（`Q_INVOKABLE`/slots），QML 只认它：

- 状态：`expl`、`query`、`capWord`、`indexCount`、`dictCount`、
  `vbook`、`vBookMode`、`vBookWord`、`vBookExpl`、`fontSize`、
  `darkTheme`、`cwsEnabled`、`cwsClipboard`、`cwsSelection`、`shortcutIndex`
- 动作：`search`、`prefixSearch`、`clickIndex`、`pageIndex`、`addToVBook`、
  `toggleVBook`、`vbookStep`、`vbookInput`、`beginWindowMove`、`beginWindowResize`

所有耗时操作经 `DictManager` 投递到工作线程，结果回主线程后由桥接层发
`propertyChanged`/信号，QML 侧永远不阻塞。

### 10.3 页面与主题

`src/gui/qml/`：

- `Main.qml`：无系统标题栏的窗口骨架，侧栏 + 内容区 + 自定义标题按钮。
- `DictionaryPage.qml`：查询框、词典页签、释义卡片、索引列表。
- `VBookPage.qml`：生词本列表、背诵/考试面板、拼写输入。
- `SettingsPage.qml`：字号、深色模式、取词开关、词典顺序。
- `HelpPage.qml`：内置帮助（HTML 渲染，跟主题字号/强调色）。
- `Theme.qml`：单例，`dark: true`、`accent #4f8cff`、`accent2 #7b5cff`、
  `baseSize: 16`（QML 兜底值），派生出 `smallSize`、`tinySize`、
  `textLine`、`controlHeight`。
- 控件：`FlatButton`、`StyledComboBox`、`ThemedSwitch`、`ThemedField`、
  `ThemedScrollBar`、`SettingRow`、`SideBar`、`Card`。

Theme 只在 QML 内部用；C++ 侧真正决定字号的是 `Configure`：
`UI_FONT_SIZE_MIN 12 / MAX 40 / DEFAULT 20`。QML 启动时读 `Configure`
的字号覆盖 `Theme.baseSize`，所以“默认大字”是 20，而不是 QML 里写的 16。

### 10.4 大字布局

1.5 的界面刻意按“大字”排：默认字号 20（普通桌面应用多为 11~13），
侧栏按钮是正方形，让文字占满按钮；卡片用大圆角、宽内边距，释义区
优先占据剩余高度。字号上限 40，下限 12，全部尺寸（图标、行高、控件高度）
都从 `baseSize` 派生，因此调字号不会把布局挤坏。

### 10.5 无边框、单实例、托盘

- 窗口无系统标题栏，最小化/关闭/拖动由 `Main.qml` 绘制并调用
  `beginWindowMove` / `beginWindowResize`。
- 单实例：启动时探测已有进程，存在则唤醒其窗口并退出（1.5 新增）。
- 关闭行为受“系统托盘”设置影响：开启时隐藏到托盘，托盘图标恢复窗口；
  关闭该选项则直接退出。
- 词典目录只接受 `*.aldict`（大小写不敏感），避免 xml 与编辑器备份文件
  出现在词典列表里。

---

## 11. 工具链

`aldctl`（`src/tools/aldctl.cpp`）：

```
aldctl build  [-o out.aldict] file.txt [more.txt ...]
aldctl export [-o out.txt]    dict.aldict
aldctl info                   dict.aldict
```

- `build`：文本 → 二进制。可合并多个输入，不给 `-o` 时输出
  `<第一个文件名>.aldict`。这是生成索引树的唯一入口。
- `export`：二进制 → 文本。先 `getIndexList(indexes, 0, INT_MAX)` 遍历
  索引树，再按数据地址分组：同一地址的第一个索引词是 `word`，其余是
  `alias`。所以导出顺序是**索引树遍历顺序（近似字典序）**，不是数据区
  顺序（实测中数据区首条是 `a`，导出文本首条是 `A AND NOT B gate`）。
  phonetic 里的 `\n` 会拆成多行 `phonetic =`，空行丢弃。
- `info`：打印文件名、名称、出版商、日期、语言、版本、条目数
  （`entries` 为去重后的数据地址数，`index` 为索引条目数）。不打印
  区起始块号。

`build` 的关键常量：`INDEX_BLOCK_NR 2`、`STRINX_WORDS_MAX 10`、
`STRINX_DEPTH_MAX 5`、`STRINX_LEN_MAX 60`、`CHRINX_DEPTH_MIN 1`、
`WORD_LEN_MAX 255`、`PHON_LEN_MAX 255`、`EXPL_LEN_MAX 65535`。

`aldconv.py`（`src/tools/`）：把第三方词典（如 StarDict/CE-DICT 导出的
文本或 TSV）转成上面的文本格式，再交给 `build`。它只做字段映射与清洗，
不碰二进制。

### 11.1 实测往返行为

用 `debian/alphadict/usr/bin/aldctl` 对 `system/dicts/langdao-ec-gb.aldict`
（435468 条）跑过一次往返：

```
info   : 朗道英汉字典5.0 / 2003-08-26 / english -> chinese / 2.4 (format 1)
export : 34,453,898 字节文本，435468 个 [entry]
build  : 32,841,557 字节二进制（原文件 32,406,089）
export : 与原文本 cmp 完全一致
```

- **文本层面幂等**：`export → build → export` 两次文本字节相同。
- **二进制不是逐字节幂等**：重建文件恰好每条多 1 字节
  （32,841,557 − 32,406,089 = 435,468）。原因是 `export` 给不以 `\n`
  结尾的 explanation 补了一个换行，`build` 原样写回；原文件首条 `a` 的
  explanation 尾字节是 `能`，重建后退化为 `\n`。header 的 `entries`、
  三个区起始块号（2 / 18409 / 37931）、版本与日期全部保持一致。
- `phonetic` 在 langdao 里全为空（导出文本 0 行），音标实际内嵌在
  explanation 首行（如 `*[ei]`），所以这部词典的 phonetic 字段是摆设。

---

## 12. 路径、配置与安装

- 用户目录：`Util::usrProfileDir()` → `$HOME/.AlphaDict`（Windows 为配置
  文件目录）。生词本、配置都在这里。
- 配置：`~/.AlphaDict/configure.xml`，首次运行从
  `<DATADIR>/configure.xml.in` 复制。
- 词典目录：`<数据目录>/dicts`，数据目录为 `DATADIR`
  （安装后 `/usr/share/alphadict`）或运行目录下的 `system`（开发时）。
- `make install` 把 `system/dicts/*.aldict` 拷到 `<datadir>/dicts`；
  `make clean` 不删词典目录。

注意 `src/config.h` 是 `configure` 生成物（被 `.gitignore` 忽略），
其中的 `SRCDATADIR` 指向构建机的源码路径，**不要**据此写死路径，也不要
把它当作仓库内容。

---

## 13. 构建

```sh
cd src
./configure --enable-qml      # 需要 Qt 5.9+，Qt6 会被明确拒绝
make -j4
sudo make install             # 装到 /usr/share/alphadict
```

- 只支持 Qt5；`configure` 用 `pkg-config` 定位 Qt，不写死 include/lib 路径。
- `--enable-qml` 自动探测 QtQuick/QML 模块，缺失时退回 QtWidgets。
- 依赖：Qt5 Core/Gui/Widgets（QML 版另需 Quick/Qml）、X11（取词）、
  SQLite（生词本，源码内置 `src/lib/sqlite3`）、tinyxml2（内置）、
  **boost 头文件**（`Util.cpp` 用 `boost/lexical_cast.hpp`，`configure`
  不检查它，缺了到编译 `Util.o` 才报错）。

---

## 14. 设计评价与已知限制

以下是主观评估，不是格式规范。

做得好的：

- **格式自洽**：块对齐 + 区内偏移 + 压平索引，读侧逻辑简单，不需要 mmap
  也能稳定跳过损坏块。
- **查找分层清楚**：字符树负责“找位置”，字符串索引负责“确认”，数据区负责
  “取内容”，每层职责单一。
- **回退策略实用**：大小写翻转 + 英文后缀退让，覆盖了屏幕取词最常见的
  “句首大写 / 复数 / 过去式”。
- **生词本存释义快照**：离线复习、词典换版本都不影响历史记录。

已知限制：

- 字符索引起始块号只有 1 字节，等于把字符索引钉死在前 255 块（约 64 KB）；
  大词典靠压平才勉强够用。
- 压平判据是启发式（深度 5 / 10 个子节点），不保证最优；不同词表体积差异
  可能很大。
- 候选上限硬编码 5 条（`start = 0, end = 5`），且 `loadIndex` 满额即
  `return false` 逐层中断递归，候选只会来自树序靠前的分支，未必是最相关的
  5 个；长前缀下尤其明显。
- 英文变形回退是规则表，不做词形还原，误命中（如 `bus` → `bu`）无法避免。
- 并发模型是“每词典单线程 + 全局锁”，`DICT_MAX_OPEN = 8` 偏小；重载期间
  界面会有可感知停顿。
- 生词本释义是快照，不随词典更新。
- 自动语言判定按字符集分类（§6.5）：凡拉丁字母都算 `english`，所以法语、
  德语词条也会走英文词典；日文里的汉字与中文无法区分，只有含假名时才判为
  `japanese`。装了 `any` 词典或无法按字符集区分的语言（`french` …）时掩码
  退化为全枚举，只装中英两本时则连日语/韩语/俄文都不判定，这些输入直接走
  全量搜索。
- `export → build` 不保二进制等价：每条 explanation 被补一个换行，
  435468 条的 langdao 重建后大 435468 字节（见 §11.1）。重新打包时体积会
  缓慢膨胀，但文本内容不变。
- 部分早于 1.5 的文档（`doc/front-end_format.txt` 中提到 XML 前端）只描述
  历史行为，XML 已移除。

---

## 15. 速查

常量：

```
ALD_BLOCK            256        块大小
ALD_ADDR_MAX         0x7FFFFFFF 数据地址上限
ALD_INVALID_ADDR     0xFFFFFFFF 空地址
F_LOCSTRINX          0x80000000 字符索引指向字符串索引
F_DUPLICATEINX       0x01       存在重复词
INDEX_BLOCK_NR       2          字符索引起始块
STRINX_WORDS_MAX     10         压平：子节点上限
STRINX_DEPTH_MAX     5          压平：分叉子树深度上限
STRINX_LEN_MAX       60         压平：单条路径字符上限
DICT_MAX_OPEN        8          同时打开词典数
CACHE_SLICE_MAX      400        块缓存（×256 B）
DCACHE_SIZE_MAX      10         词典对象缓存
ICACHE_SIZE_MAX      20         索引缓存
VB_CAPACITY          5000       生词本容量
MAX_WORK_THREAD      2          工作线程
UI_FONT_SIZE_*       12/40/20   字号下限/上限/默认
```

术语：

- **字符索引 / char index**：`aldict_charindex` 数组，树的节点。
- **字符串索引 / string index**：压平后的词串表，条目含数据区偏移。
- **压平 / trim**：把子树替换成一条字符串索引。
- **哑节点**：`wchr = 0` 的节点，用来表示“同词的另一义项”。
- **槽位 / slot**：`DictManager` 中 `m_dictOpen[i]`，一本打开的词典及其任务。
- **pending**：槽位上排队的下一次查询，任务结束时重新投递。

---

相关文档：`doc/front-end_format.txt`（文本格式完整规范）、
`doc/make_dictionary.txt`（制作流程）、`doc/dictionary_inner.pdf`（格式设计原文）、
`README.cn` / `README.en`（使用与构建）。
