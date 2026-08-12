---
name: widgetmarkup-develop
user-invocable: true
description: 'WidgetMarkup is an Unreal Engine 5 plugin providing a declarative XML-based UI framework with Python scripting. Use this skill when the user wants to: write .widgetmarkup XML files to define UMG widget blueprints; write Python components (WidgetMarkupComponent subclasses) for UI logic; set up reactive data bindings between Python properties and UMG widgets; create ListView/TileView entries with IUserObjectListEntry/IUserListEntry mixins; create regular (non-widget) Blueprints with variables; use style sheets; debug WidgetMarkup compilation or binding issues. When NOT to use: general UE5 C++ development, standard UMG widget coding outside WidgetMarkup.'
---

# WidgetMarkup Development

## 1. Blueprint Creation

### 1.1 Widget Blueprint

A Widget Blueprint (`.widgetmarkup`) compiles to a `UWidgetBlueprint` with a `WidgetTree`. It has a mandatory `Script` attribute pointing to the Python component module.

```xml
<WidgetBlueprint Script="Samples.MyComponent">
  <WidgetTree>
    <CanvasPanel>
      <TextBlock Text="Hello" />
    </CanvasPanel>
  </WidgetTree>
</WidgetBlueprint>
```

The `Script` attribute is a Python import path resolved against the **project's** `Content/Python/` directory:

| Path | Purpose |
|---|---|
| `<ProjectDir>/Content/Python/` | **Your Python components** — create/edit files here |
| `<ProjectDir>/Plugins/WidgetMarkup/Content/Python/` | Framework code (`WidgetMarkupComponent.py`, `ObservableCollection.py`) and test cases — do NOT put your files here |

When the same module name exists in both paths, the project-path copy takes precedence — the plugin copy is silently ignored. For example, `Script="Calculator"` imports `<ProjectDir>/Content/Python/Calculator.py`.

> **Finding the project directory:** The project root is NOT always `Game/`. Look for the `.uproject` file to determine the actual project directory. If you cannot find it, ask the user.
> ```powershell
> Get-ChildItem -Recurse -Filter "*.uproject" | Select-Object -First 1 DirectoryName
> ```

`.widgetmarkup` XML blueprints also live in the **project's** `Content/` directory — not in the plugin. The plugin directory is reserved for samples and test cases (both under the plugin's `Content/`).

