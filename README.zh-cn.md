# WidgetMarkup

[English](README.md) | **简体中文**

在 Unreal 引擎中利用 XML 搭建 UMG 控件树、创建蓝图，并结合 Python 实现响应式 UI。

![](Documents/WidgetMarkup.gif)

## 快速开始

1. 在 Unreal 编辑器中启用 WidgetMarkup 插件。
2. 在项目内容源中准备一个 `*.widgetmarkup` 文件。
3. 在编辑器 CMD 输入栏执行：

```txt
WidgetMarkup.Show /Game/WidgetMarkup/Example
```

4. 预览窗口会打开，并在源文件变化时自动刷新。

## XML 语法

### 控件蓝图

```xml
<WidgetBlueprint Script="Samples.MyComponent">
  <WidgetTree>
    <CanvasPanel>
      <TextBlock Text="Hello" Slot.bAutoSize="True"
        Slot.LayoutData.Anchors.Minimum="0.5,0.5"
        Slot.LayoutData.Anchors.Maximum="0.5,0.5"
        Slot.LayoutData.Alignment="0.5,0.5" />
    </CanvasPanel>
  </WidgetTree>
</WidgetBlueprint>
```

- `Script` — Python 组件模块路径。
- `Super` — 父类（控件蓝图默认为 `UWidgetMarkupUserWidget`）。
- `Implements` — 逗号分隔的 UInterface 名称。

### 普通蓝图与变量

```xml
<Blueprint Super="Actor">
  <Variable Name="Health" Type="Float" Default="100.0" />
  <Variable Name="Name" Type="String" Default="Player" />
</Blueprint>
```

变量类型使用 PascalCase：`Boolean`、`Integer`、`Float`、`Double`、`String`、`Text`、`Name`。
对象引用：`Object(Actor)`、`Class(Actor)`、`SoftObject(Texture2D)`。

`Default` 值的字符串格式与**控件属性完全一致**（向量用逗号分隔、颜色用 `#RRGGBBAA`、枚举用枚举名等）：

```xml
<Variable Name="Scale" Type="Vector2D" Default="0.5,0.5" />
<Variable Name="Tint" Type="LinearColor" Default="1,0.15,0.15,1" />
<Variable Name="HAlign" Type="EHorizontalAlignment" Default="HAlign_Center" />
```

容器（`Array(Float)`、`Set(String)`、`Map(String,Integer)`）的默认值**一律使用子元素**，Map 用 `<Pair>` 表示键值对：

```xml
<Variable Name="Weights" Type="Array(Float)">
  <Float>1.0</Float>
  <Float>2.0</Float>
</Variable>
<Variable Name="Scores" Type="Map(String,Integer)">
  <Pair Key="a" Value="1" />
  <Pair Key="b" Value="2" />
</Variable>
```

### Python 组件

```python
from WidgetMarkupComponent import WidgetMarkupComponent, reactive

class MyComponent(WidgetMarkupComponent):
    @reactive
    def count(self):
        return 0

    def on_button_clicked(self):
        self.count += 1
```

通过 `{属性名}` 将响应式属性绑定到控件：

```xml
<TextBlock Text="{count}" />
<Button OnClicked="on_button_clicked"><TextBlock Text="+" /></Button>
```

## 用法

```txt
WidgetMarkup.Show <包路径>
```

例：`/Game/WidgetMarkup/Example` → `Example.widgetmarkup`。

实时预览窗口：

![Text Block in Canvas Panel](Documents/TextBlockInCanvasPanel.png)

## 测试

在项目根目录运行测试套件：

```bat
Game\Plugins\WidgetMarkup\Content\Tests\RunTests.bat [Game/YourProject.uproject]
```

未指定项目时默认使用 `Game/Game.uproject`。

**所有修改必须通过测试后方可合入。**

测试套件覆盖控件类型、布局面板、样式表、响应式绑定、计算属性链、列表视图及子组件管理，详见 `Content/Tests/`。

## 示例

`Content/Samples/` 中提供了带 Python 组件的 `.widgetmarkup` 示例文件。

**扫雷** — 9×9 网格，支持左右键同时点击（chord），旗帜计数和响应式计时：

![扫雷](Documents/Minesweeper.png)

**科学计算器** — 样式化按钮、记忆操作和显示绑定：

![科学计算器](Documents/ScientificCalculator.png)

## 语法插件

本项目同时开发了[VSCode插件](https://github.com/9087/Unreal.WidgetMarkup.VSC)，支持简单的语法检查和智能提示。


使用插件前需要在引擎编辑器的CMD输入栏中输入：

```
WidgetMarkup.GenerateIntellisenseData <智能补全数据文件路径>
```

并将`智能补全数据文件路径`配置到插件设置中：

![Extension Settings](Documents/ExtensionSettings.png)

使用VSCode打开*.widgetmarkup文件即可生效：

![VSCode Extension](Documents/Extension.png)