> **Path prefix matters:** Project blueprints are referenced with `/Game/` (e.g. `/Game/ScientificCalculator`), while plugin samples use `/WidgetMarkup/` (e.g. `/WidgetMarkup/Samples/Counter`). See [§3.3 Class Path Resolution](#33-class-path-resolution) for details.

The parent class defaults to `UWidgetMarkupUserWidget`. Override with `Super` (must be a subclass of `UWidgetMarkupUserWidget`). Add interfaces with `Implements`:
```xml
<WidgetBlueprint Script="Samples.MyComponent"
                 Implements="MyInterface,OtherInterface" />
```

### 1.2 Regular (Non-Widget) Blueprint

A regular Blueprint has no WidgetTree. Use `<Blueprint>` instead of `<WidgetBlueprint>`:

```xml
<Blueprint Super="Actor" Implements="AbilitySystemInterface">
  <Variable Name="Health" Type="Float" Default="100.0" />
  <Variable Name="Name" Type="String" Default="Player" />
</Blueprint>
```

### 1.3 Blueprint Variables

Variables are defined with `<Variable>` elements. See [docs/blueprint-variables.md](docs/blueprint-variables.md) for complete reference.

**Quick reference** (PascalCase, case-sensitive):

| Category | Types |
|---|---|
| Basic | `Boolean`, `Integer`, `Integer64`, `Float`, `Double`, `Byte`, `String`, `Text`, `Name` |
| Struct | `Vector`, `Vector2D`, `Rotator`, `Transform`, `Color`, `LinearColor` |
| Container | `Array(Float)`, `Set(String)`, `Map(String,Integer)` |
| Object | `Object(Actor)`, `Class(Actor)`, `SoftObject(Texture2D)`, `SoftClass(Actor)` |
| Enum | `ECollisionChannel` (auto-detected) |

**`Default` value syntax (channel A — converter pipeline):**

- Scalars / structs / enums: `Default="..."` is converted via the converter registry — e.g. `Float` → `"100.0"`, `Vector2D` → `"0.5,0.5"`, `LinearColor` → `"1,0.15,0.15,1"`, enum → **full value name** like `HAlign_Center`.
- Objects / soft objects: `Default="/Game/Path"` (asset path).
- **Containers (Array/Set/Map) MUST use child elements** — a `Default` attribute on a container is an error (route A).
- A `Default` attribute combined with child elements is an error.

**Container child elements** (route A — required for containers):

```xml
<Variable Name="Weights" Type="Array(Float)">
  <Float>1.0</Float><Float>2.0</Float><Float>3.0</Float>
</Variable>
<Variable Name="Points" Type="Array(Vector2D)">
  <Vector2D X="1" Y="2" /><Vector2D X="3" Y="4" />
</Variable>
<Variable Name="Tags" Type="Set(String)">
  <String>red</String><String>blue</String>
</Variable>
<Variable Name="Scores" Type="Map(String,Integer)">
  <Pair Key="a" Value="1" /><Pair Key="b" Value="2" />
</Variable>
```

Scalars / structs may also use a single child element instead of `Default`:

```xml
<Variable Name="Offset" Type="Vector2D">
  <Vector2D X="1" Y="2" />
</Variable>
```

> **Enums: always use the full value name** (e.g. `VAlign_Center`, `HAlign_Center`). DisplayName shorthand (`Center` for `VAlign_Center`) is **disabled** — enum matching is value-name exact only.

## 2. WidgetTree & Widgets

### 2.1 Structure

The `<WidgetTree>` element is required inside `<WidgetBlueprint>`. It contains one root widget. The blueprint is auto-compiled after XML processing finishes via `FKismetEditorUtilities::CompileBlueprint`.

**Widget class resolution:** Element names are resolved via `UClass::TryFindTypeSlow` — any non-abstract, non-UserWidget UMG class works.

> **Embedding WidgetMarkup blueprints:** Use `<WidgetMarkup.X.Y>` dot-notation to embed another `.widgetmarkup` file as a child widget. For example, `<WidgetMarkup.Tests.TestChild Name="MyChild" />` compiles `TestChild.widgetmarkup` on demand and embeds the resulting widget tree. The embedded widget's `Script` component is created as usual — `get_child("MyChild")` returns its component. See [python-components.md § Reusable WidgetMarkup Components](docs/python-components.md#reusable-widgetmarkup-components) for details.

| Constraint | Widgets |
|---|---|
| Single child | Button, Border, ScrollBox, SizeBox, ScaleBox |
| Multiple children | CanvasPanel, VerticalBox, HorizontalBox, GridPanel, WrapBox, Overlay, UniformGridPanel |
| No children | TextBlock, Image, ProgressBar, Slider, CheckBox, Spacer, EditableText, ComboBoxString |

### 2.2 Widgets & Slots

> **Any UPROPERTY on the UMG class works as an XML attribute** via `FPropertyChainHandle`, which resolves dot-separated paths through UE's reflection system. Both nested structs (e.g. `Slot.Size.SizeRule`) and object pointers (e.g. `Slot.Row`) are supported — the slot object is created by `OnAddChild` before attributes are processed.\n>\n> **Sub-property syntax:** Dot notation (`Font.Size=\"18\"`) and child-element syntax (`<Font><Size>18</Size></Font>`) both work — they go through the same `FPropertyPathResolver`. Child-element form is preferred for clarity and was validated in test files. For container properties like `ColumnFill` and `RowFill`, child-element syntax is required (these are array properties, not scalar).\n>\n> **Value vs children conflict:** A property element cannot carry both a value and child elements (e.g. `<Size>18<Child/></Size>` is a compile error). Different properties may freely mix forms — `<TextBlock Text="x"><Visibility>Visible</Visibility></TextBlock>` is fine.

**[Shared base properties](docs/widgets/shared-properties.md)** — `Visibility`, `IsEnabled`, `RenderOpacity`, `RenderTransform`, `Cursor`, `ToolTipText` (all widgets).

**Widget reference:**

| Widget | File | Key bindings |
|---|---|---|
| `<TextBlock>` | [textblock.md](docs/widgets/textblock.md) | `Text`, `ColorAndOpacity`, `Font.Size`, `Justification` |
| `<Button>` | [button.md](docs/widgets/button.md) | `OnClicked`, `OnPressed`, `OnReleased`, `OnHovered` |
| `<Border>` | [border.md](docs/widgets/border.md) | `BrushColor`, `Padding`, `OnMouseButtonDownEvent`, `OnMouseButtonUpEvent` |
| `<SizeBox>` | [sizebox.md](docs/widgets/sizebox.md) | `WidthOverride`, `HeightOverride` |
| `<Image>` | [image.md](docs/widgets/image.md) | `Brush.ResourceObject`, `Brush.ImageSize` |
| `<ProgressBar>` | [progressbar.md](docs/widgets/progressbar.md) | `Percent`, `FillColorAndOpacity`, `BarFillType` |
| `<CheckBox>` | [checkbox.md](docs/widgets/checkbox.md) | `IsChecked`, `OnCheckStateChanged` |
| `<Slider>` | [slider.md](docs/widgets/slider.md) | `Value`, `MinValue`, `MaxValue`, `OnValueChanged` |
| `<EditableText>` | [editable-text.md](docs/widgets/editable-text.md) | `Text`, `HintText`, `OnTextChanged` (4 variants) |
| `<ComboBoxString>` | [combobox-string.md](docs/widgets/combobox-string.md) | `DefaultOptions`, `OnSelectionChanged` |
| `<ListView>` / `<TileView>` | [listview.md](docs/widgets/listview.md) | `ListItems`, `EntryWidgetClass` |
| `<Spacer>` | [spacer.md](docs/widgets/spacer.md) | `Size` |
| `<CanvasPanel>` | [canvas-panel.md](docs/widgets/canvas-panel.md) | `Slot.LayoutData.Anchors`, `Slot.LayoutData.Offsets`, `Slot.bAutoSize`, `Slot.ZOrder` |
| `<VerticalBox>` / `<HorizontalBox>` | [vertical-horizontal-box.md](docs/widgets/vertical-horizontal-box.md) | `Slot.Size.SizeRule`, `Slot.Size.Value`, `Slot.Padding`, `Slot.Alignment` |
| `<GridPanel>` | [grid-panel.md](docs/widgets/grid-panel.md) | `Slot.Row`, `Slot.Column`, `Slot.RowSpan`, `Slot.ColumnSpan`, `RowFill`, `ColumnFill` |
| `<UniformGridPanel>` | [uniform-grid-panel.md](docs/widgets/uniform-grid-panel.md) | `Slot.Row`, `Slot.Column` |
| `<WrapBox>` | [wrap-box.md](docs/widgets/wrap-box.md) | `Slot.bFillEmptySpace`, `Slot.bForceNewLine` |
| `<ScrollBox>` | [scroll-box.md](docs/widgets/scroll-box.md) | `Slot.Size.SizeRule`, `Slot.Padding` |
| `<Overlay>` | [overlay.md](docs/widgets/overlay.md) | `Slot.Padding`, `Slot.Alignment` |

> **SizeBox root pattern:** Wrap the WidgetTree root in a `<SizeBox WidthOverride="..." HeightOverride="...">` to give the window a known desired size for auto-sizing. Without this, `UWidgetMarkupWindow` falls back to a 300×200 minimum. All sample blueprints (Minesweeper, ScientificCalculator) use this pattern. Place SizeBox as the direct child of `<WidgetTree>`, then put your normal root widget inside it.

> **GridPanel tip:** Use `<GridPanel>` for grid layouts instead of nesting `<HorizontalBox>` inside `<VerticalBox>`. Children **must** use `Slot.Row` and `Slot.Column` (the `Slot.` prefix is required — bare `Row` causes a parse error because attributes are resolved via `FPropertyPathResolver` on the widget, not on the slot). Set `ColumnFill` and `RowFill` as child elements with `<Float>1.0</Float>` entries for equal sizing:
> ```xml
> <GridPanel>
>   <ColumnFill><Float>1.0</Float><Float>1.0</Float></ColumnFill>
>   <RowFill><Float>1.0</Float><Float>1.0</Float></RowFill>
>   <Button Slot.Row="0" Slot.Column="0" OnClicked="press_1">...</Button>
> </GridPanel>
> ```
> **Slot property timing:** Slot properties (e.g. `Slot.Row`, `Slot.Size.SizeRule`) are resolved AFTER `OnAddChild` creates the slot object (`UPanelWidget::AddChild`), so they work for any panel-managed widget. Root widgets (direct children of `<WidgetTree>`) have no slot — do NOT set `Slot.*` on them.
>
> **Slot vs container properties:** Properties on **child** widgets use `Slot.` prefix (they apply to the slot created by the parent panel). Properties on the **panel itself** (e.g. `RowFill`, `ColumnFill`, `bAutoSize`) use bare names — they belong to the container, not the slot.

**Struct types:** [docs/structs/](docs/structs/) — [FLinearColor](docs/structs/linear-color.md), [FSlateColor](docs/structs/slate-color.md), [FVector2D](docs/structs/vector2d.md), [FMargin](docs/structs/margin.md), [FSlateBrush](docs/structs/slate-brush.md), [FWidgetTransform](docs/structs/render-transform.md), [FSlateChildSize](docs/structs/slate-child-size.md), [FAnchors](docs/structs/anchors.md), [FAnchorData](docs/structs/anchor-data.md), [FSlateFontInfo](docs/structs/font-info.md), [ESlateVisibility](docs/structs/slate-visibility.md), [Alignment](docs/structs/alignment.md)

### 2.3 Naming Widgets

Use `Name="WidgetName"` only when you need to access the widget from Python via `get_editor_property()`. Most widgets (buttons, labels) don't need names — data binding handles updates automatically.

```xml
<!-- Name becomes the Blueprint variable name; needed for Python access or style targeting -->
<TextBlock Name="StatusLabel" Text="{status}" />
<Button OnClicked="on_submit">
  <TextBlock Text="Submit" />
</Button>
```

> **Note:** Button labels use a child `<TextBlock>`, not `Text="..."` on the Button itself. Only set `Name` when you need to reference the widget from Python; otherwise let the system auto-generate names.

Prefer data binding over `get_editor_property()` when possible.

## 3. Data Binding

### 3.1 Property Binding

Use `{source_expression}` to bind a widget property to a Python `@reactive` property:

```xml
<TextBlock Text="{display_value}" />
```

When `self.display_value = "new"` is set in Python, the TextBlock's `Text` updates automatically.

> **Important:** The `{expression}` in XML must match the `@reactive` property name **exactly**. Mismatched names cause `'Component' object has no attribute 'xxx'` at runtime.

Flow:
```
Python: self.display_value = "new"
  → @reactive setter → _notify()
    → on_property_changed("display_value", "new")
      → widget_markup.DataBinding.apply_property_binding(widget, binding, value)
        → C++ applies value to UMG property
```

### 3.2 Delegate Binding

Bind UMG widget events to Python methods:

```xml
<Button OnClicked="on_confirm">
  <TextBlock Text="Confirm" />
</Button>
<Button OnClicked="on_cancel">
  <TextBlock Text="Cancel" />
</Button>
```

Python:
```python
def on_confirm(self):
    self.status = "Confirmed"
    self.save_data()

def on_cancel(self):
    self.status = "Cancelled"
    self.close_panel()
```

> **Note:** `@reactive` setters must assign (e.g., `self.display = "0"`), not mutate in-place (no `self.display += "1"`).

### 3.3 Class Path Resolution

Paths starting with `/` are long package paths resolved via UE's mount point system:

| Prefix | Resolves to | Use for |
|---|---|---|
| `/Game/` | Project `Content/` directory | **Your project blueprints** |
| `/WidgetMarkup/` | Plugin `Content/` directory | Plugin samples and test cases |

Examples:
```xml
<!-- Project blueprint: Game/Content/ScientificCalculator.widgetmarkup -->
<ListView EntryWidgetClass="/Game/ScientificCalculator" ... />

<!-- Plugin sample: Game/Plugins/WidgetMarkup/Content/Samples/ListEntry.widgetmarkup -->
<ListView EntryWidgetClass="/WidgetMarkup/Samples/ListEntry" ... />
```

> **Note:** `/WidgetMarkup/` maps to the plugin's Content directory — for project-level `.widgetmarkup` files, always use `/Game/`.

### 3.4 Property Value Types

Literal (non-binding) values are converted by UE's property system:
- `"Hello"` → FText/FString/FName
- `"True"` / `"False"` → bool
- `"3.14"` → float/double
- `"42"` → int32/int64/byte
- `"/Path/To/Class"` → UClass/UObject reference (via `StaticLoadObject`)
- Colors: `"1,0,0,1"` → FLinearColor; use `unreal.SlateColor()` in Python bindings
- Structs: `"0,0,100"` → FVector; `"4,4,4,4"` → FMargin
- **Enums: use the FULL value name** (e.g. `VAlign_Center`, `HAlign_Center`; `Justification` → `Center` because `ETextJustify`'s value names ARE `Left`/`Center`/`Right`). DisplayName shorthand (`Center` for `VAlign_Center`) is **disabled** — enum matching is value-name exact only.

> **ObjectProperty asset path conversion:** Properties of type `ObjectProperty` (e.g., `Brush.ResourceObject`, `Font.FontObject`) accept asset paths like `"/Game/Textures/MyIcon"` which are loaded via `StaticLoadObject`. If the path fails, and the property is `Font.FontObject` on `FSlateFontInfo`, the system falls back to loading a system font by name (e.g., `Font.FontObject='seguisym'`).

## 4. Python & ListView

See [docs/python-components.md](docs/python-components.md) for the complete Python component development guide, design principles, and ListView setup.

> **Design principles:** Prefer data binding (`@reactive`/`@computed`) and event binding (`OnClicked`/`OnMouseButtonDownEvent`) over manual widget manipulation. Minimize coupling between WidgetMarkupComponents — use `@computed` (local derivation) and callback registration instead of reaching into parent/child internals. See [Design Principles](docs/python-components.md#design-principles) for details.

> **WidgetMarkup `unreal` API:** Python runs in WidgetMarkup, not the full Editor. Use native **`unreal`** by default. Library names follow UE `ScriptName` (`unreal.WidgetLibrary`, not `WidgetBlueprintLibrary`). For pointer-event input (mouse buttons), use `mouse_event.effecting_button.get_editor_property("key_name")` directly — see [PointerEvent](docs/unreal-python-api.md#pointerevent--use-effecting_buttonget_editor_propertykey_name). For event replies, use `widget_markup.WidgetLibrary.handled()` (returns `FWidgetMarkupEventReply`) — the legacy `unreal.WidgetLibrary.handled()` returns an incompatible `FEventReply`. See [Event Reply](docs/unreal-python-api.md#event-reply--fwidgetmarkupeventreply).

- **`@reactive`** — settable properties (int, float, str, bool, list) that auto-push to bound widgets
- **`@computed`** — read-only derived properties with automatic dependency tracking
- **`ObservableCollection`** — reactive lists; `append`/`pop`/`clear` auto-trigger ListView refresh
- **ListView/TileView** — entry mixins (`IUserListEntry`, `IUserObjectListEntry`), `on_data_refresh(data)`, delegate binding
- **Accessing widgets** — prefer data binding over `get_editor_property()`

> **Use `typing.Optional` for nullable types:** Always import `Optional` from `typing` for type hints that can be `None`:
> ```python
> from typing import Optional
> def evaluate(expr: str) -> Optional[float]: ...
> ```

## 5. Style Sheets

Style sheets apply runtime property overrides to widgets by type and/or name. They are defined inline inside `<WidgetBlueprint>` or as standalone `.widgetmarkup` files. Styles are stored in `UWidgetMarkupBlueprintExtension` and resolved via `UWidgetStyleSheet::ResolveComputedStyles`.

### 5.1 Style Sheet Forms

**Inline** — `<StyleSheet>` as a child of `<WidgetBlueprint>` (before `<WidgetTree>`):

```xml
<WidgetBlueprint Script="MyApp.MyComponent">
  <StyleSheet>
    <Style TargetType="TextBlock">
      <Setter Property="Font.Size" Value="14" />
      <Setter Property="ColorAndOpacity" Value="0.78,0.78,0.78,1" />
    </Style>
  </StyleSheet>
  <WidgetTree>
    <TextBlock Text="Hello" />
  </WidgetTree>
</WidgetBlueprint>
```

**Standalone** — root element is `<StyleSheet>`, no `<WidgetTree>`:

```xml
<StyleSheet>
  <Style TargetType="TextBlock">
    <Setter Property="Font.Size" Value="12" />
    <Setter Property="ColorAndOpacity" Value="0.5,0.5,0.5,1" />
  </Style>
</StyleSheet>
```

Standalone stylesheets can be inherited by other stylesheets (see §5.2).

### 5.2 Style Inheritance

Use `Inherit="/Path/To/Base"` on `<StyleSheet>` to merge styles from a base stylesheet. The base is loaded first, then the current sheet's styles are merged on top — same `(TargetType, Name)` pairs replace the base entry.

```xml
<!-- BaseStyles.widgetmarkup: base implicit TextBlock style -->
<StyleSheet>
  <Style TargetType="TextBlock">
    <Setter Property="Font.Size" Value="12" />
    <Setter Property="ColorAndOpacity" Value="0.5,0.5,0.5,1" />
  </Style>
</StyleSheet>
```

```xml
<!-- Override: inherits base, overrides Font.Size, adds named Title style -->
<WidgetBlueprint Script="MyApp.MyComponent">
  <StyleSheet Inherit="/Game/Styles/BaseStyles">
    <Style TargetType="TextBlock">
      <Setter Property="Font.Size" Value="18" />
    </Style>
    <Style TargetType="TextBlock" Name="Title">
      <Setter Property="Font.Size" Value="24" />
      <Setter Property="ColorAndOpacity" Value="1,0.5,0,1" />
    </Style>
  </StyleSheet>
  <WidgetTree>...</WidgetTree>
</WidgetBlueprint>
```

### 5.3 Implicit vs Explicit Styles

- **Implicit** (no `Name`): matches ALL widgets of the given `TargetType`. Applied automatically.
- **Explicit** (`Name="StyleName"`): only applied when a widget has `Style="StyleName"`.

```xml
<StyleSheet>
  <!-- Implicit: every TextBlock gets Font.Size=14 -->
  <Style TargetType="TextBlock">
    <Setter Property="Font.Size" Value="14" />
  </Style>

  <!-- Explicit: only TextBlocks with Style="Header" get these overrides -->
  <Style TargetType="TextBlock" Name="Header">
    <Setter Property="Font.Size" Value="26" />
    <Setter Property="ColorAndOpacity" Value="1,1,1,1" />
  </Style>

  <!-- Explicit on non-TextBlock types -->
  <Style TargetType="Border" Name="Card">
    <Setter Property="BrushColor" Value="0.12,0.12,0.14,1" />
    <Setter Property="Padding.Left" Value="12" />
    <Setter Property="Padding.Top" Value="8" />
    <Setter Property="Padding.Right" Value="12" />
    <Setter Property="Padding.Bottom" Value="8" />
  </Style>

  <Style TargetType="Button" Name="PrimaryButton">
    <Setter Property="ColorAndOpacity" Value="0.15,0.5,0.9,1" />
  </Style>
</StyleSheet>
```

Apply explicit styles with the `Style` attribute:

```xml
<TextBlock Style="Header" Text="Section Title" />
<Border Style="Card">
  <TextBlock Text="Card content" />
</Border>
<Button Style="PrimaryButton" OnClicked="on_confirm">
  <TextBlock Text="Confirm" />
</Button>
```

### 5.4 Setter Syntax

`<Setter Property="PropertyPath" Value="LiteralValue" />`

The `Property` supports dot-separated sub-property paths (e.g., `Font.Size`, `Padding.Left`). The `Value` is parsed through the same conversion pipeline as XML attributes (see §3.4).

| Setter example | Effect |
|---|---|
| `<Setter Property="Font.Size" Value="14" />` | Set nested struct field |
| `<Setter Property="ColorAndOpacity" Value="1,0,0,1" />` | FLinearColor from CSV string |
| `<Setter Property="Padding.Left" Value="12" />` | Single margin component |
| `<Setter Property="BrushColor" Value="0.12,0.12,0.14,1" />` | FLinearColor on Border |
| `<Setter Property="FillColorAndOpacity" Value="0.1,0.8,0.3,1" />` | Fill color on ProgressBar |

**Child-element form** — the value (or container entries) is provided as child elements instead of the `Value` attribute. The setter resolves the property type at compile time from the style's `TargetType` + `Property` path, and the parsed value is stored in the setter's buffer (applied via `SetValue(Buffer)` at runtime):

```xml
<!-- Single Basic element = scalar value -->
<Setter Property="Font.Size"><Float>24</Float></Setter>
<!-- Struct element = struct value (fields via attributes) -->
<Setter Property="BrushColor"><LinearColor R="0.3" G="0.2" B="0.1" A="1" /></Setter>
<!-- Multiple children = container entries (Array/Set) -->
<Setter Property="ColumnFill"><Float>1.0</Float><Float>2.0</Float></Setter>
```

- `Value` attribute + child elements together is an **error**.
- Container children map to container entries (Array/Set elements; Map uses `<Pair>`); a single child on a scalar/struct property is the whole value.
- Paths that cannot be resolved at compile time (object-pointer segments like `Slot.*`, array indices, unknown targets) emit a **warning** and the child element is ignored — use the `Value` attribute for those.

### 5.5 System Fonts via Font.FontObject

`Font.FontObject` accepts a font family name (e.g., `'seguisym'`) to load a system font without requiring a UFont asset:

```xml
<Style TargetType="TextBlock" Name="IconFont">
  <Setter Property="Font.FontObject" Value="seguisym" />
  <Setter Property="Font.Size" Value="20" />
  <Setter Property="ColorAndOpacity" Value="0.3,0.85,1,1" />
</Style>
```

```xml
<TextBlock Style="IconFont" Text="▲ ▼ ◀ ▶" />
```

> **How it works:** `FObjectConverter` first tries `StaticLoadObject` on the value as an asset path. If that fails and the property is `FontObject` on `FSlateFontInfo`, `UWidgetMarkupFontProvider::CreateFromFontName` resolves the font file from OS font directories (Windows: `%SystemRoot%/Fonts/`, `%LocalAppData%/Microsoft/Windows/Fonts/`; Mac: `/System/Library/Fonts/`, `/Library/Fonts/`; Linux: `/usr/share/fonts/truetype/`).

### 5.6 Resolution Order

1. Base stylesheet loaded (if `Inherit` specified)
2. Current sheet's styles merged on top (same `(TargetType, Name)` replaces)
3. `UWidgetStyleSheet::ResolveComputedStyles` produces flat `ComputedStyles` array
4. At widget construction, implicit styles apply first, then explicit `Style="Name"` overrides

> **Warning:** Styles are applied at widget construction time. Changing `Style` at runtime has no effect.

## 6. Build & Run

**WidgetMarkupApp** is only the standalone `.exe` launcher. WidgetMarkup itself is the plugin (markup compiler, Python integration, `widget_markup` module, and so on); those APIs are the same whether you run inside WidgetMarkupApp or another host that loads the plugin.

```powershell
# Build
cd C:\PROJECTS\UnrealEngine
.\Engine\Build\BatchFiles\Build.bat WidgetMarkupApp Win64 Development -Project=Game/Game.uproject

# Run a plugin sample (mounted at /WidgetMarkup/)
.\Game\Binaries\Win64\WidgetMarkupApp.exe /WidgetMarkup/Samples/Counter --project Game/Game.uproject

# Run a project blueprint (mounted at /Game/ — project Content/ directory)
.\Game\Binaries\Win64\WidgetMarkupApp.exe /Game/ScientificCalculator --project Game/Game.uproject

# Clear Python cache before testing (clears BOTH project and plugin paths)
Get-ChildItem <ProjectDir> -Recurse -Directory -Filter "__pycache__" | Remove-Item -Recurse -Force
```

> Always clear caches before testing — stale `.pyc` files can silently load old code.

## 7. Samples

Sample `.widgetmarkup` blueprints and Python components are in the plugin directory:

```
Game/Plugins/WidgetMarkup/Content/Samples/
```

Includes: **ScientificCalculator** (reactive/computed properties with style sheets) and **Minesweeper** (dynamic cell creation with `add_child`, callback event routing, and `@computed` auto-tracking). See the samples for complete working examples of all concepts covered in this document.

## 8. Blueprint Compilation Caveat

Blueprint compilation with Python components can trigger UE's background garbage collector (`FRealtimeGC`) on worker threads. When Python wrappers exist, these background threads may try to acquire the Python GIL via `FPyReferenceCollector`, causing a hang or crash. WidgetMarkup's compiler uses `EBlueprintCompileOptions::SkipGarbageCollection` to avoid this — do NOT add `IncludeCDOInReferenceReplacement` or other GC-triggering options to compile flags in `BlueprintElementNode.cpp`.